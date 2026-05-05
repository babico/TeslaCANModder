#pragma once
#include "core/forward.h"
#include "core/util/parse.h"

// ── FSD Enable/Disable Command ───────────────────────────────────────────────
bool executeFsdCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "fsd:", 4) == 0)
	{
		if (!s.features().fsd)
			return false;
		if (!parseBoolCmd(cmd + 4, s.fsdEnabled, s.fsdEnabled))
			return false;
		resetHandlerLogFlags();
		saveSettings(s);
		applyFilters(s);
		return true;
	}
	return false;
}

// ── FSD Force Enable/Disable Command ────────────────────────────────────────
// When enabled, FSD modifications are applied even if UI FSD selection bit is off.
bool executeFsdForceCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "fsd:force:", 10) == 0)
	{
		if (!s.features().fsdForce)
			return false;
		if (!parseBoolCmd(cmd + 10, s.fsdForceEnabled, s.fsdForceEnabled))
			return false;
		resetHandlerLogFlags();
		saveSettings(s);
		return true;
	}
	return false;
}
