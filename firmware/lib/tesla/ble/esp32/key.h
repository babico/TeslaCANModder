#pragma once
// ── Tesla key generation & NVS storage ───────────────────────────────────────
// Generates an ECDSA P-256 key pair via mbedTLS and persists the raw 32-byte
// private scalar in the "tesla" NVS namespace.  The uncompressed public key
// (0x04 || X || Y, 65 bytes) is re-derived on load.
//
// Dependencies: Preferences (ESP32), mbedTLS (bundled with ESP-IDF / Arduino)

#if BOARD_ENABLE_BLE

#include <Preferences.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ecp.h>
#include <stdint.h>
#include <string.h>

namespace Tesla {

static const char *NVS_NS   = "tesla";
static const char *NVS_PRIV = "priv_d";
static const char *NVS_ROLE = "key_role";
static const char *NVS_VIN  = "vin";

// Roles from keys.proto
static const uint8_t ROLE_OWNER            = 4;
static const uint8_t ROLE_CHARGING_MANAGER = 5;

struct KeyPair {
    uint8_t priv_d[32]; // raw big-endian private scalar
    uint8_t pub_xy[65]; // 0x04 || X(32) || Y(32), uncompressed P-256
    bool    valid;
};

// ── key generation ────────────────────────────────────────────────────────────
static bool keyGenerate(KeyPair &kp)
{
    mbedtls_entropy_context  ent;
    mbedtls_ctr_drbg_context drbg;
    mbedtls_ecp_keypair      pair;

    mbedtls_entropy_init(&ent);
    mbedtls_ctr_drbg_init(&drbg);
    mbedtls_ecp_keypair_init(&pair);

    kp.valid = false;
    bool ok  = false;

    do {
        const unsigned char pers[] = "tesla_key_gen";
        if (mbedtls_ctr_drbg_seed(&drbg, mbedtls_entropy_func, &ent,
                                   pers, sizeof(pers) - 1) != 0)
            break;
        if (mbedtls_ecp_gen_key(MBEDTLS_ECP_DP_SECP256R1, &pair,
                                 mbedtls_ctr_drbg_random, &drbg) != 0)
            break;

        // export private scalar d (big-endian, zero-padded to 32 bytes)
        memset(kp.priv_d, 0, 32);
        size_t dlen = mbedtls_mpi_size(&pair.d);
        if (dlen > 32) break;
        if (mbedtls_mpi_write_binary(&pair.d,
                                     kp.priv_d + (32 - dlen), dlen) != 0)
            break;

        // export uncompressed public key
        size_t olen = 0;
        if (mbedtls_ecp_point_write_binary(&pair.grp, &pair.Q,
                                            MBEDTLS_ECP_PF_UNCOMPRESSED,
                                            &olen, kp.pub_xy, 65) != 0)
            break;
        if (olen != 65) break;

        kp.valid = true;
        ok = true;
    } while (false);

    mbedtls_ecp_keypair_free(&pair);
    mbedtls_ctr_drbg_free(&drbg);
    mbedtls_entropy_free(&ent);
    return ok;
}

// ── derive public key from stored private scalar ──────────────────────────────
static bool keyDerivePublic(KeyPair &kp)
{
    mbedtls_entropy_context  ent;
    mbedtls_ctr_drbg_context drbg;
    mbedtls_ecp_keypair      pair;

    mbedtls_entropy_init(&ent);
    mbedtls_ctr_drbg_init(&drbg);
    mbedtls_ecp_keypair_init(&pair);

    bool ok = false;
    do {
        const unsigned char pers[] = "tesla_key_load";
        if (mbedtls_ctr_drbg_seed(&drbg, mbedtls_entropy_func, &ent,
                                   pers, sizeof(pers) - 1) != 0)
            break;
        if (mbedtls_ecp_group_load(&pair.grp,
                                    MBEDTLS_ECP_DP_SECP256R1) != 0)
            break;
        if (mbedtls_mpi_read_binary(&pair.d, kp.priv_d, 32) != 0) break;

        // Q = d * G
        if (mbedtls_ecp_mul(&pair.grp, &pair.Q, &pair.d, &pair.grp.G,
                             mbedtls_ctr_drbg_random, &drbg) != 0)
            break;

        size_t olen = 0;
        if (mbedtls_ecp_point_write_binary(&pair.grp, &pair.Q,
                                            MBEDTLS_ECP_PF_UNCOMPRESSED,
                                            &olen, kp.pub_xy, 65) != 0)
            break;
        if (olen != 65) break;

        kp.valid = true;
        ok = true;
    } while (false);

    mbedtls_ecp_keypair_free(&pair);
    mbedtls_ctr_drbg_free(&drbg);
    mbedtls_entropy_free(&ent);
    return ok;
}

// ── NVS persistence ───────────────────────────────────────────────────────────
static bool keySave(const KeyPair &kp)
{
    Preferences p;
    p.begin(NVS_NS, false);
    p.putBytes(NVS_PRIV, kp.priv_d, 32);
    p.end();
    return true;
}

static bool keyLoad(KeyPair &kp)
{
    Preferences p;
    p.begin(NVS_NS, true);
    size_t n = p.getBytesLength(NVS_PRIV);
    if (n != 32) { p.end(); kp.valid = false; return false; }
    p.getBytes(NVS_PRIV, kp.priv_d, 32);
    p.end();
    return keyDerivePublic(kp);
}

static void keyRoleSet(const char *role)
{
    Preferences p;
    p.begin(NVS_NS, false);
    p.putString(NVS_ROLE, role);
    p.end();
}

static String keyRoleGet()
{
    Preferences p;
    p.begin(NVS_NS, true);
    String r = p.getString(NVS_ROLE, "charging_manager");
    p.end();
    return r;
}

// Translate role string → Role enum value
static uint8_t roleValue(const String &role)
{
    if (role == "owner") return ROLE_OWNER;
    return ROLE_CHARGING_MANAGER;
}

static void vinSet(const char *vin)
{
    Preferences p;
    p.begin(NVS_NS, false);
    p.putString(NVS_VIN, vin);
    p.end();
}

static String vinGet()
{
    Preferences p;
    p.begin(NVS_NS, true);
    String v = p.getString(NVS_VIN, "");
    p.end();
    return v;
}

// Hex-encode bytes → null-terminated string (caller supplies buf ≥ 2*len+1)
static void hexEncode(const uint8_t *data, size_t len, char *out)
{
    static const char h[] = "0123456789abcdef";
    for (size_t i = 0; i < len; i++) {
        out[2 * i]     = h[data[i] >> 4];
        out[2 * i + 1] = h[data[i] & 0x0F];
    }
    out[2 * len] = '\0';
}

} // namespace Tesla

#endif // BOARD_ENABLE_BLE
