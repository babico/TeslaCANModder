#pragma once
#include "core/types.h"
#include "infra/can.h"
#include "infra/burst.h"

// ── Turn Signal Module (3-blink lane change) ────────────────────────────────
// CAN ID 0x3F5 = VCFRONT_vehicleLights
// Byte[0] bits[1:0]: turn signal request
//   0=off, 1=left, 2=right, 3=hazard
// Burst 3 frames at 100ms = one 3-blink cycle

enum TurnSignalRequest
{
	TURN_OFF = 0,
	TURN_LEFT_3 = 1,
	TURN_RIGHT_3 = 2,
	TURN_HAZARD = 3
};

inline void setTurnSignalRequest(Frame &f, TurnSignalRequest req)
{
	f.data[0] = (f.data[0] & 0xFC) | ((uint8_t)req & 0x03);
}

inline void controlTurnSignal(State &s, TurnSignalRequest req)
{
	if (!s.hasCtrl)
		return;
	Frame f;
	f.id = CAN_ID_VCFRONT_LIGHTS;
	f.dlc = 8;
	memset(f.data, 0, 8);
	setTurnSignalRequest(f, req);
	startBurst(s, f, BUS_VEHICLE, 3, 100);
}

inline bool execTurnSignalCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "turn:left3") == 0)
	{
		controlTurnSignal(s, TURN_LEFT_3);
		return true;
	}
	if (strcmp(cmd, "turn:right3") == 0)
	{
		controlTurnSignal(s, TURN_RIGHT_3);
		return true;
	}
	if (strcmp(cmd, "turn:hazard") == 0)
	{
		controlTurnSignal(s, TURN_HAZARD);
		return true;
	}
	if (strcmp(cmd, "turn:off") == 0)
	{
		controlTurnSignal(s, TURN_OFF);
		return true;
	}
	return false;
}

// Runtime decode helpers (D-05)
inline bool decodeTurnSignalLeftActive(const Frame &f)
{
	if (f.dlc < 7)
		return false;
	uint8_t status = (f.data[6] >> 2) & 0x03;
	return status == 1 || status == 2;
}

inline bool decodeTurnSignalRightActive(const Frame &f)
{
	if (f.dlc < 7)
		return false;
	uint8_t status = (f.data[6] >> 4) & 0x03;
	return status == 1 || status == 2;
}

inline uint8_t decodeBlindSpotLeftLevel(const Frame &f)
{
	if (f.dlc < 1)
		return 0;
	uint8_t level = (f.data[0] >> 4) & 0x03;
	return level <= 2 ? level : 0;
}

inline uint8_t decodeBlindSpotRightLevel(const Frame &f)
{
	if (f.dlc < 1)
		return 0;
	uint8_t level = (f.data[0] >> 6) & 0x03;
	return level <= 2 ? level : 0;
}
