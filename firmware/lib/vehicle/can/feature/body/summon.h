#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/body/summon.h
 * @brief Autopark Summon System (ASS) frame mutation and command handlers
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "core/util/parse.h"

/**
 * @brief Set the summon active flag in a UI_vehicleControl frame (0x273).
 * @param f CAN frame to modify (must have dlc >= 1).
 * @param active true to activate summon, false to deactivate.
 */
inline void setSummonActive(Frame &f, bool active)
{
	if (f.dlc < 1)
		return;
	if (active)
		f.data[0] |= 0x10;		// Bit 4: summon active
	else
		f.data[0] &= ~0x10;
}

/**
 * @brief Set the summon direction in a UI_vehicleControl frame (0x273).
 * @param f CAN frame to modify (must have dlc >= 1).
 * @param dir Desired summon direction (SUMMON_FORWARD or SUMMON_REVERSE).
 */
inline void setSummonDirection(Frame &f, SummonDirection dir)
{
	if (f.dlc < 1)
		return;
	if (dir == SUMMON_REVERSE)
		f.data[0] |= 0x20;		// Bit 5: direction = reverse
	else
		f.data[0] &= ~0x20;
}

/**
 * @brief Set the summon mode (start/stop) in a UI_vehicleControl frame (0x273).
 * @param f CAN frame to modify (must have dlc >= 1).
 * @param mode Desired summon mode (SUMMON_START or SUMMON_STOP).
 */
inline void setSummonMode(Frame &f, SummonMode mode)
{
	if (f.dlc < 1)
		return;
	if (mode == SUMMON_START)
		f.data[0] |= 0x01;		// Bit 0: summon start
	else
		f.data[0] &= ~0x01;
}

/**
 * @brief Execute the summon injection enable/disable command.
 *
 * Parses "summon-inject:<on|off>" to control whether summon frame injection
 * is permitted. When injection is disabled, any active summon burst is stopped.
 * The setting is persisted to EEPROM/NVS.
 *
 * @param cmd Null-terminated command string (expected prefix "summon-inject:").
 * @param s Global state reference.
 * @return true if the command was recognized and executed successfully.
 */
static bool executeSummonInjectCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "summon-inject:", 14) == 0)
	{
		if (!s.features().summon)
			return false;
		if (!parseBoolCmd(cmd + 14, s.summonInject, s.summonInject))
			return false;
		if (!s.summonInject)
		{
			// Halt any in-progress summon burst when injection is disabled
			s.summonMode = SUMMON_STOP;
			s.summonRemaining = 0;
		}
		resetHandlerLogFlags();
		saveSettings(s);
		return true;
	}
	return false;
}

/**
 * @brief Execute a summon movement command.
 *
 * Accepts "summon", "summon:forward", "summon:fwd", "summon:reverse",
 * "summon:rev", or "summon:stop". Requires the summon feature to be enabled
 * and summon injection to be active (except for stop, which always works).
 * Initiates a 30-frame burst in the requested direction.
 *
 * @param cmd Null-terminated command string.
 * @param s Global state reference.
 * @return true if the command was recognized and executed successfully.
 */
static bool executeSummonCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "summon:stop") == 0)
	{
		if (!s.features().summon)
			return false;
		s.summonMode = SUMMON_STOP;
		s.summonRemaining = 0;
		return true;
	}

	if (strcmp(cmd, "summon") == 0 || strncmp(cmd, "summon:", 7) == 0)
	{
		if (!s.features().summon)
			return false;
		if (!s.summonInject)
			return false;
		if (!s.hasCtrl)
			return false;

		if (strncmp(cmd, "summon:", 7) == 0)
		{
			const char *dir = cmd + 7;
			if (strcmp(dir, "forward") == 0 || strcmp(dir, "fwd") == 0)
			{
				s.summonDirection = SUMMON_FORWARD;
			}
			else if (strcmp(dir, "reverse") == 0 || strcmp(dir, "rev") == 0)
			{
				s.summonDirection = SUMMON_REVERSE;
			}
			else
			{
				return false;
			}
		}

		s.summonMode = SUMMON_START;
		s.summonRemaining = 30;	// Burst length in frames
		return true;
	}

	return false;
}
