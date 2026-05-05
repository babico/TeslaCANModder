#pragma once
#include "core/types.h"
#include "vehicle/can/ids.h"

// ── Rear Seatbelt Buckle Emulation ──────────────────────────────────────────
// Suppress rear seatbelt warnings by injecting a CAN frame that signals
// all rear seatbelts are buckled.
// CAN ID 0x3F3 = VCRIGHT_seatbeltStatus
// Byte[0] bits[2:0] = rear-left buckled | rear-center buckled | rear-right buckled

inline void controlSeatbeltEmulation(State &s, bool enable)
{
	s.seatbeltEmulation = enable;
}

inline void seatbeltEmulationTick(State &s)
{
	if (!s.seatbeltEmulation)
		return;
	if (!s.apGateOpen())
		return;
	if (s.txPaused)
		return;
	unsigned long now = millis();
	if (now - s.seatbeltLastMs < 500)
		return;
	s.seatbeltLastMs = now;
	Frame f;
	f.id = CAN_ID_SEATBELT_STATUS;
	f.dlc = 8;
	memset(f.data, 0, 8);
	f.data[0] = 0x07; // all 3 rear seatbelts "buckled"
	driverSend(f, BUS_VEHICLE);
}

static bool executeSeatbeltCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "seatbelt:on") == 0)
	{
		controlSeatbeltEmulation(s, true);
		return true;
	}
	if (strcmp(cmd, "seatbelt:off") == 0)
	{
		controlSeatbeltEmulation(s, false);
		return true;
	}
	return false;
}
