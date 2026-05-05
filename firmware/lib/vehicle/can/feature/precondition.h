#pragma once
#include "vehicle/can/fwd.h"
#include "core/util/parse.h"

// ── Battery Preconditioning Trigger ──────────────────────────────────────────
// Injects CAN 0x082 (UI_tripPlanning) to trigger battery heating for charging.
// Needs periodic injection at 500ms intervals while active.
// Sources: tuncasoftbildik, hypery11-flipper.

static void controlPrecondition(bool enable, State &s)
{
	Frame f;
	f.id = CAN_ID_PRECONDITION;
	f.dlc = 8;
	memset(f.data, 0, 8);
	f.data[0] = enable ? 0x05 : 0x00;
	startBurst(s, f, BUS_VEHICLE, 1, 0);
}

// ── Preconditioning Command ──────────────────────────────────────────────────
// Enables/disables battery preconditioning for supercharging.

static bool executePreconditionCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "precondition:", 13) == 0)
	{
		if (s.variant == LEGACY)
			return false;
		if (!parseBoolCmd(cmd + 13, s.preconditionEnabled, s.preconditionEnabled))
			return false;
		if (s.preconditionEnabled)
		{
			controlPrecondition(true, s);
			s.precondLastMs = millis();
		}
		else
		{
			controlPrecondition(false, s);
		}
		saveSettings(s);
		return true;
	}
	return false;
}
