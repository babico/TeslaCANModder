#pragma once
#include "core/forward.h"
#include "infra/parse.h"

// ── HW4 ISA Speed Chime Checksum ─────────────────────────────────────────────
// Computes the ISA speed message checksum (byte 7 = sum of bytes 0-6 + ID).

inline uint8_t computeHW4IsaChecksum(const Frame &f)
{
	if (f.dlc < 8)
		return 0;
	uint8_t sum = 0;
	for (int i = 0; i < 7; i++)
		sum += f.data[i];
	sum += (f.id & 0xFF) + (f.id >> 8);
	return sum;
}

// ── ISA Speed Chime Suppress Command ─────────────────────────────────────────
bool executeIsaChimeCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "isa-chime:", 10) == 0)
	{
		if (s.variant != HW4)
			return false; // HW4-only: no ISA handler on HW3/Legacy
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
