#pragma once
// ── Periodic Tick Functions ──────────────────────────────────────────────────
// Non-blocking timed senders driven from the main loop. Each respects the
// AP gate, OTA tx-pause, and 0x39B/0x118 freshness rules via apGateOpen().

#include "core/forward.h"
#include "core/can/bus.h"
#include "core/log/ring.h"
#include "core/driver/esp32/board.h"
#include "vehicle/can/ids.h"
#include "vehicle/can/feature/summon.h"
#include "vehicle/can/feature/drive_mode.h"

void summonTick(State &s)
{
	if (s.summonRemaining == 0 || !s.hasCtrl || !s.summonInject)
		return;
	if (!s.apGateOpen())
		return;
	if (s.txPaused)
	{
		s.summonRemaining = 0;
		return;
	}
	unsigned long now = millis();
	if (now - s.summonLastMs < 20)
		return;
	s.summonLastMs = now;

	Frame f;
	f.id = CAN_ID_UI_VEHICLE_CTRL;
	f.dlc = 8;
	memcpy(f.data, s.lastCtrl, 8);
	setSummonActive(f, true);
	setSummonDirection(f, s.summonDirection);
	setSummonMode(f, s.summonMode);
	driverSend(f, BUS_VEHICLE);
	s.summonRemaining--;
	if (s.summonRemaining == 0)
		sendLog(F("Summon burst complete"));
}

void preconditionTick(State &s)
{
	if (!s.preconditionEnabled)
		return;
	if (!s.apGateOpen())
		return;
	if (s.txPaused)
		return;
	unsigned long now = millis();
	if (now - s.precondLastMs < 500)
		return;
	s.precondLastMs = now;
	Frame f;
	f.id = CAN_ID_PRECONDITION;
	f.dlc = 8;
	memset(f.data, 0, 8);
	f.data[0] = 0x05;
	driverSend(f, BUS_VEHICLE);
}

// Burst Tick (non-blocking one-shot sends)
void burstTick(State &s)
{
	if (s.burstRemaining == 0)
		return;
	if (!s.apGateOpen())
		return;
	if (s.txPaused)
	{
		s.burstRemaining = 0;
		return;
	}
	unsigned long now = millis();
	if (now - s.burstLastMs < s.burstDelayMs)
		return;
	s.burstLastMs = now;
	driverSend(s.burstFrame, s.burstBus);
	s.burstRemaining--;
}

void driveModeTick_dispatch(State &s)
{
	driveModeTick(s, millis());
}
