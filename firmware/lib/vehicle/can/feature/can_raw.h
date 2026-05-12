#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/can_raw.h
 * @brief Command handler for enabling/disabling raw (unfiltered) CAN frame listening
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "core/util/parse.h"

/**
 * @brief Execute the raw CAN listening toggle command (can:raw:on|off|toggle)
 *
 * Enables or disables unfiltered CAN frame reception. When raw mode is active,
 * all frames are forwarded without ID-based filtering. Filter state is reapplied
 * after the toggle.
 *
 * @param cmd Command string beginning with "can:raw:"
 * @param s Device state; rawCanListen is updated and filters are reapplied
 * @return True if the command was recognized and executed
 */
static bool executeCanRawCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "can:raw:", 8) == 0)
	{
		if (!parseBoolCmd(cmd + 8, s.rawCanListen, s.rawCanListen))
			return false;
		applyFilters(s); // reapply hardware filters to reflect new raw mode state
		return true;
	}
	return false;
}
