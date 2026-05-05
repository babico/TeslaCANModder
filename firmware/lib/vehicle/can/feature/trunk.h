#pragma once
#include <string.h>
#include "core/forward.h"
#include "vehicle/can/ids.h"
#include "vehicle/can/burst.h"

// ── Trunk/Frunk Control ──────────────────────────────────────────────────────
// Unified control for all trunk/frunk operations

enum TrunkTarget
{
	TRUNK_FRUNK = 0,   // Front trunk (uses 0x273)
	TRUNK_REAR = 1,	   // Rear trunk (uses 0x3B3)
	TRUNK_GLOVEBOX = 2 // Glovebox (uses 0x3B3)
};

enum TrunkAction
{
	TRUNK_OPEN = 0,
	TRUNK_CLOSE = 1
};

// Send frunk open command (0x273 bit 5)
static void controlFrunk(const uint8_t *lastCtrl, bool open, State &s)
{
	Frame f;
	f.id = CAN_ID_UI_VEHICLE_CTRL;
	f.dlc = 8;
	memcpy(f.data, lastCtrl, 8);

	if (open)
	{
		f.data[0] |= 0x20; // Set bit 5
	}
	else
	{
		f.data[0] &= ~0x20; // Clear bit 5
	}

	startBurst(s, f, BUS_VEHICLE, 20, 20);
}

// Send trunk open/close command (0x3B3)
static void controlTrunk(bool open, State &s)
{
	Frame f;
	f.id = CAN_ID_TRUNK_CTRL;
	f.dlc = 4;
	f.data[0] = open ? 0x02 : 0x03; // 0x02 = open, 0x03 = close
	f.data[1] = 0x00;
	f.data[2] = 0x00;
	f.data[3] = 0x00;

	startBurst(s, f, BUS_BODY, 20, 100);
}

// Send glovebox open command (0x3B3 - glovebox doesn't have close)
static void controlGlovebox(State &s)
{
	Frame f;
	f.id = CAN_ID_TRUNK_CTRL;
	f.dlc = 4;
	f.data[0] = 0x01; // Glovebox command (open only)
	f.data[1] = 0x00;
	f.data[2] = 0x00;
	f.data[3] = 0x00;

	startBurst(s, f, BUS_BODY, 20, 100);
}

// Unified trunk control interface
static bool executeTrunkControl(TrunkTarget target, TrunkAction action, State &s)
{
	switch (target)
	{
	case TRUNK_FRUNK:
		if (!s.hasCtrl)
			return false; // Need 0x273 frame cached
		controlFrunk(s.lastCtrl, action == TRUNK_OPEN, s);
		return true;

	case TRUNK_REAR:
		controlTrunk(action == TRUNK_OPEN, s);
		return true;

	case TRUNK_GLOVEBOX:
		if (action == TRUNK_CLOSE)
			return false; // Glovebox can't close via CAN
		controlGlovebox(s);
		return true;

	default:
		return false;
	}
}

// ── Trunk Command Execution ──────────────────────────────────────────────────

static bool executeTrunkCmd(const char *cmd, State &s)
{
	if (s.variant == LEGACY)
		return false;
	if (strcmp(cmd, "frunk:open") == 0 || strcmp(cmd, "frunk") == 0)
	{
		return executeTrunkControl(TRUNK_FRUNK, TRUNK_OPEN, s);
	}
	if (strcmp(cmd, "frunk:close") == 0)
	{
		return executeTrunkControl(TRUNK_FRUNK, TRUNK_CLOSE, s);
	}
	if (strcmp(cmd, "trunk:open") == 0 || strcmp(cmd, "trunk") == 0)
	{
		return executeTrunkControl(TRUNK_REAR, TRUNK_OPEN, s);
	}
	if (strcmp(cmd, "trunk:close") == 0)
	{
		return executeTrunkControl(TRUNK_REAR, TRUNK_CLOSE, s);
	}
	if (strcmp(cmd, "glovebox") == 0)
	{
		return executeTrunkControl(TRUNK_GLOVEBOX, TRUNK_OPEN, s);
	}
	return false;
}
