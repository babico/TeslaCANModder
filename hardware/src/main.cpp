/*
 * TeslaCANModder - Arduino Uno Firmware
 * Main entry point - setup() and loop()
 * 
 * This firmware provides runtime-switchable Tesla CAN bus modification
 * with full web UI control over USB and optional Bluetooth.
 */

#include <Arduino.h>
#include <EEPROM.h>
#include <SPI.h>
#include "core/config.h"
#include "core/types.h"
#include "core/persist.h"
#include "core/driver.h"
#include "handler/dispatch.h"
#include "io/serial.h"

static State state;        // Global board state
static bool driverReady = false;
static bool settingsLoaded = false;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);

  // Load saved settings BEFORE sending boot so the boot message reflects
  // the current persisted state (fsd/nag/profile/offset/isaChime/variant).
  settingsLoaded = loadSettings(state);

  // Initialize serial bridge (USB + optional Bluetooth)
  serialInit(state);

  // Initialize MCP2515 CAN driver
  driverReady = driverInit();
  if (!driverReady) {
    sendLog(F("ERROR: CAN init failed. Check wiring and crystal."));
    return;
  }

  // Apply CAN filters for current variant
  applyFilters(state);
  sendLog(F("CAN driver ready"));
  sendLog(settingsLoaded ? F("Settings loaded from EEPROM") : F("EEPROM empty - using defaults"));
#if BOARD_ENABLE_MCP2515_2
  extern bool bus2Available;
  if (bus2Available) {
    sendLog(F("Dual CAN: both buses online"));
  } else {
    sendLog(F("Dual CAN: bus2 init FAILED - single bus fallback"));
  }
#else
  sendLog(F("Single CAN bus mode"));
#endif
}

void loop() {
  // Process incoming commands from USB/Bluetooth
  serialTick(state);

  if (!driverReady) {
    delay(10);
    return;
  }

  unsigned long now = millis();

  // ── CAN Timeout Detection ───────────────────────────────────────────────
  // If we were online and no frame for CAN_TIMEOUT_MS → enter standby
  if (state.canOnline && state.lastFrameMs > 0 &&
      (now - state.lastFrameMs) >= CAN_TIMEOUT_MS) {
    state.canOnline = false;
    state.standby = true;
    // Invalidate stale frame caches so commands don't use old data
    state.hasCtrl = false;
    state.hasClimate = false;
    state.hasCharge = false;
    state.hasDrive = false;
    state.summonRemaining = 0;
    sendLog(F("CAN bus silent - entering standby"));
  }

  // ── Standby Mode ───────────────────────────────────────────────────────
  if (state.standby) {
    // Slow LED blink to indicate standby
    digitalWrite(PIN_LED, (now / (LED_STANDBY_INTERVAL / 2)) % 2 ? HIGH : LOW);

    // Periodically re-init MCP2515 to recover from bus-off / error state
    if (now - state.lastReinitMs >= CAN_REINIT_INTERVAL) {
      state.lastReinitMs = now;
      driverReinit();
      applyFilters(state);
    }

    // Check if CAN frames have returned
    Frame frame;
    uint8_t bus;
    if (driverRead(frame, bus)) {
      // CAN is back! Exit standby
      state.standby = false;
      state.canOnline = true;
      state.lastFrameMs = now;
      sendLog(F("CAN bus active - resuming operation"));
      // Don't reinit — the successful read proves driver is healthy.
      // Reinit would cause a ~20ms blind spot that risks immediate re-standby.
      // Process the frame that woke us
      sendFrame(frame, "rx", bus, now, state);
      handleMessage(frame, bus, state);
    }
    return;
  }

  // ── Normal Operation ───────────────────────────────────────────────────
  // Non-blocking summon burst (sends queued 0x273 injections at 20ms intervals)
  summonTick(state);

  // Read and process all available CAN frames from either bus
  Frame frame;
  uint8_t bus;
  while (driverRead(frame, bus)) {
    state.lastFrameMs = now;
    if (!state.canOnline) {
      state.canOnline = true;
      sendLog(F("CAN bus online"));
    }
    digitalWrite(PIN_LED, LOW);
    sendFrame(frame, "rx", bus, now, state);
    handleMessage(frame, bus, state);
  }
  digitalWrite(PIN_LED, HIGH);
}
