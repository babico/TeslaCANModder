#pragma once
// ── Bus 2 (Body) Frame Handler ───────────────────────────────────────────────
// Body bus (X179 pins 2-3): currently write-only from our side. Body
// commands (window vent, sentry, trunk control) are originated by the
// command layer and sent fresh — no incoming-frame caching is required.
//
// This stub exists so the bus/ folder mirrors all four physical buses and
// future body-side decoders (e.g. door-handle telemetry, lock state echo)
// have an obvious home. Today it is a no-op.

#include "core/forward.h"
#include "core/can/bus.h"

inline void handleBodyBus(Frame & /*f*/, State & /*s*/)
{
	// Intentionally empty — no body-bus decoders defined yet.
}
