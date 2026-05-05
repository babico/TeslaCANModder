#pragma once
#include <stdlib.h>
#include "vehicle/can/fwd.h"

// ── Window Vent Control (0x119) ──────────────────────────────────────────────
// Controls window vent position via CAN ID 0x119.
// Byte 0 = 0x1F (window mask — all four windows), byte 1 = position 0-100.
// Position 0 = fully closed, 100 = fully open (vent).

enum WindowVentPosition
{
	WINDOW_VENT_CLOSE = 0,
	WINDOW_VENT_OPEN = 100
};

// Send a vent command at the given position (0-100), clamped to valid range.
// Burst-sends 30 frames at 20 ms intervals on BUS_BODY to ensure ECU accepts.
static void controlWindowVent(uint8_t pos, State &s)
{
	if (pos > 100)
		pos = 100;
	Frame f;
	f.id = CAN_ID_WINDOW_VENT;
	f.dlc = 2;
	f.data[0] = 0x1F; // all-window mask
	f.data[1] = pos;

	startBurst(s, f, BUS_BODY, 30, 20);
}

// ── Window Command Execution ────────────────────────────────────────────────
// Supports:
//   window:vent:open   — fully open vent (position 100)
//   window:vent:close  — fully close vent (position 0)
//   window:vent:N      — set vent to arbitrary position 0-100
//   vent:open / vent:close — shorthand aliases

static bool executeWindowCmd(const char *cmd, State &s)
{
	if (s.variant == LEGACY)
		return false;

	if (strcmp(cmd, "window:vent:open") == 0 || strcmp(cmd, "vent:open") == 0)
	{
		controlWindowVent(WINDOW_VENT_OPEN, s);
		return true;
	}
	if (strcmp(cmd, "window:vent:close") == 0 || strcmp(cmd, "vent:close") == 0)
	{
		controlWindowVent(WINDOW_VENT_CLOSE, s);
		return true;
	}
	// window:vent:N — arbitrary position 0-100
	if (strncmp(cmd, "window:vent:", 12) == 0)
	{
		const char *valStr = cmd + 12;
		if (*valStr == '\0')
			return false;
		char *end;
		long val = strtol(valStr, &end, 10);
		if (*end != '\0')
			return false; // trailing non-digit chars
		if (val < 0 || val > 100)
			return false;
		controlWindowVent((uint8_t)val, s);
		return true;
	}
	return false;
}
