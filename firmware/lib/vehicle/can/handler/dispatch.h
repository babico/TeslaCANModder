#pragma once
// ── CAN Frame Dispatch Entry Point ───────────────────────────────────────────
// Thin orchestrator. All real work lives in the focused sub-headers:
//
//   handler/helpers.h           — DAS readers, dispatchPlatform, frame-rate
//                                 accounting, log-flag reset
//   handler/filters.h           — applyFilters() per-bus filter setup
//   handler/ticks.h             — summon/precondition/burst/drive-mode ticks
//   handler/bus/chassis.h       — handleChassisBus()  (BUS_CHASSIS,  bus 0)
//   handler/bus/vehicle.h       — handleVehicleBus()  (BUS_VEHICLE,  bus 1)
//   handler/bus/body.h          — handleBodyBus()     (BUS_BODY,     bus 2)
//                                 stub today (body bus is write-only)
//   handler/variant/{hw4,hw3,legacy}.h — per-variant FSD frame handlers
//
// Bus 2 (Body) needs no read-side logic: body commands generate fresh frames.

#include "core/forward.h"
#include "core/can/bus.h"
#include "core/can/recorder.h"
#include "core/driver/esp32/board.h"
#include "handler/helpers.h"
#include "handler/filters.h"
#include "handler/ticks.h"
#include "handler/bus/chassis.h"
#include "handler/bus/vehicle.h"
#include "handler/bus/body.h"

void handleMessage(Frame &f, uint8_t bus, State &s)
{
	canRecorderCapture(f, bus, millis());
	_updateCanFrameRate(s, bus, millis());

	switch (bus)
	{
	case BUS_CHASSIS:
		handleChassisBus(f, s);
		return;
	case BUS_VEHICLE:
		handleVehicleBus(f, s);
		return;
	case BUS_BODY:
		handleBodyBus(f, s);
		return;
	default:
		return;
	}
}
