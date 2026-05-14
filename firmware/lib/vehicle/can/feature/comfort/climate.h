#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/comfort/climate.h
 * @brief Climate keeper mode control via CAN 0x2F3 — toggle climate keep on/off.
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "vehicle/can/fwd.h"

/**
 * @brief Climate keeper mode values written to bits 33-34 of the 0x2F3 frame.
 */
enum ClimateMode
{
	CLIMATE_OFF = 0,  // Disable climate keeper
	CLIMATE_KEEP = 1  // Enable climate keeper (cabin conditioning)
};

/**
 * @brief Send a climate mode command by mutating the cached 0x2F3 frame and bursting it.
 * @param mode The desired climate keeper mode.
 * @param lastClimate Pointer to the last observed 0x2F3 frame payload (5 bytes).
 * @param s Global state reference used for burst transmission.
 */
static void controlClimate(ClimateMode mode, const uint8_t *lastClimate, State &s)
{
	Frame f;
	f.id = CAN_ID_CLIMATE;
	f.dlc = 5;
	memcpy(f.data, lastClimate, 5);

	// Modify bits 33-34: byte 4, bits 1-2 (mask 0x06)
	f.data[4] = (f.data[4] & ~0x06) | ((mode & 0x03) << 1);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

/**
 * @brief Execute a climate command string received from the client.
 * @param cmd Null-terminated command string (e.g. "climate:keep", "climate:off").
 * @param s Global state reference containing variant info and cached frames.
 * @return True if the command was recognized and executed, false otherwise.
 */
static bool executeClimateCmd(const char *cmd, State &s)
{
	if (s.variant == LEGACY)
		return false;
	if (!s.hasClimate)
		return false; // Need 0x2F3 frame cached before we can mutate it

	if (strcmp(cmd, "climate:keep") == 0)
	{
		controlClimate(CLIMATE_KEEP, s.lastClimate, s);
		return true;
	}
	if (strcmp(cmd, "climate:off") == 0)
	{
		controlClimate(CLIMATE_OFF, s.lastClimate, s);
		return true;
	}
	return false;
}
