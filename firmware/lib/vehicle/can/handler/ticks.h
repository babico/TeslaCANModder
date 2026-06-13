#pragma once

/**
 * @file firmware/lib/vehicle/can/handler/ticks.h
 * @brief Non-blocking periodic tick functions for summon, precondition, burst, and drive mode
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "core/can/bus.h"
#include "core/log/ring.h"
#include "core/driver/esp32/board.h"
#include "vehicle/can/ids.h"
#include "vehicle/can/feature/body/summon.h"
#include "vehicle/can/feature/drive/drive_mode.h"

/**
 * @brief Transmit summon control frames at 20 ms intervals until the burst completes
 *
 * Respects the AP gate and control frame freshness.
 * Clears the remaining count if transmission is paused mid-burst.
 *
 * @param s Reference to the global firmware state
 */
void summonTick(State &s)
{
	if (s.summonRemaining == 0 || !s.hasCtrl || !s.summonInject)
		return;
	if (!s.apGateOpen())
		return;
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

/**
 * @brief Transmit cabin precondition keep-alive frames at 500 ms intervals
 *
 * Sends a fixed precondition request on the vehicle bus while the feature
 * is enabled and the AP gate is open.
 *
 * @param s Reference to the global firmware state
 */
void preconditionTick(State &s)
{
	if (!s.preconditionEnabled)
		return;
	if (!s.apGateOpen())
		return;
	unsigned long now = millis();
	if (now - s.precondLastMs < 500)
		return;
	s.precondLastMs = now;
	Frame f;
	f.id = CAN_ID_PRECONDITION;
	f.dlc = 8;
	memset(f.data, 0, 8);
	f.data[0] = 0x05; // Precondition request command byte
	driverSend(f, BUS_VEHICLE);
}

/**
 * @brief Transmit queued burst frames at the configured delay interval
 *
 * Sends one frame per tick until the burst count reaches zero.
 * Aborts immediately if transmission is paused.
 *
 * @param s Reference to the global firmware state
 */
void burstTick(State &s)
{
	if (s.burstRemaining == 0)
		return;
	if (!s.apGateOpen())
		return;
	unsigned long now = millis();
	if (now - s.burstLastMs < s.burstDelayMs)
		return;
	s.burstLastMs = now;
	driverSend(s.burstFrame, s.burstBus);
	s.burstRemaining--;
}

/**
 * @brief Wrapper that invokes the drive mode tick with the current timestamp
 * @param s Reference to the global firmware state
 */
void driveModeTick_dispatch(State &s)
{
	driveModeTick(s, millis());
}
