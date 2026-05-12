#pragma once

/**
 * @file firmware/lib/vehicle/ble/feature/key.h
 * @brief P-256 ECDSA key pair generation and utility functions for Tesla BLE authentication
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

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

/**
 * @brief Holds a P-256 ECDSA key pair (private scalar and uncompressed public point)
 */
struct KeyPair
{
	uint8_t priv_d[32]; // Raw big-endian private scalar (32 bytes)
	uint8_t pub_xy[65]; // Uncompressed P-256 point: 0x04 || X(32) || Y(32)
	bool valid;
};

/**
 * @brief Generate a fresh P-256 key pair using mbedTLS CSPRNG
 * @param kp Output key pair; valid flag is set on success
 * @return True if key generation succeeded
 */
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

		// Zero-pad private scalar to 32 bytes (big-endian, right-aligned)
		memset(kp.priv_d, 0, 32);
		size_t dlen = mbedtls_mpi_size(&pair.d);
		if (dlen > 32)
			break;
		if (mbedtls_mpi_write_binary(&pair.d, kp.priv_d + (32 - dlen), dlen) != 0)
			break;

		// Export public point in uncompressed form (0x04 || X || Y, 65 bytes)
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

/**
 * @brief Re-derive the public key from a stored private scalar
 * @param kp Key pair with priv_d populated; pub_xy and valid are set on success
 * @return True if public key derivation succeeded
 */
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

		// Q = d * G (scalar multiplication on the generator point)
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

/**
 * @brief Hex-encode a byte array into a null-terminated lowercase string
 * @param data Source bytes to encode
 * @param len Number of bytes to encode
 * @param out Output buffer; must be at least (2 * len + 1) bytes
 */
static void hexEncode(const uint8_t *data, size_t len, char *out)
{
	static const char h[] = "0123456789abcdef";
	for (size_t i = 0; i < len; i++)
	{
		out[2 * i] = h[data[i] >> 4];       // High nibble
		out[2 * i + 1] = h[data[i] & 0x0F]; // Low nibble
	}
	out[2 * len] = '\0';
}

} // namespace Tesla

#endif // BOARD_ENABLE_BLE
