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
#include "handler/frame_readers.h"
#include "handler/variant/hw4.h"
#include "handler/variant/hw3.h"
#include "handler/variant/legacy.h"

// ── DAS frame readers ────────────────────────────────────────────────────────
// readDASAutopilotStatus, readDASAutopilotState, isDASAutopilotActive and
// readGtwAutopilotTier moved to handler/frame_readers.h so unit tests can
// pull them in without the full dispatch surface.

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
