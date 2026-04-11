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
#include "core/config/uno.h"
#include "core/types.h"
#include "core/persist/uno.h"
#include "core/driver/uno.h"
#include "handler/dispatch/uno.h"
#include "io/serial/uno.h"

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
  sendLog(settingsLoaded ? F("Settings loaded from EEPROM") : F("EEPROM empty - using defaults"));

  extern bool mcpAvailable[];
  for (uint8_t i = 0; i < BUS_MAX; i++) {
    if (!busActive(i)) continue;
    char msg[40];
    if (mcpAvailable[i])
      snprintf(msg, sizeof(msg), "MCP2515_%d ready (Bus %d)", i + 1, i);
    else
      snprintf(msg, sizeof(msg), "MCP2515_%d not detected", i + 1);
    sendLog(msg);
  }
  char busMsg[48];
  snprintf(busMsg, sizeof(busMsg), "Buses: FSD=%d Veh=%d Body=%d",
           BUS_FSD_ACTIVE, BUS_VEHICLE_ACTIVE, BUS_BODY_ACTIVE);
  sendLog(busMsg);
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
  if (state.canOnline && state.lastFrameMs > 0 &&
      (now - state.lastFrameMs) >= CAN_TIMEOUT_MS) {
    state.canOnline = false;
    state.standby = true;
    state.hasCtrl = false;
    state.hasClimate = false;
    state.hasCharge = false;
    state.hasDrive = false;
    state.summonRemaining = 0;
    sendLog(F("CAN bus silent - entering standby"));
  }

  // ── Standby Mode ───────────────────────────────────────────────────────
  if (state.standby) {
    digitalWrite(PIN_LED, (now / (LED_STANDBY_INTERVAL / 2)) % 2 ? HIGH : LOW);

    if (now - state.lastReinitMs >= CAN_REINIT_INTERVAL) {
      state.lastReinitMs = now;
      driverReinit();
      applyFilters(state);
    }

    Frame frame;
    uint8_t bus;
    if (driverRead(frame, bus)) {
      state.standby = false;
      state.canOnline = true;
      state.lastFrameMs = now;
      sendLog(F("CAN bus active - resuming operation"));
      sendFrame(frame, "rx", bus, now, state);
      handleMessage(frame, bus, state);
    }
    return;
  }

  // ── Normal Operation ───────────────────────────────────────────────────
  summonTick(state);

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
