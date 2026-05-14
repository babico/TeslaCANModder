#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/display.h
 * @brief Display brightness control via CAN 0x273 (UI_vehicleControl).
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <stdlib.h>
#include "vehicle/can/fwd.h"

/**
 * @brief Set the display brightness level in a UI_vehicleControl frame.
 * @param f Reference to the CAN frame to modify (must have dlc >= 5).
 * @param level Brightness level (0-127), encoded at bits 32-39 with factor 0.5.
 */
inline void setDisplayBrightness(Frame &f, uint8_t level)
{
	if (f.dlc < 5)
		return;
	f.data[4] = level; // bits 32-39, brightness factor 0.5
}

/**
 * @brief Build and burst a display brightness command on the vehicle bus.
 * @param level Brightness level (0-127).
 * @param s Global state reference used for frame construction and burst transmission.
 */
static void controlDisplayBrightness(uint8_t level, State &s)
{
	Frame f = makeCtrlFrame(s);
	setDisplayBrightness(f, level);

	startBurst(s, f, BUS_VEHICLE, 20, 20);
}

/**
 * @brief Execute a display command string received from the client.
 * @param cmd Null-terminated command string (e.g. "maindisplay:50").
 * @param s Global state reference containing variant info and cached frames.
 * @return True if the command was recognized and executed, false otherwise.
 */
static bool executeDisplayCmd(const char *cmd, State &s)
{
	if (!s.hasCtrl)
		return false;
	if (strncmp(cmd, "maindisplay:", 12) != 0)
		return false;

	int level = atoi(cmd + 12);
	if (level < 0 || level > 127)
		return false;

	controlDisplayBrightness((uint8_t)level, s);
	return true;
}
