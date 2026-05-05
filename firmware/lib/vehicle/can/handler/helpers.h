#pragma once
// ── Dispatch Helpers ─────────────────────────────────────────────────────────
// Small read/utility inlines and shared state used by both the bus-specific
// handlers (bus_chassis.h / bus_vehicle.h) and dispatch.h.
//
// Kept header-only because firmware/src/esp32/main.cpp is the single
// translation unit; static globals here are safe.

#include "core/forward.h"
#include "core/can/bus.h"
#include "core/platform.h"
#include "vehicle/can/ids.h"
#include "vehicle/can/feature/das_drive.h"
#include "handler/variant/hw4.h"
#include "handler/variant/hw3.h"
#include "handler/variant/legacy.h"

// ── DAS frame readers ────────────────────────────────────────────────────────
inline uint8_t readDASAutopilotStatus(const Frame &f)
{
	return f.dlc >= 1 ? (f.data[0] & 0x0F) : 0;
}

// DAS_autopilotState from 0x39B byte1 bits[7:4].
// 0=UNAVAIL 1=AVAIL 2=ACTIVE_NOMINAL 3=ACTIVE_MIN_DRIVER ...
// Used by AP-First mode to delay 0x3FD injection until AP is running.
inline uint8_t readDASAutopilotState(const Frame &f)
{
	return f.dlc >= 2 ? ((f.data[1] >> 4) & 0x0F) : 0;
}

inline bool isDASAutopilotActive(uint8_t status)
{
	return status >= 3 && status <= 5;
}

inline int8_t readGtwAutopilotTier(const Frame &f)
{
	if (f.dlc < 6)
		return -1;
	if (readMuxID(f) != 2)
		return -1;
	return (int8_t)((f.data[5] >> 2) & 0x07);
}

// Module-level platform instance for re-resolution on CAN updates.
// Shared by bus_vehicle.h (firmware version + carConfig decoders).
static VehiclePlatform dispatchPlatform;

inline void resetHandlerLogFlags()
{
	resetHW4LogFlags();
	resetHW3LogFlags();
	resetLegacyLogFlags();
}

// ── CAN frame-rate accounting (per bus, rolling 1-second window) ─────────────
static inline void _updateCanFrameRate(State &s, uint8_t bus, uint32_t now)
{
	if (bus >= 3)
		return;
	CanBusStat &b = s.canDiag.bus[bus];
	b.frames++;
	b.windowCount++;
	uint32_t elapsed = now - b.windowStartMs;
	if (elapsed >= 1000UL)
	{
		// Compute Hz × 10 as integer; clamp to uint16_t max
		uint32_t hz10 = (b.windowCount * 10000UL) / elapsed;
		b.hz = (hz10 > 0xFFFFu) ? 0xFFFFu : (uint16_t)hz10;
		if (b.hz < b.hzMin)
			b.hzMin = b.hz;
		if (b.hz > b.hzMax)
			b.hzMax = b.hz;
		b.windowCount = 0;
		b.windowStartMs = now;
	}
}
