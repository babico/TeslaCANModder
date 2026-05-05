#pragma once
#include "core/forward.h"
#include "core/util/parse.h"

// ── ASS (Autopark Summon System) Commands ────────────────────────────────────
// SummonDirection and SummonMode enums are defined in types.h

// ── ASS Summon Control (0x273 UI_vehicleControl) ─────────────────────────────
inline void setSummonActive(Frame &f, bool active)
{
	if (f.dlc < 1)
		return;
	if (active)
		f.data[0] |= 0x10; // bit 4
	else
		f.data[0] &= ~0x10;
}

inline void setSummonDirection(Frame &f, SummonDirection dir)
{
	if (f.dlc < 1)
		return;
	if (dir == SUMMON_REVERSE)
		f.data[0] |= 0x20; // bit 5
	else
		f.data[0] &= ~0x20;
}

inline void setSummonMode(Frame &f, SummonMode mode)
{
	if (f.dlc < 1)
		return;
	if (mode == SUMMON_START)
		f.data[0] |= 0x01; // bit 0
	else
		f.data[0] &= ~0x01;
}

// ── Summon Injection Enable/Disable Command ──────────────────────────────────
// Controls whether summon injection is allowed. Persisted to EEPROM/NVS.
static bool executeSummonInjectCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "summon-inject:", 14) == 0)
	{
		if (!s.features().summon)
			return false;
		if (!parseBoolCmd(cmd + 14, s.summonInject, s.summonInject))
			return false;
		// If injection is disabled, stop any active burst
		if (!s.summonInject)
		{
			s.summonMode = SUMMON_STOP;
			s.summonRemaining = 0;
		}
		resetHandlerLogFlags();
		saveSettings(s);
		return true;
	}
	return false;
}

// ── Summon Command (summon, summon:forward, summon:reverse, summon:stop) ─────
// Requires summonInject to be enabled (except for stop, which always works).
static bool executeSummonCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "summon:stop") == 0)
	{
		if (!s.features().summon)
			return false;
		s.summonMode = SUMMON_STOP;
		s.summonRemaining = 0;
		return true;
	}

	if (strcmp(cmd, "summon") == 0 || strncmp(cmd, "summon:", 7) == 0)
	{
		if (!s.features().summon)
			return false;
		if (!s.summonInject)
			return false;
		if (!s.hasCtrl)
			return false;

		if (strncmp(cmd, "summon:", 7) == 0)
		{
			const char *dir = cmd + 7;
			if (strcmp(dir, "forward") == 0 || strcmp(dir, "fwd") == 0)
			{
				s.summonDirection = SUMMON_FORWARD;
			}
			else if (strcmp(dir, "reverse") == 0 || strcmp(dir, "rev") == 0)
			{
				s.summonDirection = SUMMON_REVERSE;
			}
			else
			{
				return false;
			}
		}

		s.summonMode = SUMMON_START;
		s.summonRemaining = 30;
		return true;
	}

	return false;
}
