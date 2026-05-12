#pragma once

/**
 * @file firmware/lib/vehicle/can/handler/dispatch.h
 * @brief CAN frame dispatch entry point routing incoming frames to bus-specific handlers
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

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

/**
 * @brief Top-level CAN message handler that records, meters, and dispatches a frame
 * @param f Reference to the received CAN frame
 * @param bus Index of the originating bus (0=Chassis, 1=Vehicle, 2=Body)
 * @param s Reference to the global firmware state
 */
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
