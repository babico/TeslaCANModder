#pragma once
#include "core/forward.h"
#include "infra/parse.h"

// ── CAN Raw Command Execution ───────────────────────────────────────────────
// Enables/disables raw (unfiltered) CAN frame listening

static bool executeCanRawCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "can:raw:", 8) == 0)
	{
		if (!parseBoolCmd(cmd + 8, s.rawCanListen, s.rawCanListen))
			return false;
		applyFilters(s);
		return true;
	}
	return false;
}
