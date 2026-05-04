#pragma once
#include "core/types.h"

// ── Shared forward declarations ──────────────────────────────────────────────
// Include this header instead of repeating individual forward declarations.
// Each function is defined by the platform-specific implementation file.

// Defined by core/driver/<board>.h
void driverSend(const Frame &f, uint8_t bus);
uint32_t driverGetAndResetTxFails();
uint32_t driverGetAndResetBusOffEvents();
void driverPollBusErrors();

// Defined by core/persist/<board>.h
void saveSettings(const State &s);

// Defined by handler/dispatch/<board>.h
void resetHandlerLogFlags();
void applyFilters(State &s);
