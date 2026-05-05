#pragma once
#include <string.h>
#include "core/forward.h"
#include "vehicle/can/ids.h"
#include "vehicle/can/burst.h"

// ── Control-frame helper ─────────────────────────────────────────────────────
// Produces a copy of the last-seen UI_vehicleControl (0x273) frame, ready
// for bit-manipulation before burst-sending.  All feature controlX() functions
// start with these two lines — centralising them here removes the repetition.
inline Frame makeCtrlFrame(const State &s)
{
	Frame f = {CAN_ID_UI_VEHICLE_CTRL, 8};
	memcpy(f.data, s.lastCtrl, 8);
	return f;
}
