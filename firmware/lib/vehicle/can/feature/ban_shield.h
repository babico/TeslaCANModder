#pragma once
#include "core/forward.h"
#include "core/util/parse.h"

// ── Ban Shield Command ────────────────────────────────────────────────────────
// banshield:on|off
// Enables/disables experimental telemetry monitoring for ban threat detection
bool executeBanShieldCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "banshield:", 10) == 0)
	{
		if (!parseBoolCmd(cmd + 10, s.banShieldEnabled, s.banShieldEnabled))
			return false;
		// Reset threat level when toggling on
		if (s.banShieldEnabled)
		{
			s.banThreatLevel = 0;
			s.banDetectionCount = 0;
		}
		saveSettings(s);
		return true;
	}
	return false;
}

// ── GTW Shield Commands ───────────────────────────────────────────────────────
// gtwshield:arm      — activate snapshot-based frame defense on 0x7FF
// gtwshield:disarm   — deactivate (keep snapshot)
// gtwshield:reset    — disarm + clear snapshot (re-learning phase)
//
// Source pattern: hypery11/flipper-tesla-fsd fsd_handle_gtw_shield()
bool executeGtwShieldCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "gtwshield:", 10) != 0)
		return false;
	const char *arg = cmd + 10;
	if (strcmp(arg, "arm") == 0)
	{
		s.gtwShieldArmed = true;
		return true;
	}
	if (strcmp(arg, "disarm") == 0)
	{
		s.gtwShieldArmed = false;
		return true;
	}
	if (strcmp(arg, "reset") == 0)
	{
		s.gtwShieldArmed = false;
		s.gtwShieldBlocks = 0;
		for (uint8_t i = 0; i < 8; i++)
		{
			s.gtwSnapshotValid[i] = false;
			for (uint8_t j = 0; j < 8; j++)
				s.gtwSnapshot[i][j] = 0;
		}
		return true;
	}
	return false;
}

// ── GTW Shield Frame Handler ──────────────────────────────────────────────────
// Called for every 0x7FF frame from the vehicle bus.
// Phase 1 (not armed): learn snapshot for each mux variant.
// Phase 2 (armed): compare incoming frame against snapshot;
//   if any byte differs, overwrite frame data with snapshot and return true
//   so the caller can retransmit the healthy version immediately.
//
// Source: hypery11/flipper-tesla-fsd fsd_logic/fsd_handler.c
bool handleGtwShield(Frame &f, State &s)
{
	if (f.dlc < 8)
		return false;
	// Suspend shield during OTA to avoid blocking firmware update frames
	if (s.otaInProgress)
		return false;
	uint8_t mux = f.data[0] & 0x07;

	if (!s.gtwShieldArmed)
	{
		// Learning phase: capture first-seen frame per mux
		if (!s.gtwSnapshotValid[mux])
		{
			for (uint8_t i = 0; i < 8; i++)
				s.gtwSnapshot[mux][i] = f.data[i];
			s.gtwSnapshotValid[mux] = true;
		}
		return false;
	}

	// Armed: compare against snapshot
	if (!s.gtwSnapshotValid[mux])
		return false;

	bool changed = false;
	for (uint8_t i = 0; i < 8; i++)
	{
		if (f.data[i] != s.gtwSnapshot[mux][i])
		{
			changed = true;
			break;
		}
	}

	if (changed)
	{
		// Overwrite with healthy snapshot and signal retransmit
		for (uint8_t i = 0; i < 8; i++)
			f.data[i] = s.gtwSnapshot[mux][i];
		s.gtwShieldBlocks++;
		return true;
	}
	return false;
}
