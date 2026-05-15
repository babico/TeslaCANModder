#pragma once

/**
 * @file firmware/lib/transport/can/handler/bus/body.h
 * @brief Bus 2 (Body) frame handler stub for future body-side CAN decoders
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "transport/can/bus.h"

/**
 * @brief Handle an incoming frame on the body bus (X179 pins 2-3).
 *
 * The body bus is currently write-only from the device side. Body commands
 * such as window vent, sentry, and trunk control are originated by the
 * command layer and sent fresh, so no incoming-frame caching is required.
 *
 * This stub exists so the bus/ folder mirrors all physical buses and
 * provides a home for future body-side decoders (door-handle telemetry,
 * lock state echo, etc.).
 *
 * @param f Reference to the received CAN frame (unused).
 * @param s Reference to the shared vehicle state (unused).
 */
inline void handleBodyBus(Frame & /*f*/, State & /*s*/)
{
	// No body-bus decoders defined yet; intentionally empty
}
