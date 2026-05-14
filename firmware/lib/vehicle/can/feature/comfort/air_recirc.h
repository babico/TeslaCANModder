#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/comfort/air_recirc.h
 * @brief Automatic cabin air recirculation control via CAN
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/types.h"
#include "vehicle/can/ids.h"

/**
 * @brief Set the air recirculation request bit in a CAN frame.
 * @param f CAN frame to modify (byte[0] bit 0 controls recirc request).
 * @param recirc True to request recirculation, false for fresh air.
 */
inline void setAirRecircRequest(Frame &f, bool recirc)
{
	if (recirc)
		f.data[0] |= 0x01; // Bit 0 = recirculation request (1=recirc)
	else
		f.data[0] &= ~0x01; // Clear bit 0 (0=fresh air)
}

/**
 * @brief Build and burst-send an air recirculation command frame.
 * @param s Device state; must have climate data available.
 * @param enable True to enable recirculation, false to disable.
 */
inline void controlAirRecirc(State &s, bool enable)
{
	if (!s.hasClimate)
		return;
	Frame f;
	f.id = CAN_ID_AIR_RECIRC; // 0x2AA — climate recirculation control
	f.dlc = 8;
	memcpy(f.data, s.lastClimate, 5); // Preserve current climate state bytes
	memset(f.data + 5, 0, 3); // Zero padding bytes 5-7
	setAirRecircRequest(f, enable);
	startBurst(s, f, BUS_VEHICLE, 20, 20); // 20 frames at 20ms interval for ECU acceptance
}

/**
 * @brief Execute an air recirculation command string.
 * @param cmd Command string ("airecirc:on" or "airecirc:off").
 * @param s Device state.
 * @return True if the command was recognized and executed.
 */
static bool executeAirRecircCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "airecirc:on") == 0)
	{
		controlAirRecirc(s, true);
		return true;
	}
	if (strcmp(cmd, "airecirc:off") == 0)
	{
		controlAirRecirc(s, false);
		return true;
	}
	return false;
}
