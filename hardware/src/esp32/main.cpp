/*
 * TeslaCANModder - ESP32-S DevKit Firmware
 * Main entry point - setup() and loop()
 *
 * Supports up to 3 CAN buses (3x MCP2515 via SPI),
 * optional WiFi REST API, and optional BLE (Bluetooth Low Energy).
 */

#include <Arduino.h>
#include "core/config/esp32.h"
#include "core/types.h"
#include "core/persist/esp32.h"
#include "core/driver/esp32.h"
#include "handler/dispatch/esp32.h"
#include "io/serial/esp32.h"

#if BOARD_ENABLE_WIFI
  #include "io/wifi/esp32.h"
#endif

static State state;
static bool driverReady = false;
static bool settingsLoaded = false;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);

  // Load saved settings from NVS
  settingsLoaded = loadSettings(state);

  // Initialize serial bridge (USB + optional BLE)
  serialInit(state);

  // Initialize CAN driver(s)
  driverReady = driverInit();
  if (!driverReady) {
    sendLog(F("ERROR: CAN init failed. Check wiring."));
  }

  // Apply CAN filters for current variant
  if (driverReady) {
    applyFilters(state);
  }

  sendLog(settingsLoaded ? F("Settings loaded from NVS") : F("NVS empty - using defaults"));

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

  // Initialize WiFi REST API
#if BOARD_ENABLE_WIFI
  wifiInit(state);
#endif

#if BOARD_ENABLE_BLE
  sendLog(F("BLE active"));
#endif
}

void loop() {
  // Process incoming commands from USB/Bluetooth
  serialTick(state);

  // Handle WiFi REST API requests
#if BOARD_ENABLE_WIFI
  wifiTick();
#endif

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
  preconditionTick(state);

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
