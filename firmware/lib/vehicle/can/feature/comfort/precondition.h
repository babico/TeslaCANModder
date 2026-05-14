#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/comfort/precondition.h
 * @brief Battery preconditioning control via CAN frame 0x082 (UI_tripPlanning)
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "vehicle/can/fwd.h"
#include "core/util/parse.h"

/**
 * @brief Inject a battery preconditioning request frame on the vehicle bus
 * @param enable true to start preconditioning (heats battery for charging), false to stop
 * @param s Device state used for burst scheduling
 * @note Requires periodic re-injection at ~500 ms intervals while active
 */
static void controlPrecondition(bool enable, State &s)
{
	Frame f;
	f.id = CAN_ID_PRECONDITION;
	f.dlc = 8;
	memset(f.data, 0, 8);
	f.data[0] = enable ? 0x05 : 0x00;  // 0x05 triggers battery heating
	startBurst(s, f, BUS_VEHICLE, 1, 0);
}

/**
 * @brief Execute a preconditioning command string
 * @param cmd Null-terminated command (e.g. "precondition:on", "precondition:off")
 * @param s Device state; precondition settings are persisted on change
 * @return true if the command was recognized and executed
 */
static bool executePreconditionCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "precondition:", 13) == 0)
	{
		if (s.variant == LEGACY)
			return false;
		if (!parseBoolCmd(cmd + 13, s.preconditionEnabled, s.preconditionEnabled))
			return false;
		if (s.preconditionEnabled)
		{
			controlPrecondition(true, s);
			s.precondLastMs = millis();
		}
		else
		{
			controlPrecondition(false, s);
		}
		saveSettings(s);
		return true;
	}
	return false;
}
