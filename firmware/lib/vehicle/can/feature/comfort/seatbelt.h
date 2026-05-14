#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/comfort/seatbelt.h
 * @brief Rear seatbelt buckle emulation via periodic CAN frame injection
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/types.h"
#include "vehicle/can/ids.h"

/**
 * @brief Enable or disable rear seatbelt buckle emulation
 * @param s Device state reference
 * @param enable True to activate emulation, false to deactivate
 */
inline void controlSeatbeltEmulation(State &s, bool enable)
{
	s.seatbeltEmulation = enable;
}

/**
 * @brief Periodic tick that injects a seatbelt-status frame when emulation is active
 * @param s Device state reference
 *
 * @note Transmits at 500 ms intervals on the vehicle bus. The injected frame
 *       signals all three rear seatbelts as buckled, suppressing dashboard warnings.
 */
inline void seatbeltEmulationTick(State &s)
{
	if (!s.seatbeltEmulation)
		return;
	if (!s.apGateOpen())
		return;
	if (s.txPaused)
		return;
	unsigned long now = millis();
	if (now - s.seatbeltLastMs < 500)
		return;
	s.seatbeltLastMs = now;
	Frame f;
	f.id = CAN_ID_SEATBELT_STATUS;
	f.dlc = 8;
	memset(f.data, 0, 8);
	// Bits [2:0] = rear-left | rear-center | rear-right buckled
	f.data[0] = 0x07;
	driverSend(f, BUS_VEHICLE);
}

/**
 * @brief Execute a seatbelt emulation command string
 * @param cmd Null-terminated command (e.g. "seatbelt:on" or "seatbelt:off")
 * @param s Device state reference
 * @return True if the command was recognized and handled
 */
static bool executeSeatbeltCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "seatbelt:on") == 0)
	{
		controlSeatbeltEmulation(s, true);
		return true;
	}
	if (strcmp(cmd, "seatbelt:off") == 0)
	{
		controlSeatbeltEmulation(s, false);
		return true;
	}
	return false;
}
