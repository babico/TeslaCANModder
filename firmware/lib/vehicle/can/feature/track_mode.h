#pragma once
#include <string.h>
#include "core/forward.h"
#include "vehicle/can/ids.h"
#include "vehicle/can/burst.h"
#include "core/util/parse.h"

// ── Track Mode Inject ────────────────────────────────────────────────────────
// Modifies CAN 0x313 (UI_trackModeSettings) byte[0] bits 1:0 = 0x01 to enable.
// Sources: ev-open-can-tools, hypery11-flipper.

static void controlTrackMode(bool enable, State &s)
{
	Frame f;
	f.id = CAN_ID_TRACK_MODE;
	f.dlc = 8;
	memset(f.data, 0, 8);
	if (enable)
	{
		f.data[0] = (f.data[0] & 0xFC) | 0x01; // bits 1:0 = 01
	}
	startBurst(s, f, BUS_VEHICLE, 20, 20);
}

// ── Track Mode Command ───────────────────────────────────────────────────────
// Enables/disables Track Mode via CAN 0x313 injection.

bool execTrackModeCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "trackmode:", 10) == 0)
	{
		if (s.variant == LEGACY)
			return false;
		if (!parseBoolCmd(cmd + 10, s.trackModeEnabled, s.trackModeEnabled))
			return false;
		controlTrackMode(s.trackModeEnabled, s);
		saveSettings(s);
		return true;
	}
	return false;
}
