#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/drive/drive_mode.h
 * @brief Drive mode override ("Ghost Mode") via CAN ID 0x334 injection
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "vehicle/can/ids.h"

#define CAN_ID_DRIVE_MODE 0x334
#define DRIVE_MODE_INTERVAL_MS 50

/**
 * @brief Available drive mode override values for CAN injection.
 */
enum DriveMode
{
	DRIVE_MODE_NONE = 0,	   // No injection (pass-through)
	DRIVE_MODE_CHILL = 1,	   // Chill mode — reduced torque response
	DRIVE_MODE_STANDARD = 2,   // Standard mode — default torque mapping
	DRIVE_MODE_PERFORMANCE = 3 // Performance mode — maximum torque response
};

/**
 * @brief Get a human-readable name for a drive mode value.
 * @param mode Drive mode enum value.
 * @return Null-terminated string name ("chill", "standard", "performance", or "none").
 */
inline const char *driveModeName(uint8_t mode)
{
	switch (mode)
	{
	case DRIVE_MODE_CHILL:
		return "chill";
	case DRIVE_MODE_STANDARD:
		return "standard";
	case DRIVE_MODE_PERFORMANCE:
		return "performance";
	default:
		return "none";
	}
}

/**
 * @brief Parse a drive mode name string into its numeric enum value.
 * @param name Null-terminated mode name ("chill", "standard", "performance", "off", "none").
 * @param out Output parameter receiving the parsed DriveMode value.
 * @return True if the name was recognized and parsed successfully.
 */
inline bool parseDriveMode(const char *name, uint8_t &out)
{
	if (strcmp(name, "chill") == 0)
	{
		out = DRIVE_MODE_CHILL;
		return true;
	}
	if (strcmp(name, "standard") == 0)
	{
		out = DRIVE_MODE_STANDARD;
		return true;
	}
	if (strcmp(name, "performance") == 0)
	{
		out = DRIVE_MODE_PERFORMANCE;
		return true;
	}
	if (strcmp(name, "off") == 0 || strcmp(name, "none") == 0)
	{
		out = DRIVE_MODE_NONE;
		return true;
	}
	return false;
}

/**
 * @brief Build a drive mode injection frame for CAN ID 0x334.
 * @param mode Target drive mode to inject.
 * @param lastDrive Pointer to the last cached 8-byte drive config frame payload.
 * @return Fully constructed Frame ready for transmission on the vehicle bus.
 */
inline Frame buildDriveModeFrame(uint8_t mode, const uint8_t *lastDrive)
{
	Frame f;
	f.id = CAN_ID_DRIVE_MODE;
	f.dlc = 8;
	// Copy base payload from last cached drive config frame
	for (uint8_t i = 0; i < 8; i++)
		f.data[i] = lastDrive[i];
	// Map mode enum to 2-bit CAN encoding for byte[0] bits[6:5]
	uint8_t modeBits = 0;
	switch (mode)
	{
	case DRIVE_MODE_CHILL:
		modeBits = 0x00; // 00 = Chill
		break;
	case DRIVE_MODE_STANDARD:
		modeBits = 0x01; // 01 = Standard
		break;
	case DRIVE_MODE_PERFORMANCE:
		modeBits = 0x02; // 10 = Performance
		break;
	default:
		modeBits = 0x01; // Default to Standard
		break;
	}
	f.data[0] = (f.data[0] & ~0x60) | ((modeBits & 0x03) << 5); // bits 6:5 of byte 0
	f.data[7] = driveChecksum(f.data, 8);						// Recalculate frame checksum
	return f;
}

/**
 * @brief Periodic tick for drive mode injection, called from the main loop.
 * @param s Global state containing drive mode override settings and timing.
 * @param now Current timestamp in milliseconds.
 */
inline void driveModeTick(State &s, unsigned long now)
{
	if (s.driveModeOverride == DRIVE_MODE_NONE)
		return;
	if (!s.hasDrive)
		return;
	if (!s.apGateOpen())
		return;
	if (now - s.driveModeLastMs < DRIVE_MODE_INTERVAL_MS)
		return;
	s.driveModeLastMs = now;
	Frame f = buildDriveModeFrame(s.driveModeOverride, s.lastDrive);
	driverSend(f, BUS_VEHICLE);
}

/**
 * @brief Execute the "drivemode:<mode>" command to set the drive mode override.
 * @param cmd Full command string (expected prefix "drivemode:").
 * @param s Global state to update with the new drive mode.
 * @return True if the command was recognized and executed successfully.
 */
static bool executeDriveModeCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "drivemode:", 10) != 0)
		return false;
	const char *arg = cmd + 10;
	uint8_t mode;
	if (!parseDriveMode(arg, mode))
		return false;
	s.driveModeOverride = mode;
	s.driveModeLastMs = 0;
	saveSettings(s);
	return true;
}
