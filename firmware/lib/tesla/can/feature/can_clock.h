#pragma once
#include "core/forward.h"

// ── MCP2515 Clock Profile Command ──────────────────────────────────────────
// canclock:auto|8|12|16|20
// 12MHz is accepted as a compatibility request and resolved by driver fallback.
bool executeCanClockCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "canclock:", 9) != 0)
		return false;

	const char *arg = cmd + 9;
	if (strcmp(arg, "auto") == 0)
	{
		s.canClockReqMHz = 0;
		saveSettings(s);
		return true;
	}

	int mhz = atoi(arg);
	if (mhz != 8 && mhz != 12 && mhz != 16 && mhz != 20)
		return false;
	s.canClockReqMHz = (uint8_t)mhz;
	saveSettings(s);
	return true;
}
