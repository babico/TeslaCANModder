#pragma once
#include "vehicle/can/fwd.h"

// ── Regen Level Control (0x334 byte 2, 0-200) ───────────────────────────────

static void controlRegenLevel(uint8_t level, const uint8_t *lastDrive, State &s)
{
	Frame f;
	f.id = CAN_ID_DRIVE_CONFIG;
	f.dlc = 8;
	memcpy(f.data, lastDrive, 8);

	if (level > 200)
		level = 200;
	f.data[2] = level;
	f.data[7] = driveChecksum(f.data, 8);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

// ── Regen Command Execution ─────────────────────────────────────────────────

static bool executeRegenCmd(const char *cmd, State &s)
{
	if (s.variant == LEGACY)
		return false;
	if (!s.hasDrive)
		return false;

	if (strcmp(cmd, "regen:off") == 0)
	{
		controlRegenLevel(0, s.lastDrive, s);
		return true;
	}
	if (strcmp(cmd, "regen:low") == 0)
	{
		controlRegenLevel(50, s.lastDrive, s);
		return true;
	}
	if (strcmp(cmd, "regen:standard") == 0 || strcmp(cmd, "regen:std") == 0)
	{
		controlRegenLevel(100, s.lastDrive, s);
		return true;
	}
	if (strcmp(cmd, "regen:max") == 0)
	{
		controlRegenLevel(200, s.lastDrive, s);
		return true;
	}
	return false;
}
