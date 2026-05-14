#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/body/track_mode.h
 * @brief Track Mode enable/disable via CAN 0x313 injection
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "vehicle/can/fwd.h"
#include "core/util/parse.h"

/**
 * @brief Inject a CAN frame to enable or disable Track Mode.
 *
 * Modifies CAN ID 0x313 (UI_trackModeSettings) byte[0] bits 1:0.
 * Setting bits to 0x01 enables Track Mode; clearing them disables it.
 *
 * @param enable True to enable Track Mode, false to disable.
 * @param s Device state used for burst transmission.
 */
static void controlTrackMode(bool enable, State &s)
{
	Frame f;
	f.id = CAN_ID_TRACK_MODE;
	f.dlc = 8;
	memset(f.data, 0, 8);
	if (enable)
	{
		f.data[0] = (f.data[0] & 0xFC) | 0x01; // bits 1:0 = 01 (enable)
	}
	startBurst(s, f, BUS_VEHICLE, 20, 20);
}

/**
 * @brief Execute a "trackmode:" command to toggle Track Mode.
 *
 * Parses the boolean suffix, updates state, injects the CAN frame,
 * and persists the new setting. Rejected on LEGACY variant.
 *
 * @param cmd Null-terminated command string (e.g. "trackmode:on").
 * @param s Device state for variant check, persistence, and burst.
 * @return True if the command was recognized and executed successfully.
 */
static bool executeTrackModeCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "trackmode:", 10) == 0)
	{
		if (s.variant == LEGACY)
			return false;
		if (!parseBoolCmd(cmd + 10, s.trackModeEnabled, s.trackModeEnabled))
			return false;
		controlTrackMode(s.trackModeEnabled, s);
		saveSettings(s);
		return true;
	}
	return false;
}
