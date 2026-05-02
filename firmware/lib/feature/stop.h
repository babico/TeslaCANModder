#pragma once
#include <string.h>
#include "core/forward.h"
#include "infra/can.h"
#include "infra/burst.h"

// ── Stop Mode Control (0x334 byte 5, bits 0-1) ──────────────────────────────

enum StopMode
{
	STOP_CREEP = 0,
	STOP_ROLL = 1,
	STOP_HOLD = 2
};

static void controlStopMode(StopMode mode, const uint8_t *lastDrive, State &s)
{
	Frame f;
	f.id = CAN_ID_DRIVE_CONFIG;
	f.dlc = 8;
	memcpy(f.data, lastDrive, 8);

	f.data[5] = (f.data[5] & ~0x03) | (mode & 0x03);
	f.data[7] = driveChecksum(f.data, 8);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

// ── Stop Mode Command Execution ─────────────────────────────────────────────

static bool execStopCmd(const char *cmd, State &s)
{
	if (s.variant == LEGACY)
		return false;
	if (!s.hasDrive)
		return false;

	if (strcmp(cmd, "stop:creep") == 0)
	{
		controlStopMode(STOP_CREEP, s.lastDrive, s);
		return true;
	}
	if (strcmp(cmd, "stop:roll") == 0)
	{
		controlStopMode(STOP_ROLL, s.lastDrive, s);
		return true;
	}
	if (strcmp(cmd, "stop:hold") == 0)
	{
		controlStopMode(STOP_HOLD, s.lastDrive, s);
		return true;
	}
	return false;
}
