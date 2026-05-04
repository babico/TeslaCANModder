#pragma once
#include <string.h>
#include "core/forward.h"
#include "vehicle/can/ids.h"
#include "vehicle/can/burst.h"

// ── Seat Heating Bit Helpers (0x273 UI_vehicleControl) ───────────────────────
enum SeatHeatLevel
{
	SEAT_OFF = 0,
	SEAT_LOW = 1,
	SEAT_MED = 2,
	SEAT_HIGH = 3
};

inline void setSeatHeatFL(Frame &f, SeatHeatLevel level)
{
	if (f.dlc < 6)
		return;
	f.data[5] = (f.data[5] & ~0x0C) | ((level & 0x03) << 2); // bits 42-43
}

inline void setSeatHeatFR(Frame &f, SeatHeatLevel level)
{
	if (f.dlc < 6)
		return;
	f.data[5] = (f.data[5] & ~0x30) | ((level & 0x03) << 4); // bits 44-45
}

inline void setSeatHeatRL(Frame &f, SeatHeatLevel level)
{
	if (f.dlc < 6)
		return;
	f.data[5] = (f.data[5] & ~0xC0) | ((level & 0x03) << 6); // bits 46-47
}

inline void setSeatHeatRR(Frame &f, SeatHeatLevel level)
{
	if (f.dlc < 7)
		return;
	f.data[6] = (f.data[6] & ~0x0C) | ((level & 0x03) << 2); // bits 50-51
}

inline void setSeatHeatRC(Frame &f, SeatHeatLevel level)
{
	if (f.dlc < 7)
		return;
	f.data[6] = (f.data[6] & ~0x03) | (level & 0x03); // bits 48-49
}

// ── Seat Heating Control (0x273) ─────────────────────────────────────────────

static void controlSeatHeat(uint8_t seat, SeatHeatLevel level, State &s)
{
	Frame f = {CAN_ID_UI_VEHICLE_CTRL, 8};
	memcpy(f.data, s.lastCtrl, 8);

	if (seat == 0)
		setSeatHeatFL(f, level);
	else if (seat == 1)
		setSeatHeatFR(f, level);
	else if (seat == 2)
		setSeatHeatRL(f, level);
	else if (seat == 3)
		setSeatHeatRR(f, level);
	else if (seat == 4)
		setSeatHeatRC(f, level);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

// ── Seat Command Execution ───────────────────────────────────────────────────

static bool execSeatCmd(const char *cmd, State &s)
{
	if (!s.hasCtrl)
		return false;
	if (strncmp(cmd, "seat:", 5) != 0)
		return false;

	const char *pos = cmd + 5;
	char lastChar = cmd[strlen(cmd) - 1];
	if (lastChar < '0' || lastChar > '3')
		return false;

	SeatHeatLevel level = (SeatHeatLevel)(lastChar - '0');
	uint8_t seat = 255;

	if (strncmp(pos, "fl:", 3) == 0)
		seat = 0;
	else if (strncmp(pos, "fr:", 3) == 0)
		seat = 1;
	else if (strncmp(pos, "rl:", 3) == 0)
		seat = 2;
	else if (strncmp(pos, "rr:", 3) == 0)
		seat = 3;
	else if (strncmp(pos, "rc:", 3) == 0)
		seat = 4;
	else
		return false;

	controlSeatHeat(seat, level, s);
	return true;
}
