#pragma once
#include "core/types.h"

// ── Bus IDs (= array slot index) ─────────────────────────────────────────────
#define BUS_CHASSIS  0
#define BUS_VEHICLE  1
#define BUS_BODY     2

// ── Active flags (injected by build, default off) ────────────────────────────
#ifndef BUS_CHASSIS_ACTIVE
#define BUS_CHASSIS_ACTIVE  0
#endif
#ifndef BUS_VEHICLE_ACTIVE
#define BUS_VEHICLE_ACTIVE  0
#endif
#ifndef BUS_BODY_ACTIVE
#define BUS_BODY_ACTIVE     0
#endif

#define BUS_MAX 3

// ── Compile-time bus tables (indexed by BUS_* id) ────────────────────────────
static const char* const kBusName[BUS_MAX]  = { "chassis", "vehicle", "body" };
static const bool kBusActive[BUS_MAX]       = { BUS_CHASSIS_ACTIVE, BUS_VEHICLE_ACTIVE, BUS_BODY_ACTIVE };
inline bool busActive(uint8_t bus) { return bus < BUS_MAX && kBusActive[bus]; }

