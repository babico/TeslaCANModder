#pragma once

/**
 * @file firmware/lib/vehicle/can/fwd.h
 * @brief Control-frame helper for building UI_vehicleControl frames
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <string.h>
#include "core/forward.h"
#include "vehicle/can/ids.h"
#include "vehicle/can/burst.h"

/**
 * @brief Produce a copy of the last-seen UI_vehicleControl (0x273) frame.
 *
 * Returns a frame ready for bit-manipulation before burst-sending.
 * All feature controlX() functions start with this helper to avoid repetition.
 *
 * @param s Application state containing the last-captured control frame bytes.
 * @return Frame initialized with CAN ID 0x273 and the last-seen payload.
 */
inline Frame makeCtrlFrame(const State &s)
{
	Frame f = {CAN_ID_UI_VEHICLE_CTRL, 8}; // 0x273, 8-byte payload
	memcpy(f.data, s.lastCtrl, 8);
	return f;
}
