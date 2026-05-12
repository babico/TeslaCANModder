#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/isa_chime.h
 * @brief HW4 ISA speed chime suppression feature and checksum computation
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "core/util/parse.h"

/**
 * @brief Compute the HW4 ISA speed message checksum.
 *
 * The checksum occupies byte 7 and is calculated as the sum of bytes 0-6
 * plus both bytes of the CAN identifier.
 *
 * @param f CAN frame to compute the checksum for.
 * @return Computed checksum byte, or 0 if the frame DLC is too short.
 */
inline uint8_t computeHW4IsaChecksum(const Frame &f)
{
	if (f.dlc < 8)
		return 0;
	uint8_t sum = 0;
	for (int i = 0; i < 7; i++)
		sum += f.data[i];
	sum += (f.id & 0xFF) + (f.id >> 8); // Add both bytes of the 11-bit CAN ID
	return sum;
}

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
