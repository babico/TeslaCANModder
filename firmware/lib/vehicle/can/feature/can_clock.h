#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/can_clock.h
 * @brief MCP2515 oscillator clock profile selection command
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"

/**
 * @brief Execute the CAN clock profile command (canclock:auto|8|12|16|20)
 *
 * Sets the requested MCP2515 oscillator frequency. The value 0 means auto-detect.
 * 12 MHz is accepted as a compatibility alias and resolved by the driver fallback logic.
 *
 * @param cmd Command string beginning with "canclock:"
 * @param s Device state; canClockReqMHz is updated and persisted on success
 * @return True if the command was recognized and executed
 */
static bool executeCanClockCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "canclock:", 9) != 0)
		return false;

	const char *arg = cmd + 9; // pointer past the "canclock:" prefix
	if (strcmp(arg, "auto") == 0)
	{
		s.canClockReqMHz = 0; // 0 = auto-detect oscillator
		saveSettings(s);
		return true;
	}

	int mhz = atoi(arg);
	if (mhz != 8 && mhz != 12 && mhz != 16 && mhz != 20) // only valid MCP2515 crystal values
		return false;
	s.canClockReqMHz = (uint8_t)mhz;
	saveSettings(s);
	return true;
}
