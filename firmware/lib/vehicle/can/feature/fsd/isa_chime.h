#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/fsd/isa_chime.h
 * @brief HW4 ISA speed chime suppression feature and checksum computation
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "core/util/parse.h"
#include "vehicle/can/checksum.h"

/**
 * @brief Execute the ISA speed chime suppress command.
 *
 * Parses "isa-chime:<on|off>" and toggles the chime suppression flag.
 * Only supported on HW4 variants with the ISA chime feature enabled.
 *
 * @param cmd Null-terminated command string.
 * @param s Vehicle state to modify.
 * @return True if the command was recognized and executed successfully.
 */
static bool executeIsaChimeCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "isa-chime:", 10) == 0)
	{
		if (s.variant != HW4)
			return false; // HW4-only: ISA handler not present on HW3/Legacy
		if (!s.features().isaChime)
			return false;
		if (!parseBoolCmd(cmd + 10, s.isaChimeSuppress, s.isaChimeSuppress))
			return false;
		resetHandlerLogFlags();
		saveSettings(s);
		applyFilters(s);
		return true;
	}
	return false;
}
