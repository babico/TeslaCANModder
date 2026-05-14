#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/fsd/offsets.h
 * @brief HW3/HW4 speed offset reading, writing, and command handling
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"

/**
 * @brief Write a computed speed offset value into the CAN frame.
 * @param f Mutable reference to the CAN frame.
 * @param offset Speed offset value to encode across bytes 0-1.
 */
inline void writeHW3SpeedOffset(Frame &f, int offset)
{
	if (f.dlc < 2)
		return;
	// Offset encoded as: 2 bits in byte 0 bits[7:6], remaining bits in byte 1 bits[5:0]
	f.data[0] = (f.data[0] & ~0xC0) | ((offset & 0x03) << 6);
	f.data[1] = (f.data[1] & ~0x3F) | (offset >> 2);
}

/**
 * @brief Read the UI offset steps from a CAN frame.
 * @param f Const reference to the CAN frame.
 * @return Signed step count (-30 to +33 range), or 0 if frame too short.
 */
inline int readHW3UiOffsetSteps(const Frame &f)
{
	// 6-bit field in byte 3 bits[6:1], biased by -30 to center at zero
	return f.dlc >= 4 ? (int)((f.data[3] >> 1) & 0x3F) - 30 : 0;
}

/**
 * @brief Convert UI offset steps to a speed offset value.
 * @param steps Signed step count from readHW3UiOffsetSteps.
 * @return Speed offset clamped to [0, 100].
 */
inline int calculateHW3SpeedOffset(int steps)
{
	int val = steps * 5;
	if (val < 0)
		return 0;
	if (val > 100)
		return 100;
	return val;
}

/**
 * @brief Execute an offset command (set value, auto mode, or disable).
 * @param cmd Raw command string starting with "offset:".
 * @param s Mutable reference to the global state.
 * @return True if the command was recognized and applied successfully.
 */
static bool executeOffsetCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "offset:", 7) != 0)
		return false;
	if (!s.features().offset)
		return false;
	const char *arg = cmd + 7;

	if (strcmp(arg, "auto") == 0)
	{
		s.offsetOverride = false;
		resetHandlerLogFlags();
		saveSettings(s);
		return true;
	}

	if (strcmp(arg, "off") == 0 || strcmp(arg, "0") == 0)
	{
		s.speedOffset = 0;
		s.offsetOverride = true;
		resetHandlerLogFlags();
		saveSettings(s);
		return true;
	}

	int val = atoi(arg);
	if (val < 0)
		return false;
	// HW4 range 0-63, HW3 range 0-100
	int maxVal = (s.detectedHW == 3 || s.variant == HW4) ? 63 : 100;
	if (val > maxVal)
		return false;
	s.speedOffset = val;
	s.offsetOverride = true;
	resetHandlerLogFlags();
	saveSettings(s);
	return true;
}
