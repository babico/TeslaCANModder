#pragma once

/**
 * @file firmware/lib/vehicle/ble/local_name.h
 * @brief Derive the observed Tesla BLE advertisement Local Name from a VIN
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 *
 * Observed advert format (community summary evidence; no vehicle capture in
 * this repo): "S" + uppercase(first 8 hex chars of SHA-1(VIN)) + "C".
 * The SHA-1 provider is injected so this pure derivation is also testable in
 * the native host environment, where mbedTLS is not linked.
 */

#include <stdint.h>
#include <stddef.h>

namespace Tesla
{

/** @brief SHA-1 provider: fills the 20-byte digest and returns true on success. */
typedef bool (*Sha1Fn)(const uint8_t *data, size_t len, uint8_t digest[20]);

/**
 * @brief Derive the BLE advertisement Local Name used as the scan filter.
 * @param vin VIN bytes (need not be NUL-terminated)
 * @param vinLen VIN byte length
 * @param out Output buffer, at least 11 bytes (10 chars plus NUL)
 * @param sha1 SHA-1 provider (mbedTLS on device, independent reference in tests)
 * @return True if the name was derived and written to @p out.
 */
static bool teslaLocalName(const char *vin, size_t vinLen, char out[11], Sha1Fn sha1)
{
	if (vin == nullptr || vinLen == 0 || sha1 == nullptr || out == nullptr)
		return false;

	uint8_t digest[20] = {};
	if (!sha1(reinterpret_cast<const uint8_t *>(vin), vinLen, digest))
		return false;

	static const char hex[] = "0123456789ABCDEF";
	out[0] = 'S';
	// sha1(VIN)[:8] is eight HEX characters, i.e. four digest bytes.
	for (size_t i = 0; i < 4; ++i)
	{
		out[1 + 2 * i] = hex[(digest[i] >> 4) & 0x0F];
		out[2 + 2 * i] = hex[digest[i] & 0x0F];
	}
	out[9] = 'C';
	out[10] = '\0';
	return true;
}

} // namespace Tesla
