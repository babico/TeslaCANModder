#pragma once
#include "core/types.h"

// ── Non-Blocking Burst Send ──────────────────────────────────────────────────
// Replaces all blocking delay() loops in one-shot protocol functions.
// Stores the frame in State and lets burstTick() in the main loop send it.
// Only one burst active at a time (new burst overrides previous).

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
	s.burstLastMs = 0;
}
