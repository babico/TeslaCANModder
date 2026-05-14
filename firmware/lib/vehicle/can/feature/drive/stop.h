#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/drive/stop.h
 * @brief Stop mode control (creep, roll, hold) via drive config CAN frame mutation
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "vehicle/can/fwd.h"

/**
 * @brief Available stopping behavior modes
 */
enum StopMode
{
	STOP_CREEP = 0,  // Vehicle creeps forward when brake is released
	STOP_ROLL = 1,   // Vehicle rolls freely when brake is released
	STOP_HOLD = 2    // Vehicle holds position when brake is released
};

/**
 * @brief Inject a drive config frame with the requested stop mode
 * @param mode Desired stopping behavior
 * @param lastDrive Pointer to the most recently captured drive config frame data
 * @param s Device state reference
 *
 * @note Mutates byte 5 bits [1:0] of the drive config frame and recalculates
 *       the checksum in byte 7 before transmitting as a burst on the vehicle bus.
 */
static void controlStopMode(StopMode mode, const uint8_t *lastDrive, State &s)
{
	Frame f;
	f.id = CAN_ID_DRIVE_CONFIG;
	f.dlc = 8;
	memcpy(f.data, lastDrive, 8);

	// Set bits [1:0] of byte 5 to the stop mode value
	f.data[5] = (f.data[5] & ~0x03) | (mode & 0x03);
	// Recalculate frame checksum after modification
	f.data[7] = driveChecksum(f.data, 8);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

/**
 * @brief Execute a stop mode command string
 * @param cmd Null-terminated command (e.g. "stop:creep", "stop:roll", "stop:hold")
 * @param s Device state reference
 * @return True if the command was recognized and handled
 *
 * @note Rejected on LEGACY variant vehicles or when no drive frame has been captured.
 */
static bool executeStopCmd(const char *cmd, State &s)
{
	if (s.variant == LEGACY)
		return false;
	if (!s.hasDrive)
		return false;

	if (strcmp(cmd, "stop:creep") == 0)
	{
		controlStopMode(STOP_CREEP, s.lastDrive, s);
		return true;
	}
	if (strcmp(cmd, "stop:roll") == 0)
	{
		controlStopMode(STOP_ROLL, s.lastDrive, s);
		return true;
	}
	if (strcmp(cmd, "stop:hold") == 0)
	{
		controlStopMode(STOP_HOLD, s.lastDrive, s);
		return true;
	}
	return false;
}
