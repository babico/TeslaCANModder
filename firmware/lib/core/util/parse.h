#pragma once
#include <cstring>

// ── Boolean Command Parser ──────────────────────────────────────────────────
// Shared utility for on/off style commands

static bool parseBoolCmd(const char *suffix, bool current, bool &out)
{
	if (strcmp(suffix, "on") == 0)
	{
		out = true;
		return true;
	}
	if (strcmp(suffix, "off") == 0)
	{
		out = false;
		return true;
	}
	return false;
}
