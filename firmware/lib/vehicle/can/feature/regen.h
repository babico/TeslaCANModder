#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/regen.h
 * @brief Regenerative braking level control via CAN drive config frame
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "vehicle/can/fwd.h"

/**
 * @brief Send a CAN burst to set the regenerative braking level
 * @param level Regen level value (0-200, clamped if exceeding 200)
 * @param lastDrive Pointer to the most recent 8-byte drive config payload
 * @param s Global vehicle state used for burst transmission
 *
 * @note Constructs a drive config frame (0x334) with the specified regen level
 *       in byte 2, recalculates the checksum in byte 7, and transmits as a burst
 *       on the vehicle bus.
 */
static void controlRegenLevel(uint8_t level, const uint8_t *lastDrive, State &s)
{
	Frame f;
	f.id = CAN_ID_DRIVE_CONFIG;
	f.dlc = 8;
	memcpy(f.data, lastDrive, 8);

	if (level > 200)
		level = 200;
	f.data[2] = level;
	f.data[7] = driveChecksum(f.data, 8);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

/**
 * @brief Execute a regen braking command
 * @param cmd Command string (e.g. "regen:off", "regen:low", "regen:standard", "regen:max")
 * @param s Global vehicle state
 * @return True if the command was recognized and executed
 *
 * @note Only supported on non-legacy variants when drive config data is available.
 *       Level mapping: off=0, low=50, standard=100, max=200.
 */
static bool executeRegenCmd(const char *cmd, State &s)
{
	if (s.variant == LEGACY)
		return false;
	if (!s.hasDrive)
		return false;

	if (strcmp(cmd, "regen:off") == 0)
	{
		controlRegenLevel(0, s.lastDrive, s);
		return true;
	}
	if (strcmp(cmd, "regen:low") == 0)
	{
		controlRegenLevel(50, s.lastDrive, s);
		return true;
	}
	if (strcmp(cmd, "regen:standard") == 0 || strcmp(cmd, "regen:std") == 0)
	{
		controlRegenLevel(100, s.lastDrive, s);
		return true;
	}
	if (strcmp(cmd, "regen:max") == 0)
	{
		controlRegenLevel(200, s.lastDrive, s);
		return true;
	}
	return false;
}
