#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/fsd/profile.h
 * @brief Speed profile encoding and command execution for FSD mux frames
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"

/**
 * @brief Write speed profile into the FSD mux frame for HW3/Legacy v12/v13
 * @param f CAN frame to modify (requires DLC >= 7)
 * @param profile Speed profile index (2-bit value, 0-3)
 */
inline void setSpeedProfileV12V13(Frame &f, int profile)
{
	if (f.dlc < 7)
		return;
	// Profile occupies bits [2:1] of byte 6
	f.data[6] = (f.data[6] & ~0x06) | ((profile & 0x03) << 1);
}

/**
 * @brief Write speed profile into the FSD mux frame for HW4
 * @param f CAN frame to modify (requires DLC >= 8)
 * @param profile Speed profile index (3-bit value, 0-7)
 */
inline void writeHW4SpeedProfile(Frame &f, int profile)
{
	if (f.dlc < 8)
		return;
	// Profile occupies bits [6:4] of byte 7
	f.data[7] = (f.data[7] & ~0x70) | ((profile & 0x07) << 4);
}

/**
 * @brief Map HW3 follow-distance CAN value to a speed profile index
 * @param fd Follow-distance value from CAN (0-3)
 * @return Speed profile index, or -1 if the value is invalid
 */
inline int mapHW3FollowDistToProfile(uint8_t fd)
{
	static const int8_t map[] = {-1, 2, 1, 0};
	return fd < 4 ? map[fd] : -1;
}

/**
 * @brief Map HW4 follow-distance CAN value to a speed profile index
 * @param fd Follow-distance value from CAN (0-5)
 * @return Speed profile index, or -1 if the value is invalid
 */
inline int mapHW4FollowDistToProfile(uint8_t fd)
{
	static const int8_t map[] = {-1, 3, 2, 1, 0, 4};
	return fd < 6 ? map[fd] : -1;
}

/**
 * @brief Execute a speed profile command
 * @param cmd Command string (e.g. "profile:N", "sp:N", "profile:auto", "profile:lock", "profile:unlock")
 * @param s Global vehicle state
 * @return True if the command was recognized and executed
 *
 * @note Accepts both "profile:" and "sp:" prefixes for convenience.
 *       "auto"/"unlock" disables override, "lock" enables override,
 *       numeric values 0-4 set the profile and enable override.
 */
static bool executeProfileCmd(const char *cmd, State &s)
{
	const char *arg = nullptr;
	if (strncmp(cmd, "profile:", 8) == 0)
		arg = cmd + 8;
	else if (strncmp(cmd, "sp:", 3) == 0)
		arg = cmd + 3;
	else
		return false;
	if (!s.features().profile)
		return false;

	if (strcmp(arg, "auto") == 0)
	{
		s.profileOverride = false;
		resetHandlerLogFlags();
		saveSettings(s);
		return true;
	}
	if (strcmp(arg, "lock") == 0)
	{
		s.profileOverride = true;
		resetHandlerLogFlags();
		saveSettings(s);
		return true;
	}
	if (strcmp(arg, "unlock") == 0)
	{
		s.profileOverride = false;
		resetHandlerLogFlags();
		saveSettings(s);
		return true;
	}
	int p = atoi(arg);
	if (p < 0 || p > 4)
		return false;
	s.speedProfile = p;
	s.profileOverride = true;
	resetHandlerLogFlags();
	saveSettings(s);
	return true;
}
