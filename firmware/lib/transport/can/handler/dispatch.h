#pragma once

/**
 * @file firmware/lib/transport/can/handler/dispatch.h
 * @brief CAN frame dispatch entry point routing incoming frames to bus-specific handlers
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "transport/can/bus.h"
#include "transport/can/recorder.h"
#include "transport/can/esp32.h"
#include "helpers.h"
#include "filters.h"
#include "ticks.h"
#include "bus/chassis.h"
#include "bus/vehicle.h"
#include "bus/body.h"

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
