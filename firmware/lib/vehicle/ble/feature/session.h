#pragma once

/**
 * @file firmware/lib/vehicle/ble/feature/session.h
 * @brief ECDH session establishment and AES-128-GCM authenticated command dispatch for Tesla BLE
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 *
 * TeslaSession<Rng> handles ECDH key exchange and AES-128-GCM authenticated
 * command dispatch. No board-specific dependencies.
 *
 * Rng concept — must provide:
 *   void random(uint8_t *out, size_t len)
 *
 * Transport concept (deduced from usage) — must provide:
 *   bool exchange(const uint8_t *req, size_t reqLen,
 *                 const uint8_t **resp, size_t *respLen, uint32_t timeoutMs)
 *   bool send(const uint8_t *buf, size_t len)
 */

#if BOARD_ENABLE_BLE

#include "key.h"
#include "../msg.h"
#include <mbedtls/ecdh.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/gcm.h>
#include <mbedtls/md.h>
#include <mbedtls/ecp.h>
#include <stdint.h>
#include <string.h>

namespace Tesla {

/**
 * @brief Manages an authenticated ECDH session with a Tesla vehicle over BLE
 * @tparam Rng Random number generator satisfying the Rng concept
 */
template<typename Rng>
class TeslaSession {
public:
	/**
	 * @brief Construct a session bound to the given RNG source
	 * @param rng Reference to a random number generator instance
	 */
	explicit TeslaSession(Rng &rng)
		: _rng(rng), _counter(0), _valid(false) {}

	/**
	 * @brief Check whether the session has been successfully established
	 * @return True if the session keys are valid and ready for command dispatch
	 */
	bool isValid() const { return _valid; }

	/**
	 * @brief Establish an ECDH session with the vehicle
	 * @tparam Transport BLE transport satisfying the Transport concept
	 * @param client Transport instance for request/response exchange
	 * @param domain Target domain: DOMAIN_INFOTAINMENT (3) or DOMAIN_VEHICLE_SECURITY (2)
	 * @return True if session establishment succeeded
	 */
	template<typename Transport>
	bool establish(Transport &client, uint8_t domain = DOMAIN_INFOTAINMENT)
	{
		_valid = false;

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
			                           pers, sizeof(pers) - 1) != 0) break;
			if (mbedtls_ecp_gen_key(MBEDTLS_ECP_DP_SECP256R1, &eph,
			                         mbedtls_ctr_drbg_random, &drbg) != 0) break;

			// Export ephemeral public key in uncompressed form
			size_t olen = 0;
			if (mbedtls_ecp_point_write_binary(&eph.grp, &eph.Q,
			                                    MBEDTLS_ECP_PF_UNCOMPRESSED,
			                                    &olen, _ephPub, 65) != 0) break;
			if (olen != 65) break;

			genRandom16(_myChallenge);
			uint8_t uuid[16]; genRandom16(uuid);

			uint8_t reqBuf[200];
			size_t  reqLen = 0;
			if (!buildSessionInfoRequest(_ephPub, _myChallenge, domain, uuid,
			                             reqBuf, sizeof(reqBuf), reqLen)) break;

			const uint8_t *resp = nullptr;
			size_t         respLen = 0;
			if (!client.exchange(reqBuf, reqLen, &resp, &respLen, 12000)) break;

			size_t siLen = 0;
			const uint8_t *si = parseSessionInfo(resp, respLen, siLen);
			if (!si || siLen == 0) break;

			SessionInfoFields sif = parseSessionInfoFields(si, siLen);
			if (!sif.valid || sif.publicKeyLen != 65) break;
			memcpy(_vehEphPub, sif.publicKey, 65);
			memcpy(_vehChallenge, sif.challenge,
			       sif.challengeLen < 32 ? sif.challengeLen : 32);
			_vehChallengeLen = sif.challengeLen;

			// ECDH: shared secret = ephemeral_private * vehicle_ephemeral_public
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
				if (mbedtls_ecp_point_read_binary(&grp, &vehQ,
				                                   _vehEphPub, 65) != 0) break;
				if (mbedtls_mpi_copy(&d, &eph.d) != 0) break;

				mbedtls_ecp_point sharedPoint;
				mbedtls_ecp_point_init(&sharedPoint);
				int r = mbedtls_ecp_mul(&grp, &sharedPoint, &d, &vehQ,
				                         mbedtls_ctr_drbg_random, &drbg);
				if (r != 0) { mbedtls_ecp_point_free(&sharedPoint); break; }

				// Extract X coordinate of the shared point as the raw shared secret
				uint8_t sharedSecret[32] = {};
				size_t xLen = mbedtls_mpi_size(&sharedPoint.X);
				if (xLen > 32) { mbedtls_ecp_point_free(&sharedPoint); break; }
				mbedtls_mpi_write_binary(&sharedPoint.X,
				                          sharedSecret + (32 - xLen), xLen);
				mbedtls_ecp_point_free(&sharedPoint);

				// HKDF-SHA256 (manual Extract + Expand) to derive session keys
				const mbedtls_md_info_t *sha256 =
					mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

				// Extract: PRK = HMAC-SHA256(salt=vehicle_challenge, IKM=shared_secret)
				uint8_t prk[32];
				if (mbedtls_md_hmac(sha256,
				                    _vehChallenge, _vehChallengeLen,
				                    sharedSecret, 32, prk) != 0) break;

				// Info = our_ephemeral_pub || vehicle_ephemeral_pub (130 bytes)
				uint8_t info[130];
				memcpy(info,      _ephPub,    65);
				memcpy(info + 65, _vehEphPub, 65);

				// Expand T(1) = HMAC-SHA256(PRK, info || 0x01)
				uint8_t t1[32];
				{
					uint8_t m[131];
					memcpy(m, info, 130);
					m[130] = 0x01;
					if (mbedtls_md_hmac(sha256, prk, 32, m, 131, t1) != 0) break;
				}
				// Expand T(2) = HMAC-SHA256(PRK, T(1) || info || 0x02)
				uint8_t t2[32];
				{
					uint8_t m[163];
					memcpy(m,      t1,   32);
					memcpy(m + 32, info, 130);
					m[162] = 0x02;
					if (mbedtls_md_hmac(sha256, prk, 32, m, 163, t2) != 0) break;
				}

				// First 16 bytes = AES encryption key, next 32 bytes = MAC key
				uint8_t derived[48];
				memcpy(derived,      t1, 32);
				memcpy(derived + 32, t2, 16);
				memset(prk, 0, 32); memset(t1, 0, 32); memset(t2, 0, 32);

				memcpy(_encKey, derived,      16);
				memcpy(_macKey, derived + 16, 32);
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

	/**
	 * @brief Encrypt a CarServer.Action payload and send as an authenticated RoutableMessage
	 * @tparam Transport BLE transport satisfying the Transport concept
	 * @param client Transport instance for sending the encrypted message
	 * @param plaintext Raw payload bytes to encrypt
	 * @param ptLen Length of the plaintext buffer
	 * @return True if encryption and transmission succeeded
	 */
	template<typename Transport>
	bool sendCommand(Transport &client, const uint8_t *plaintext, size_t ptLen)
	{
		if (!_valid) return false;

		_counter++;

		// IV is the little-endian counter padded to 12 bytes
		uint8_t iv[12] = {};
		iv[0] = (uint8_t)(_counter & 0xFF);
		iv[1] = (uint8_t)((_counter >>  8) & 0xFF);
		iv[2] = (uint8_t)((_counter >> 16) & 0xFF);
		iv[3] = (uint8_t)((_counter >> 24) & 0xFF);

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
			                               ptLen, iv, sizeof(iv),
			                               nullptr, 0,
			                               plaintext, ciphertext,
			                               sizeof(tag), tag) != 0) break;
			gcmOk = true;
		} while (false);
		mbedtls_gcm_free(&gcm);
		if (!gcmOk) return false;

		uint8_t uuid[16]; genRandom16(uuid);
		uint8_t outBuf[600];
		size_t  outLen = 0;
		if (!buildAuthMessage(ciphertext, ptLen, tag, _counter, _domain,
		                      uuid, outBuf, sizeof(outBuf), outLen))
			return false;

		return client.send(outBuf, outLen);
	}

private:
	Rng     &_rng;
	bool     _valid;
	uint8_t  _domain;           // Target domain (infotainment or vehicle security)
	uint8_t  _ephPub[65];       // Our ephemeral public key (uncompressed P-256)
	uint8_t  _vehEphPub[65];    // Vehicle ephemeral public key (uncompressed P-256)
	uint8_t  _myChallenge[16];  // Random challenge sent to the vehicle
	uint8_t  _vehChallenge[32]; // Challenge received from the vehicle
	size_t   _vehChallengeLen;
	uint8_t  _encKey[16];       // AES-128-GCM encryption key derived from HKDF
	uint8_t  _macKey[32];       // HMAC key derived from HKDF
	uint32_t _counter;          // Monotonic message counter used as GCM nonce

	/**
	 * @brief Generate 16 random bytes using the bound RNG
	 * @param out Output buffer (must be at least 16 bytes)
	 */
	void genRandom16(uint8_t *out) { _rng.random(out, 16); }
};

} // namespace Tesla

#endif // BOARD_ENABLE_BLE
