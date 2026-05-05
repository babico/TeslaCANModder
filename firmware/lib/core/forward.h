#pragma once
#include "core/types.h"

// ── Arduino/native compatibility ─────────────────────────────────────────────
// __FlashStringHelper and F() are Arduino-only. Provide no-op stubs for native
// builds (unit tests, IntelliSense) so headers compile cleanly without Arduino.
#ifndef ARDUINO
class __FlashStringHelper;
#ifndef F
#define F(string_literal) (reinterpret_cast<const __FlashStringHelper *>(string_literal))
#endif
#endif

// ── Shared forward declarations ──────────────────────────────────────────────
// Include this header instead of repeating individual forward declarations.
// Each function is defined by the platform-specific implementation file.

// Logging — implemented by platform serial layer (sendLog(F("...")) on device)
void sendLog(const char *msg);
void sendLog(const __FlashStringHelper *msg);

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
