#pragma once

/**
 * @file firmware/lib/core/can/bus.h
 * @brief CAN bus index definitions, active flags, and compile-time bus tables
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/types.h"

#define BUS_CHASSIS  0	// Array slot index for Chassis bus (X179 pins 13-14)
#define BUS_VEHICLE  1	// Array slot index for Vehicle bus (X179 pins 9-10)
#define BUS_BODY     2	// Array slot index for Body bus (X179 pins 2-3)

#ifndef BUS_CHASSIS_ACTIVE
#define BUS_CHASSIS_ACTIVE  0	// Build flag: set to 1 to enable Chassis bus
#endif
#ifndef BUS_VEHICLE_ACTIVE
#define BUS_VEHICLE_ACTIVE  0	// Build flag: set to 1 to enable Vehicle bus
#endif
#ifndef BUS_BODY_ACTIVE
#define BUS_BODY_ACTIVE     0	// Build flag: set to 1 to enable Body bus
#endif

#define BUS_MAX 3	// Total number of physical CAN bus slots

static const char* const kBusName[BUS_MAX]  = { "chassis", "vehicle", "body" };
static const bool kBusActive[BUS_MAX]       = { BUS_CHASSIS_ACTIVE, BUS_VEHICLE_ACTIVE, BUS_BODY_ACTIVE };

/**
 * @brief Check whether a given bus slot is enabled in the current build
 * @param bus Bus index (0 = Chassis, 1 = Vehicle, 2 = Body)
 * @return true if the bus index is valid and the bus is active
 */
inline bool busActive(uint8_t bus) { return bus < BUS_MAX && kBusActive[bus]; }
