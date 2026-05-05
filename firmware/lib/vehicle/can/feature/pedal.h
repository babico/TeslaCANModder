#pragma once
#include <string.h>
#include "core/forward.h"
#include "vehicle/can/ids.h"
#include "vehicle/can/burst.h"

// ── Pedal Mode Control (0x334 byte 0, bits 5-6) ─────────────────────────────

enum PedalMode
{
	PEDAL_STANDARD = 0,
	PEDAL_CHILL = 1,
	PEDAL_SPORT = 2
};

static void controlPedalMode(PedalMode mode, const uint8_t *lastDrive, State &s)
{
	Frame f;
	f.id = CAN_ID_DRIVE_CONFIG;
	f.dlc = 8;
	memcpy(f.data, lastDrive, 8);

	uint8_t value = 0;
	switch (mode)
	{
	case PEDAL_CHILL:
		value = 0x20;
		break;
	case PEDAL_SPORT:
		value = 0x40;
		break;
	case PEDAL_STANDARD:
		value = 0x00;
		break;
	}
	f.data[0] = (f.data[0] & ~0x60) | value;
	f.data[7] = driveChecksum(f.data, 8);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

// ── Pedal Command Execution ─────────────────────────────────────────────────

static bool executePedalCmd(const char *cmd, State &s)
{
	if (s.variant == LEGACY)
		return false;
	if (!s.hasDrive)
		return false;

	if (strcmp(cmd, "pedal:standard") == 0 || strcmp(cmd, "pedal:std") == 0)
	{
		controlPedalMode(PEDAL_STANDARD, s.lastDrive, s);
		return true;
	}
	if (strcmp(cmd, "pedal:chill") == 0)
	{
		controlPedalMode(PEDAL_CHILL, s.lastDrive, s);
		return true;
	}
	if (strcmp(cmd, "pedal:sport") == 0)
	{
		controlPedalMode(PEDAL_SPORT, s.lastDrive, s);
		return true;
	}
	return false;
}
