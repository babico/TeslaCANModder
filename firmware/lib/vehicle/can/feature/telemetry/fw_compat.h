#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/telemetry/fw_compat.h
 * @brief Vehicle firmware version decoding and compatibility checking via CAN ID 0x392
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/types.h"
#include "vehicle/can/ids.h"

/**
 * @brief Firmware compatibility level indicating whether the detected vehicle
 *        firmware is fully supported, partially supported, or incompatible.
 */
enum FwCompatLevel : uint8_t
{
	FW_COMPAT_UNKNOWN = 0, // Version not yet decoded
	FW_COMPAT_OK = 1,      // Fully compatible
	FW_COMPAT_WARN = 2,    // Legacy protocol, limited features
	FW_COMPAT_FAIL = 3     // Known-incompatible combination
};

/**
 * @brief Get a human-readable name for a firmware compatibility level.
 * @param level Firmware compatibility level enum value.
 * @return Null-terminated string ("OK", "WARN", "FAIL", or "UNKNOWN").
 */
inline const char *fwCompatName(FwCompatLevel level)
{
	switch (level)
	{
	case FW_COMPAT_OK:
		return "OK";
	case FW_COMPAT_WARN:
		return "WARN";
	case FW_COMPAT_FAIL:
		return "FAIL";
	default:
		return "UNKNOWN";
	}
}

/**
 * @brief Decode firmware version from CAN ID 0x392 (GTW_versionInfo) and evaluate
 *        compatibility against known firmware/hardware combinations.
 *
 * Frame layout (mux field = byte 0 bits [2:0]):
 *   mux 0: bytes 1-2 = year (big-endian uint16), byte 3 = release, byte 4 = minor
 *   mux 1: bytes 1-4 = build number (big-endian uint32)
 *
 * @param f CAN frame from ID 0x392.
 * @param s Global state to populate with decoded version and compatibility level.
 */
inline void decodeFwVersion(const Frame &f, State &s)
{
	if (f.dlc < 5)
		return;
	uint8_t mux = f.data[0] & 0x07; // bits 2:0 — mux selector
	if (mux == 0)
	{
		s.fwYear = ((uint16_t)f.data[1] << 8) | f.data[2];    // bytes 1-2: year (BE)
		s.fwRelease = f.data[3];                                // byte 3: release number
		s.fwMinor = f.data[4];                                  // byte 4: minor version
		s.hasFwVersion = true;

		// Evaluate compatibility based on year/release/variant
		if (s.fwYear == 2026 && s.fwRelease >= 8)
		{
			// FSD v13 legacy protocol — limited feature set on HW4
			s.fwCompat = (s.variant == HW4) ? FW_COMPAT_WARN : FW_COMPAT_OK;
		}
		else if (s.fwYear == 2026 && s.fwRelease == 2 && s.fwMinor >= 9)
		{
			// FSD v14 — fully compatible with HW4
			s.fwCompat = FW_COMPAT_OK;
		}
		else if (s.fwYear >= 2026)
		{
			s.fwCompat = FW_COMPAT_OK;
		}
		else
		{
			s.fwCompat = FW_COMPAT_WARN;
		}
	}
	else if (mux == 1)
	{
		// Mux 1: 32-bit build number (big-endian)
		s.fwBuild = ((uint32_t)f.data[1] << 24) | ((uint32_t)f.data[2] << 16) |
					((uint32_t)f.data[3] << 8) | f.data[4];
	}
}

/**
 * @brief Execute the "fwcompat" query command to report firmware version and status.
 * @param cmd Full command string (must be exactly "fwcompat").
 * @param s Global state containing decoded firmware version data.
 * @return True if the command matched.
 */
static bool executeFwCompatCmd(const char *cmd, State &s)
{
	return strcmp(cmd, "fwcompat") == 0;
}
