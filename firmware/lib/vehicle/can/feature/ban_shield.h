#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/ban_shield.h
 * @brief Ban shield and GTW snapshot-based frame defense commands
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "core/util/parse.h"

/**
 * @brief Execute the ban shield toggle command ("banshield:on" or "banshield:off").
 *
 * Enables or disables experimental telemetry monitoring for ban threat detection.
 * Resets threat counters when enabling.
 *
 * @param cmd Command string to parse.
 * @param s Device state to update.
 * @return True if the command was recognized and executed.
 */
static bool executeBanShieldCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "banshield:", 10) == 0)
	{
		if (!parseBoolCmd(cmd + 10, s.banShieldEnabled, s.banShieldEnabled))
			return false;
		// Reset threat level when toggling on to start fresh
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

/**
 * @brief Execute GTW shield commands ("gtwshield:arm|disarm|reset").
 *
 * Controls snapshot-based frame defense on 0x7FF:
 *   - arm: activate defense (compare incoming frames against snapshot)
 *   - disarm: deactivate defense (keep learned snapshot)
 *   - reset: disarm and clear snapshot (re-enter learning phase)
 *
 * @param cmd Command string to parse.
 * @param s Device state to update.
 * @return True if the command was recognized and executed.
 */
static bool executeGtwShieldCmd(const char *cmd, State &s)
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

/**
 * @brief Handle an incoming 0x7FF GTW frame with snapshot-based defense.
 *
 * Operates in two phases:
 *   - Learning (not armed): captures the first-seen frame per mux variant as the baseline.
 *   - Armed: compares incoming frames against the snapshot; if any byte differs,
 *     overwrites frame data with the healthy snapshot for retransmission.
 *
 * Suspended during OTA to avoid blocking firmware update frames.
 *
 * @param f CAN frame to inspect and potentially overwrite.
 * @param s Device state with shield configuration and snapshot storage.
 * @return True if the frame was overwritten (caller should retransmit).
 */
static bool handleGtwShield(Frame &f, State &s)
{
	if (f.dlc < 8)
		return false;
	// Suspend shield during OTA to avoid blocking firmware update frames
	if (s.otaInProgress)
		return false;
	uint8_t mux = f.data[0] & 0x07; // Mux variant from bits[2:0] of byte[0]

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

	// Armed phase: compare against learned snapshot
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
		// Overwrite with healthy snapshot and signal retransmit needed
		for (uint8_t i = 0; i < 8; i++)
			f.data[i] = s.gtwSnapshot[mux][i];
		s.gtwShieldBlocks++;
		return true;
	}
	return false;
}
