#pragma once
#include "core/forward.h"
#include "infra/util/parse.h"

// ── Stream Command Execution ────────────────────────────────────────────────
// Enables/disables real-time CAN frame streaming

static bool executeStreamCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "stream:", 7) == 0)
	{
		bool prev = s.streamEnabled;
		if (!parseBoolCmd(cmd + 7, s.streamEnabled, s.streamEnabled))
			return false;
		if (s.streamEnabled && !prev)
			s.streamCount = 0;
		return true;
	}
	return false;
}
