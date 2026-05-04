#pragma once
#include "core/types.h"

// ── Tesla X179 Connector — Fixed Bus Positions ───────────────────────────────
// Physical MCP2515 modules at fixed pin positions:
//   Bus 0 (MCP2515_1): X179 pins 13-14 → Chassis / Autopilot CAN
//   Bus 1 (MCP2515_2): X179 pins 9-10  → Vehicle Control CAN
//   Bus 2 (MCP2515_3): X179 pins 2-3   → Body Control CAN

#define BUS_CHASSIS 0
#define BUS_VEHICLE 1
#define BUS_BODY 2

// ── Active Bus Flags (set by build: 0=off, 1=on) ────────────────────────────
// If a flag is not provided by the build, default it to off.
#ifndef BUS_CHASSIS_ACTIVE
#define BUS_CHASSIS_ACTIVE 0
#endif
#ifndef BUS_VEHICLE_ACTIVE
#define BUS_VEHICLE_ACTIVE 0
#endif
#ifndef BUS_BODY_ACTIVE
#define BUS_BODY_ACTIVE 0
#endif

// ── Bus Helpers ─────────────────────────────────────────────────────────────
inline bool busActive(uint8_t bus)
{
	switch (bus)
	{
	case BUS_CHASSIS:
		return BUS_CHASSIS_ACTIVE;
	case BUS_VEHICLE:
		return BUS_VEHICLE_ACTIVE;
	case BUS_BODY:
		return BUS_BODY_ACTIVE;
	default:
		return false;
	}
}
#define BUS_MAX 3
