#pragma once
#include "vehicle/can/fwd.h"

// ── Power Bit Helpers (0x273 UI_vehicleControl) ─────────────────────────────
inline void setAccessoryPower(Frame &f, bool enable)
{
	if (f.dlc < 1)
		return;
	if (enable)
		f.data[0] |= 0x01; // bit 0
	else
		f.data[0] &= ~0x01;
}

inline void setPowerOff(Frame &f, bool off)
{
	if (f.dlc < 4)
		return;
	if (off)
		f.data[3] |= 0x80; // bit 31
	else
		f.data[3] &= ~0x80;
}

inline void setDriveStateRequest(Frame &f, bool enable)
{
	if (f.dlc < 8)
		return;
	if (enable)
		f.data[7] |= 0x40; // bit 62
	else
		f.data[7] &= ~0x40;
}

// ── Power Control (0x273) ────────────────────────────────────────────────────

static void controlPowerOff(State &s)
{
	Frame f = makeCtrlFrame(s);
	setPowerOff(f, true);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

static void controlAccessoryPower(bool enable, State &s)
{
	Frame f = makeCtrlFrame(s);
	setAccessoryPower(f, enable);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

static void controlDriveState(State &s)
{
	Frame f = makeCtrlFrame(s);
	setDriveStateRequest(f, true);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

// ── Power Command Execution ──────────────────────────────────────────────────

static bool executePowerCmd(const char *cmd, State &s)
{
	if (!s.hasCtrl)
		return false;

	if (strcmp(cmd, "power:acc:on") == 0)
	{
		controlAccessoryPower(true, s);
		return true;
	}
	if (strcmp(cmd, "power:acc:off") == 0)
	{
		controlAccessoryPower(false, s);
		return true;
	}
	if (strcmp(cmd, "power:ready") == 0)
	{
		controlDriveState(s);
		return true;
	}
	if (strcmp(cmd, "power:off") == 0)
	{
		controlPowerOff(s);
		return true;
	}
	return false;
}
