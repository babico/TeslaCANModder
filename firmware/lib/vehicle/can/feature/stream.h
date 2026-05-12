#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/stream.h
 * @brief CAN frame streaming control command handler
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "core/util/parse.h"

/**
 * @brief Execute the stream enable/disable command.
 *
 * Parses "stream:<on|off>" and toggles real-time CAN frame streaming.
 * Resets the stream frame counter when transitioning from disabled to enabled.
 *
 * @param cmd Null-terminated command string (expected prefix "stream:").
 * @param s Global state reference to update streaming flags.
 * @return true if the command was recognized and executed successfully.
 */
static bool executeStreamCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "stream:", 7) == 0)
	{
		bool prev = s.streamEnabled;
		if (!parseBoolCmd(cmd + 7, s.streamEnabled, s.streamEnabled))
			return false;
		if (s.streamEnabled && !prev)
			s.streamCount = 0;	// Reset counter on fresh enable
		return true;
	}
	return false;
}
