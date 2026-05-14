#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/body/window.h
 * @brief Window vent control via CAN ID 0x119 with burst-send support
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <stdlib.h>
#include "vehicle/can/fwd.h"

/**
 * @brief Window vent position constants
 */
enum WindowVentPosition
{
	WINDOW_VENT_CLOSE = 0,    // Fully closed
	WINDOW_VENT_OPEN = 100    // Fully open (vent)
};

/**
 * @brief Send a window vent command at the given position
 * @param pos Target vent position (0-100), clamped to valid range.
 * @param s Device state used for burst-send scheduling.
 * @note Burst-sends 30 frames at 20 ms intervals on BUS_BODY to ensure ECU acceptance.
 */
static void controlWindowVent(uint8_t pos, State &s)
{
	if (pos > 100)
		pos = 100;
	Frame f;
	f.id = CAN_ID_WINDOW_VENT;
	f.dlc = 2;
	f.data[0] = 0x1F;  // All-window mask (all four windows targeted)
	f.data[1] = pos;

	startBurst(s, f, BUS_BODY, 30, 20);
}

/**
 * @brief Execute window vent commands
 * @param cmd The command string to match (e.g., "window:vent:open", "vent:close", "window:vent:N").
 * @param s Device state for burst-send and variant check.
 * @return True if the command was recognized and executed, false otherwise.
 * @note Legacy vehicle variants are not supported and will return false.
 */
static bool executeWindowCmd(const char *cmd, State &s)
{
	if (s.variant == LEGACY)
		return false;

	if (strcmp(cmd, "window:vent:open") == 0 || strcmp(cmd, "vent:open") == 0)
	{
		controlWindowVent(WINDOW_VENT_OPEN, s);
		return true;
	}
	if (strcmp(cmd, "window:vent:close") == 0 || strcmp(cmd, "vent:close") == 0)
	{
		controlWindowVent(WINDOW_VENT_CLOSE, s);
		return true;
	}
	// Arbitrary position: "window:vent:N" where N is 0-100
	if (strncmp(cmd, "window:vent:", 12) == 0)
	{
		const char *valStr = cmd + 12;
		if (*valStr == '\0')
			return false;
		char *end;
		long val = strtol(valStr, &end, 10);
		if (*end != '\0')
			return false;  // Reject trailing non-digit characters
		if (val < 0 || val > 100)
			return false;
		controlWindowVent((uint8_t)val, s);
		return true;
	}
	return false;
}
