#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/fsd/fsd.h
 * @brief FSD (Full Self-Driving) enable/disable and force-enable command handlers
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "core/util/parse.h"

/**
 * @brief Execute the "fsd:<on|off>" command to toggle FSD modifications.
 * @param cmd Full command string (expected prefix "fsd:").
 * @param s Global state containing FSD enable flag and feature gates.
 * @return True if the command was recognized and executed successfully.
 */
static bool executeFsdCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "fsd:", 4) == 0)
	{
		if (!s.features().fsd)
			return false;
		if (!parseBoolCmd(cmd + 4, s.fsdEnabled, s.fsdEnabled))
			return false;
		resetHandlerLogFlags();
		saveSettings(s);
		applyFilters(s);
		return true;
	}
	return false;
}

/**
 * @brief Execute the "fsd:force:<on|off>" command to toggle forced FSD application.
 *
 * When force-enabled, FSD modifications are applied regardless of the UI FSD
 * selection bit state.
 *
 * @param cmd Full command string (expected prefix "fsd:force:").
 * @param s Global state containing FSD force-enable flag and feature gates.
 * @return True if the command was recognized and executed successfully.
 */
static bool executeFsdForceCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "fsd:force:", 10) == 0)
	{
		if (!s.features().fsdForce)
			return false;
		if (!parseBoolCmd(cmd + 10, s.fsdForceEnabled, s.fsdForceEnabled))
			return false;
		resetHandlerLogFlags();
		saveSettings(s);
		return true;
	}
	return false;
}
