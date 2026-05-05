#pragma once
#include "core/forward.h"

// ── HW3 Speed Offset ─────────────────────────────────────────────────────────
// Reads UI offset steps from CAN and writes computed speed offset.

inline void writeHW3SpeedOffset(Frame &f, int offset)
{
	if (f.dlc < 2)
		return;
	f.data[0] = (f.data[0] & ~0xC0) | ((offset & 0x03) << 6);
	f.data[1] = (f.data[1] & ~0x3F) | (offset >> 2);
}

inline int readHW3UiOffsetSteps(const Frame &f)
{
	return f.dlc >= 4 ? (int)((f.data[3] >> 1) & 0x3F) - 30 : 0;
}

inline int calculateHW3SpeedOffset(int steps)
{
	int val = steps * 5;
	if (val < 0)
		return 0;
	if (val > 100)
		return 100;
	return val;
}

// ── Offset Command (offset:N, offset:auto, offset:off) ──────────────────────
// Unified: HW4 accepts 0–63, HW3 accepts 0–100. Legacy rejected.
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
