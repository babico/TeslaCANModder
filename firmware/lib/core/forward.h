#pragma once
#include "core/types.h"

// ── Shared forward declarations ──────────────────────────────────────────────
// Include this header instead of repeating individual forward declarations.
// Each function is defined by the platform-specific implementation file.

// Defined by core/driver/{esp32,uno}.h
void driverSend(const Frame& f, uint8_t bus);

// Defined by core/persist/{esp32,uno}.h
void saveSettings(const State& s);

// Defined by handler/dispatch/{esp32,uno}.h
void resetHandlerLogFlags();
void applyFilters(State& s);
