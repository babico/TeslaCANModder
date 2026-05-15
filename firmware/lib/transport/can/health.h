#pragma once

/**
 * @file firmware/lib/transport/can/health.h
 * @brief MCP2515 CAN bus health checking and diagnostic reporting
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <stdint.h>
#include "transport/can/bus.h"

/**
 * @brief Health status for a single CAN bus slot
 */
struct BusHealth
{
	bool configured;   // Bus is enabled in build config
	bool detected;	   // MCP2515 physically detected on SPI
	bool receiving;	   // At least one frame received since boot
	uint32_t lastRxMs; // millis() timestamp of last received frame
};

/**
 * @brief Aggregated health report across all CAN bus slots
 */
struct CanHealthReport
{
	BusHealth bus[BUS_MAX];
	uint8_t configuredCount; // Number of buses enabled in build config
	uint8_t detectedCount;	 // Number of MCP2515 chips responding on SPI
	uint8_t receivingCount;	 // Number of buses that have received frames
	bool allDetected;		 // True when every configured bus has a detected MCP2515
	bool anyDetected;		 // True when at least one MCP2515 is detected
};

/**
 * @brief Build a health report by probing driver-layer MCP2515 state
 * @param mcpDetected Array of per-bus detection flags from the driver layer
 * @param chassisOnline Whether the Chassis bus (index 0) is actively receiving frames
 * @return Populated CanHealthReport summarizing all bus states
 */
inline CanHealthReport checkCanHealth(const bool mcpDetected[], bool chassisOnline)
{
	CanHealthReport r = {};

	for (uint8_t i = 0; i < BUS_MAX; i++)
	{
		r.bus[i].configured = busActive(i);
		r.bus[i].detected = mcpDetected[i];
		r.bus[i].receiving = false;
		r.bus[i].lastRxMs = 0;

		if (r.bus[i].configured)
		{
			r.configuredCount++;
			if (r.bus[i].detected)
				r.detectedCount++;
		}
	}

	// Bus 0 (Chassis) receiving status is derived from the chassisOnline flag
	if (r.bus[0].configured && r.bus[0].detected && chassisOnline)
	{
		r.bus[0].receiving = true;
		r.receivingCount++;
	}

	r.allDetected = (r.detectedCount == r.configuredCount) && r.configuredCount > 0;
	r.anyDetected = r.detectedCount > 0;
	return r;
}

/**
 * @brief Get a human-readable status name for a single bus
 * @param b BusHealth struct to evaluate
 * @return Status string: "disabled", "NOT_DETECTED", "online", or "idle"
 */
inline const char *busHealthName(const BusHealth &b)
{
	if (!b.configured)
		return "disabled";
	if (!b.detected)
		return "NOT_DETECTED";
	if (b.receiving)
		return "online";
	return "idle";
}

/**
 * @brief Get the display name for a bus by its slot index
 * @param i Bus index (0-2)
 * @return Bus name string: "Chassis", "Vehicle", "Body", or "?" for invalid
 */
inline const char *busIndexName(uint8_t i)
{
	switch (i)
	{
	case 0:
		return "Chassis";
	case 1:
		return "Vehicle";
	case 2:
		return "Body";
	default:
		return "?";
	}
}
