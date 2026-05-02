#pragma once
#include <string.h>
#include "core/forward.h"
#include "infra/can.h"
#include "infra/burst.h"

// ── Lighting Bit Helpers (0x273 UI_vehicleControl) ───────────────────────────
inline void setFrontFogSwitch(Frame &f, bool enable)
{
	if (f.dlc < 1)
		return;
	if (enable)
		f.data[0] |= 0x08; // bit 3
	else
		f.data[0] &= ~0x08;
}

inline void setRearFogSwitch(Frame &f, bool enable)
{
	if (f.dlc < 3)
		return;
	if (enable)
		f.data[2] |= 0x80; // bit 23
	else
		f.data[2] &= ~0x80;
}

inline void setAutoHighBeam(Frame &f, bool enable)
{
	if (f.dlc < 6)
		return;
	if (enable)
		f.data[5] |= 0x02; // bit 41
	else
		f.data[5] &= ~0x02;
}

inline void setAmbientLighting(Frame &f, bool enable)
{
	if (f.dlc < 6)
		return;
	if (enable)
		f.data[5] |= 0x01; // bit 40
	else
		f.data[5] &= ~0x01;
}

inline void setSeeYouHomeLighting(Frame &f, bool enable)
{
	if (f.dlc < 4)
		return;
	if (enable)
		f.data[3] |= 0x40; // bit 30
	else
		f.data[3] &= ~0x40;
}

enum DomeLightSwitch
{
	DOME_OFF = 0,
	DOME_ON = 1,
	DOME_AUTO = 2
};

inline void setDomeLightSwitch(Frame &f, DomeLightSwitch mode)
{
	if (f.dlc < 8)
		return;
	f.data[7] = (f.data[7] & ~0x18) | ((mode & 0x03) << 3); // bits 59-60
}

// ── Light Control (0x273) ────────────────────────────────────────────────────

static void controlFrontFog(State &s)
{
	Frame f = {CAN_ID_UI_VEHICLE_CTRL, 8};
	memcpy(f.data, s.lastCtrl, 8);
	setFrontFogSwitch(f, true);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

static void controlRearFog(State &s)
{
	Frame f = {CAN_ID_UI_VEHICLE_CTRL, 8};
	memcpy(f.data, s.lastCtrl, 8);
	setRearFogSwitch(f, true);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

static void controlAutoHighBeam(State &s)
{
	Frame f = {CAN_ID_UI_VEHICLE_CTRL, 8};
	memcpy(f.data, s.lastCtrl, 8);
	setAutoHighBeam(f, true);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

static void controlAmbientLight(State &s)
{
	Frame f = {CAN_ID_UI_VEHICLE_CTRL, 8};
	memcpy(f.data, s.lastCtrl, 8);
	setAmbientLighting(f, true);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

static void controlHomeLight(State &s)
{
	Frame f = {CAN_ID_UI_VEHICLE_CTRL, 8};
	memcpy(f.data, s.lastCtrl, 8);
	setSeeYouHomeLighting(f, true);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

static void controlDomeLight(DomeLightSwitch mode, State &s)
{
	Frame f = {CAN_ID_UI_VEHICLE_CTRL, 8};
	memcpy(f.data, s.lastCtrl, 8);
	setDomeLightSwitch(f, mode);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

// ── Light Command Execution ──────────────────────────────────────────────────

static bool execLightCmd(const char *cmd, State &s)
{
	if (!s.hasCtrl)
		return false;

	if (strcmp(cmd, "light:fog:front") == 0)
	{
		controlFrontFog(s);
		return true;
	}
	if (strcmp(cmd, "light:fog:rear") == 0)
	{
		controlRearFog(s);
		return true;
	}
	if (strcmp(cmd, "light:highbeam:auto") == 0)
	{
		controlAutoHighBeam(s);
		return true;
	}
	if (strcmp(cmd, "light:ambient") == 0)
	{
		controlAmbientLight(s);
		return true;
	}
	if (strcmp(cmd, "light:home") == 0)
	{
		controlHomeLight(s);
		return true;
	}
	if (strcmp(cmd, "light:dome:off") == 0)
	{
		controlDomeLight(DOME_OFF, s);
		return true;
	}
	if (strcmp(cmd, "light:dome:on") == 0)
	{
		controlDomeLight(DOME_ON, s);
		return true;
	}
	if (strcmp(cmd, "light:dome:auto") == 0)
	{
		controlDomeLight(DOME_AUTO, s);
		return true;
	}
	return false;
}
