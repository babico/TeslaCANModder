#pragma once
// ── Tesla ECDH session establishment + authenticated command dispatch ──────────
//
// Protocol (per Tesla vehicle-command SDK, DOMAIN_INFOTAINMENT):
//   1. Generate ephemeral P-256 key pair.
//   2. Build SessionInfoRequest { public_key: eph_pub, challenge: random16 }.
//   3. Wrap in RoutableMessage(session_info_request=6) and send via BLE.
//   4. Receive RoutableMessage; extract session_info (field 10).
//   5. Parse SessionInfo: vehicle ephemeral pubkey + vehicle challenge.
//   6. ECDH(our_eph_priv, vehicle_eph_pub) → 32-byte shared secret (x-coord).
//   7. HKDF-SHA256(ikm=shared, salt=vehicle_challenge, info=our_eph_pub||veh_eph_pub)
//      → derive 16-byte AES key + 32-byte (unused in AES-GCM path, kept for
//        future HMAC use).
//   8. For each command: AES-128-GCM encrypt plaintext, build authenticated
//      RoutableMessage with SignedData, send via BLE.
//
// NOTE: The exact HKDF salt/info values follow the Tesla SDK convention.  If
//       the vehicle rejects session setup the salt/info parameters may need
//       adjustment to match any SDK revision changes.

#if BOARD_ENABLE_BLE

#include "key.h"
#include "msg.h"
#include "ble_client.h"
#include <mbedtls/ecdh.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/gcm.h>
#include <mbedtls/md.h>
#include <mbedtls/ecp.h>
#include <stdint.h>
#include <string.h>

namespace Tesla {

// ── Random 16-byte UUID generator ────────────────────────────────────────────
static void genRandom16(uint8_t *out)
{
    // Use ESP32 hardware RNG via stdlib
    for (int i = 0; i < 16; i += 4) {
        uint32_t r = esp_random();
        memcpy(out + i, &r, 4);
    }
}

// ── TeslaSession ─────────────────────────────────────────────────────────────
class TeslaSession {
public:
    TeslaSession() : _counter(0), _valid(false) {}

    // Establish ECDH session with the vehicle.
    // domain: DOMAIN_INFOTAINMENT (3) or DOMAIN_VEHICLE_SECURITY (2)
    bool establish(TeslaClient &client, uint8_t domain = DOMAIN_INFOTAINMENT)
    {
        _valid = false;

        // ── 1. Generate ephemeral P-256 key pair ─────────────────────────────
        mbedtls_entropy_context  ent;
        mbedtls_ctr_drbg_context drbg;
        mbedtls_ecp_keypair      eph;

        mbedtls_entropy_init(&ent);
        mbedtls_ctr_drbg_init(&drbg);
        mbedtls_ecp_keypair_init(&eph);

        bool ok = false;
        do {
            const unsigned char pers[] = "tesla_session";
            if (mbedtls_ctr_drbg_seed(&drbg, mbedtls_entropy_func, &ent,
                                       pers, sizeof(pers) - 1) != 0)
                break;
            if (mbedtls_ecp_gen_key(MBEDTLS_ECP_DP_SECP256R1, &eph,
                                     mbedtls_ctr_drbg_random, &drbg) != 0)
                break;

            // Export ephemeral public key (uncompressed, 65 bytes)
            size_t olen = 0;
            if (mbedtls_ecp_point_write_binary(&eph.grp, &eph.Q,
                                                MBEDTLS_ECP_PF_UNCOMPRESSED,
                                                &olen, _ephPub, 65) != 0)
                break;
            if (olen != 65) break;

            // ── 2. Build SessionInfoRequest ───────────────────────────────────
            genRandom16(_myChallenge);
            uint8_t uuid[16]; genRandom16(uuid);

            uint8_t reqBuf[200];
            size_t  reqLen = 0;
            if (!buildSessionInfoRequest(_ephPub, _myChallenge, domain, uuid,
                                         reqBuf, sizeof(reqBuf), reqLen))
                break;

            // ── 3. Send and receive ───────────────────────────────────────────
            const uint8_t *resp = nullptr;
            size_t         respLen = 0;
            if (!client.exchange(reqBuf, reqLen, &resp, &respLen, 12000))
                break;

            // ── 4. Extract session_info bytes (field 10) ──────────────────────
            size_t siLen = 0;
            const uint8_t *si = parseSessionInfo(resp, respLen, siLen);
            if (!si || siLen == 0) break;

            // ── 5. Parse SessionInfo fields ───────────────────────────────────
            SessionInfoFields sif = parseSessionInfoFields(si, siLen);
            if (!sif.valid || sif.publicKeyLen != 65) break;
            memcpy(_vehEphPub, sif.publicKey, 65);
            memcpy(_vehChallenge, sif.challenge,
                   sif.challengeLen < 32 ? sif.challengeLen : 32);
            _vehChallengeLen = sif.challengeLen;

            // ── 6. ECDH: shared secret = eph_priv × veh_eph_pub ─────────────
            mbedtls_ecp_group  grp;
            mbedtls_ecp_point  vehQ;
            mbedtls_mpi        d, sharedX;

            mbedtls_ecp_group_init(&grp);
            mbedtls_ecp_point_init(&vehQ);
            mbedtls_mpi_init(&d);
            mbedtls_mpi_init(&sharedX);

            bool ecdhOk = false;
            do {
                if (mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP256R1) != 0) break;

                // Import vehicle ephemeral public key
                if (mbedtls_ecp_point_read_binary(&grp, &vehQ,
                                                   _vehEphPub, 65) != 0) break;

                // Copy private scalar from ephemeral key pair
                if (mbedtls_mpi_copy(&d, &eph.d) != 0) break;

                // Compute shared point: P = d * vehQ
                mbedtls_ecp_point sharedPoint;
                mbedtls_ecp_point_init(&sharedPoint);
                int r = mbedtls_ecp_mul(&grp, &sharedPoint, &d, &vehQ,
                                         mbedtls_ctr_drbg_random, &drbg);
                if (r != 0) { mbedtls_ecp_point_free(&sharedPoint); break; }

                // Extract x-coordinate as 32 bytes (big-endian)
                uint8_t sharedSecret[32] = {};
                size_t xLen = mbedtls_mpi_size(&sharedPoint.X);
                if (xLen > 32) { mbedtls_ecp_point_free(&sharedPoint); break; }
                mbedtls_mpi_write_binary(&sharedPoint.X,
                                          sharedSecret + (32 - xLen), xLen);
                mbedtls_ecp_point_free(&sharedPoint);

                // ── 7. HKDF-SHA256 (manual Extract + Expand) ─────────────────
                // IKM  = sharedSecret (32 bytes)
                // Salt = vehicle challenge from SessionInfo
                // Info = our_eph_pub || veh_eph_pub  (65 + 65 = 130 bytes)
                //
                // HKDF-Extract: PRK = HMAC-SHA256(salt, IKM)
                // HKDF-Expand:  T(1) = HMAC-SHA256(PRK, info || 0x01) [32 bytes]
                //               T(2) = HMAC-SHA256(PRK, T(1) || info || 0x02) [32 bytes]
                //               OKM  = T(1)[0:16] as encKey, T(1)[16:32]+T(2)[0:16] as macKey
                const mbedtls_md_info_t *sha256 =
                    mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

                // Extract
                uint8_t prk[32];
                if (mbedtls_md_hmac(sha256,
                                    _vehChallenge, _vehChallengeLen,
                                    sharedSecret, 32,
                                    prk) != 0) break;

                // Expand — need 48 bytes: two SHA-256 rounds
                uint8_t info[130];
                memcpy(info,      _ephPub,    65);
                memcpy(info + 65, _vehEphPub, 65);

                // T(1) = HMAC-SHA256(PRK, info || 0x01)
                uint8_t t1[32];
                {
                    uint8_t m[131]; // info(130) + counter(1)
                    memcpy(m, info, 130);
                    m[130] = 0x01;
                    if (mbedtls_md_hmac(sha256, prk, 32, m, 131, t1) != 0) break;
                }
                // T(2) = HMAC-SHA256(PRK, T(1) || info || 0x02)
                uint8_t t2[32];
                {
                    uint8_t m[163]; // t1(32) + info(130) + counter(1)
                    memcpy(m,       t1,   32);
                    memcpy(m + 32,  info, 130);
                    m[162] = 0x02;
                    if (mbedtls_md_hmac(sha256, prk, 32, m, 163, t2) != 0) break;
                }

                uint8_t derived[48];
                memcpy(derived,      t1, 32);
                memcpy(derived + 32, t2, 16);
                // zero-out temporaries
                memset(prk, 0, 32); memset(t1, 0, 32); memset(t2, 0, 32);

                memcpy(_encKey, derived,      16); // AES-128 key
                memcpy(_macKey, derived + 16, 32); // HMAC-SHA256 key (reserved)

                ecdhOk = true;
            } while (false);

            mbedtls_ecp_group_free(&grp);
            mbedtls_ecp_point_free(&vehQ);
            mbedtls_mpi_free(&d);
            mbedtls_mpi_free(&sharedX);

            if (!ecdhOk) break;

            _counter = 0;
            _domain  = domain;
            _valid   = true;
            ok       = true;
        } while (false);

        mbedtls_ecp_keypair_free(&eph);
        mbedtls_ctr_drbg_free(&drbg);
        mbedtls_entropy_free(&ent);
        return ok;
    }

    bool isValid() const { return _valid; }

    // Encrypt a CarServer.Action payload and send as authenticated RoutableMessage.
    // plaintext/ptLen : serialised CarServer.Action bytes
    bool sendCommand(TeslaClient &client,
                     const uint8_t *plaintext, size_t ptLen)
    {
        if (!_valid) return false;

        _counter++;

        // ── AES-128-GCM nonce: 4-byte LE counter + 8 zero bytes ──────────────
        uint8_t iv[12] = {};
        iv[0] = (uint8_t)(_counter & 0xFF);
        iv[1] = (uint8_t)((_counter >> 8) & 0xFF);
        iv[2] = (uint8_t)((_counter >> 16) & 0xFF);
        iv[3] = (uint8_t)((_counter >> 24) & 0xFF);

        // ── Encrypt ───────────────────────────────────────────────────────────
        static uint8_t ciphertext[512];
        uint8_t tag[16];

        if (ptLen > sizeof(ciphertext)) return false;

        mbedtls_gcm_context gcm;
        mbedtls_gcm_init(&gcm);
        bool gcmOk = false;
        do {
            if (mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES,
                                   _encKey, 128) != 0) break;
            if (mbedtls_gcm_crypt_and_tag(&gcm, MBEDTLS_GCM_ENCRYPT,
                                           ptLen,
                                           iv, sizeof(iv),
                                           nullptr, 0,      // no AAD
                                           plaintext, ciphertext,
                                           sizeof(tag), tag) != 0) break;
            gcmOk = true;
        } while (false);
        mbedtls_gcm_free(&gcm);
        if (!gcmOk) return false;

        // ── Build authenticated RoutableMessage ───────────────────────────────
        uint8_t uuid[16]; genRandom16(uuid);
        uint8_t outBuf[600];
        size_t  outLen = 0;
        if (!buildAuthMessage(ciphertext, ptLen, tag, _counter, _domain,
                              uuid, outBuf, sizeof(outBuf), outLen))
            return false;

        return client.send(outBuf, outLen);
    }

private:
    bool     _valid;
    uint8_t  _domain;
    uint8_t  _ephPub[65];
    uint8_t  _vehEphPub[65];
    uint8_t  _myChallenge[16];
    uint8_t  _vehChallenge[32];
    size_t   _vehChallengeLen;
    uint8_t  _encKey[16];  // AES-128 session key
    uint8_t  _macKey[32];  // HMAC key (reserved, not used in AES-GCM path)
    uint32_t _counter;
};

} // namespace Tesla

#endif // BOARD_ENABLE_BLE
