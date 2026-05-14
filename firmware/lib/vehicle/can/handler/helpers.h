#pragma once

/**
 * @file firmware/lib/vehicle/can/handler/helpers.h
 * @brief Shared dispatch utilities including platform state, log resets, and frame-rate metering
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "core/can/bus.h"
#include "core/platform.h"
#include "vehicle/can/ids.h"
#include "vehicle/can/feature/das/das_drive.h"
#include "handler/frame_readers.h"
#include "handler/variant/hw4.h"
#include "handler/variant/hw3.h"
#include "handler/variant/legacy.h"

/**
 * @brief Module-level platform instance shared by bus_vehicle.h for firmware version
 *        and carConfig decoders
 */
static VehiclePlatform dispatchPlatform;

/**
 * @brief Reset one-shot log flags for all variant handlers
 */
inline void resetHandlerLogFlags()
{
	resetHW4LogFlags();
	resetHW3LogFlags();
	resetLegacyLogFlags();
}

/**
 * @brief Update rolling 1-second frame-rate statistics for a given bus
 * @param s Reference to the global firmware state containing diagnostic counters
 * @param bus Index of the bus to update (0-2)
 * @param now Current timestamp in milliseconds
 */
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
		// Compute Hz × 10 as fixed-point integer; clamp to uint16_t max
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
