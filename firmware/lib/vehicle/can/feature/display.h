#pragma once
#include <stdlib.h>
#include "vehicle/can/fwd.h"

// ── Display Bit Helpers (0x273 UI_vehicleControl) ────────────────────────────
inline void setDisplayBrightness(Frame &f, uint8_t level)
{
	if (f.dlc < 5)
		return;
	f.data[4] = level; // bits 32-39, factor 0.5
}

// ── Display Control (0x273) ──────────────────────────────────────────────────

static void controlDisplayBrightness(uint8_t level, State &s)
{
	Frame f = makeCtrlFrame(s);
	setDisplayBrightness(f, level);

	startBurst(s, f, BUS_VEHICLE, 20, 20);
}

// ── Display Command Execution ────────────────────────────────────────────────

static bool executeDisplayCmd(const char *cmd, State &s)
{
	if (!s.hasCtrl)
		return false;
	if (strncmp(cmd, "maindisplay:", 12) != 0)
		return false;

	int level = atoi(cmd + 12);
	if (level < 0 || level > 127)
		return false;

	controlDisplayBrightness((uint8_t)level, s);
	return true;
}
