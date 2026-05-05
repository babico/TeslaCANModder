#pragma once
// ── Tesla key generation (board-agnostic) ────────────────────────────────────
// Pure P-256 ECDSA key generation via mbedTLS.  No storage dependencies.
// Key persistence belongs in the Board adapter (e.g. Esp32Board in esp32/board.h).

#if BOARD_ENABLE_BLE

#include <mbedtls/ecdsa.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ecp.h>
#include <stdint.h>
#include <string.h>

namespace Tesla
{

static const uint8_t ROLE_OWNER = 4;
static const uint8_t ROLE_CHARGING_MANAGER = 5;

struct KeyPair
{
	uint8_t priv_d[32]; // raw big-endian private scalar
	uint8_t pub_xy[65]; // 0x04 || X(32) || Y(32), uncompressed P-256
	bool valid;
};

// ── Generate a new P-256 key pair ────────────────────────────────────────────
static bool keyGenerate(KeyPair &kp)
{
	mbedtls_entropy_context ent;
	mbedtls_ctr_drbg_context drbg;
	mbedtls_ecp_keypair pair;

	mbedtls_entropy_init(&ent);
	mbedtls_ctr_drbg_init(&drbg);
	mbedtls_ecp_keypair_init(&pair);

	kp.valid = false;
	bool ok = false;

	do
	{
		const unsigned char pers[] = "tesla_key_gen";
		if (mbedtls_ctr_drbg_seed(&drbg, mbedtls_entropy_func, &ent, pers, sizeof(pers) - 1) != 0)
			break;
		if (mbedtls_ecp_gen_key(MBEDTLS_ECP_DP_SECP256R1, &pair, mbedtls_ctr_drbg_random, &drbg) != 0)
			break;

		memset(kp.priv_d, 0, 32);
		size_t dlen = mbedtls_mpi_size(&pair.d);
		if (dlen > 32)
			break;
		if (mbedtls_mpi_write_binary(&pair.d, kp.priv_d + (32 - dlen), dlen) != 0)
			break;

		size_t olen = 0;
		if (mbedtls_ecp_point_write_binary(&pair.grp, &pair.Q, MBEDTLS_ECP_PF_UNCOMPRESSED, &olen, kp.pub_xy, 65) != 0)
			break;
		if (olen != 65)
			break;

		kp.valid = true;
		ok = true;
	} while (false);

	mbedtls_ecp_keypair_free(&pair);
	mbedtls_ctr_drbg_free(&drbg);
	mbedtls_entropy_free(&ent);
	return ok;
}

// ── Re-derive public key from stored private scalar ───────────────────────────
static bool keyDerivePublic(KeyPair &kp)
{
	mbedtls_entropy_context ent;
	mbedtls_ctr_drbg_context drbg;
	mbedtls_ecp_keypair pair;

	mbedtls_entropy_init(&ent);
	mbedtls_ctr_drbg_init(&drbg);
	mbedtls_ecp_keypair_init(&pair);

	bool ok = false;
	do
	{
		const unsigned char pers[] = "tesla_key_load";
		if (mbedtls_ctr_drbg_seed(&drbg, mbedtls_entropy_func, &ent, pers, sizeof(pers) - 1) != 0)
			break;
		if (mbedtls_ecp_group_load(&pair.grp, MBEDTLS_ECP_DP_SECP256R1) != 0)
			break;
		if (mbedtls_mpi_read_binary(&pair.d, kp.priv_d, 32) != 0)
			break;

		if (mbedtls_ecp_mul(&pair.grp, &pair.Q, &pair.d, &pair.grp.G, mbedtls_ctr_drbg_random, &drbg) != 0)
			break;

		size_t olen = 0;
		if (mbedtls_ecp_point_write_binary(&pair.grp, &pair.Q, MBEDTLS_ECP_PF_UNCOMPRESSED, &olen, kp.pub_xy, 65) != 0)
			break;
		if (olen != 65)
			break;

		kp.valid = true;
		ok = true;
	} while (false);

	mbedtls_ecp_keypair_free(&pair);
	mbedtls_ctr_drbg_free(&drbg);
	mbedtls_entropy_free(&ent);
	return ok;
}

// ── Hex-encode bytes → null-terminated string (buf must be ≥ 2*len+1) ────────
static void hexEncode(const uint8_t *data, size_t len, char *out)
{
	static const char h[] = "0123456789abcdef";
	for (size_t i = 0; i < len; i++)
	{
		out[2 * i] = h[data[i] >> 4];
		out[2 * i + 1] = h[data[i] & 0x0F];
	}
	out[2 * len] = '\0';
}

} // namespace Tesla

#endif // BOARD_ENABLE_BLE
