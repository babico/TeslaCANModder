#pragma once
#include "core/types.h"
#include "vehicle/can/ids.h"

// ── Automatic Air Recirculation ─────────────────────────────────────────────
// Toggle cabin air recirculation via CAN ID 0x2AA
// Byte[0] bit 0 = recirculation request (0=fresh, 1=recirc)
// Burst 20 frames at 20ms to ensure ECU accepts

inline void setAirRecircRequest(Frame &f, bool recirc)
{
	if (recirc)
		f.data[0] |= 0x01;
	else
		f.data[0] &= ~0x01;
}

inline void controlAirRecirc(State &s, bool enable)
{
	if (!s.hasClimate)
		return;
	Frame f;
	f.id = CAN_ID_AIR_RECIRC;
	f.dlc = 8;
	memcpy(f.data, s.lastClimate, 5);
	memset(f.data + 5, 0, 3);
	setAirRecircRequest(f, enable);
	startBurst(s, f, BUS_VEHICLE, 20, 20);
}

static bool executeAirRecircCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "airecirc:on") == 0)
	{
		controlAirRecirc(s, true);
		return true;
	}
	if (strcmp(cmd, "airecirc:off") == 0)
	{
		controlAirRecirc(s, false);
		return true;
	}
	return false;
}
