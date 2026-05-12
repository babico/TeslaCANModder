#pragma once

/**
 * @file firmware/lib/vehicle/can/burst.h
 * @brief Non-blocking burst send for one-shot CAN protocol functions
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/types.h"

/**
 * @brief Start a non-blocking burst transmission of a CAN frame.
 *
 * Replaces all blocking delay() loops in one-shot protocol functions.
 * Stores the frame in State and lets burstTick() in the main loop send it.
 * Only one burst is active at a time; a new burst overrides the previous one.
 *
 * @param s Application state holding burst parameters and TX gate flags.
 * @param f CAN frame to transmit repeatedly.
 * @param bus Target CAN bus index (0=Chassis, 1=Vehicle, 2=Body).
 * @param count Number of times to send the frame.
 * @param delayMs Milliseconds to wait between consecutive sends.
 */
inline void startBurst(State &s, const Frame &f, uint8_t bus, uint8_t count, uint8_t delayMs)
{
	if (s.txPaused)
		return;
	if (!s.apGateOpen())
		return;
	s.burstFrame = f;
	s.burstBus = bus;
	s.burstRemaining = count;
	s.burstDelayMs = delayMs;
	s.burstLastMs = 0; // Reset timer so first frame sends immediately
}
