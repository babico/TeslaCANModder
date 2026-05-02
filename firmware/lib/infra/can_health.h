#pragma once
#include <stdint.h>
#include "infra/can.h"

// ── MCP2515 CAN Bus Health Check ────────────────────────────────────────────
// Validates that all configured MCP2515 modules are physically connected
// and responding on SPI. Uses existing mcpAvailable[] from driver layer.

struct BusHealth
{
	bool configured;   // Bus is enabled in build config
	bool detected;	   // MCP2515 physically detected on SPI
	bool receiving;	   // At least one frame received since boot
	uint32_t lastRxMs; // millis() of last received frame on this bus
};

struct CanHealthReport
{
	BusHealth bus[BUS_MAX];
	uint8_t configuredCount; // How many buses are enabled
	uint8_t detectedCount;	 // How many MCP2515 chips responded
	uint8_t receivingCount;	 // How many buses have received frames
	bool allDetected;		 // All configured buses have MCP2515 detected
	bool anyDetected;		 // At least one MCP2515 detected
};

// Build health report from driver state
// Requires: extern bool mcpAvailable[] from driver layer
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

	// Bus 0 (Chassis) receiving status comes from chassisOnline flag
	if (r.bus[0].configured && r.bus[0].detected && chassisOnline)
	{
		r.bus[0].receiving = true;
		r.receivingCount++;
	}

	r.allDetected = (r.detectedCount == r.configuredCount) && r.configuredCount > 0;
	r.anyDetected = r.detectedCount > 0;
	return r;
}

// Diagnostic summary names
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
