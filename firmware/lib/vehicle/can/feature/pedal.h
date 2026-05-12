#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/pedal.h
 * @brief Pedal mode control via CAN frame 0x334 (drive config)
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "vehicle/can/fwd.h"

/**
 * @brief Available accelerator pedal response modes
 */
enum PedalMode
{
	PEDAL_STANDARD = 0,  // Factory default pedal response
	PEDAL_CHILL = 1,     // Reduced acceleration sensitivity
	PEDAL_SPORT = 2      // Aggressive acceleration response
};

/**
 * @brief Set the accelerator pedal response mode via CAN burst on 0x334
 * @param mode Desired pedal response mode
 * @param lastDrive Pointer to the most recent 8-byte drive config frame payload
 * @param s Device state used for burst scheduling
 */
static void controlPedalMode(PedalMode mode, const uint8_t *lastDrive, State &s)
{
	Frame f;
	f.id = CAN_ID_DRIVE_CONFIG;
	f.dlc = 8;
	memcpy(f.data, lastDrive, 8);

	uint8_t value = 0;
	switch (mode)
	{
	case PEDAL_CHILL:
		value = 0x20;
		break;
	case PEDAL_SPORT:
		value = 0x40;
		break;
	case PEDAL_STANDARD:
		value = 0x00;
		break;
	}
	f.data[0] = (f.data[0] & ~0x60) | value;  // bits 5-6 encode pedal mode
	f.data[7] = driveChecksum(f.data, 8);      // recalculate trailing checksum

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

/**
 * @brief Execute a pedal mode command string
 * @param cmd Null-terminated command (e.g. "pedal:chill", "pedal:sport", "pedal:standard")
 * @param s Device state; must have a valid drive frame captured
 * @return true if the command was recognized and executed
 */
static bool executePedalCmd(const char *cmd, State &s)
{
	if (s.variant == LEGACY)
		return false;
	if (!s.hasDrive)
		return false;

	if (strcmp(cmd, "pedal:standard") == 0 || strcmp(cmd, "pedal:std") == 0)
	{
		controlPedalMode(PEDAL_STANDARD, s.lastDrive, s);
		return true;
	}
	if (strcmp(cmd, "pedal:chill") == 0)
	{
		controlPedalMode(PEDAL_CHILL, s.lastDrive, s);
		return true;
	}
	if (strcmp(cmd, "pedal:sport") == 0)
	{
		controlPedalMode(PEDAL_SPORT, s.lastDrive, s);
		return true;
	}
	return false;
}
