#pragma once

// ── ESP32-S DevKit Pin Configuration ─────────────────────────────────────────
// ESP32-S DevKit with up to 3 CAN buses via MCP2515 modules over SPI.
//
// Tesla X179 Connector Mapping (hardcoded):
//   Bus 0 (MCP2515_1): X179 pins 13-14 → FSD / Autopilot CAN
//   Bus 1 (MCP2515_2): X179 pins 9-10  → Vehicle Control CAN
//   Bus 2 (MCP2515_3): X179 pins 2-3   → Body Control CAN
//
// Bus activation is controlled by BUS_FSD_ACTIVE, BUS_VEHICLE_ACTIVE, BUS_BODY_ACTIVE
// Build server injects these flags. Default: FSD only.

#define PIN_LED 2  // ESP32 DevKit on-board LED

// ── Bus 0: MCP2515_1 (SPI) — FSD bus, X179 pins 13-14 ──────────────────────
#define PIN_MCP2515_1_CS   15
#define PIN_MCP2515_1_INT  34   // Input-only pin, good for interrupt

// ── Bus 1: MCP2515_2 (SPI) — Vehicle bus, X179 pins 9-10 ───────────────────
#define PIN_MCP2515_2_CS   27
#define PIN_MCP2515_2_INT  35   // Input-only pin, good for interrupt

// ── Bus 2: MCP2515_3 (SPI) — Body bus, X179 pins 2-3 ───────────────────────
#define PIN_MCP2515_3_CS   26
#define PIN_MCP2515_3_INT  33

// ── SPI (shared between MCP2515 modules) ────────────────────────────────────
#define PIN_SPI_SCK  18
#define PIN_SPI_MISO 19
#define PIN_SPI_MOSI 23

// ── BLE (Bluetooth Low Energy for iPhone) ────────────────────────────────────
#define BLE_DEVICE_NAME "TeslaCANModder"

// ── WiFi REST API ────────────────────────────────────────────────────────────
#ifndef WIFI_AP_SSID
  #define WIFI_AP_SSID     "TeslaCANModder"
#endif

#ifndef WIFI_AP_PASSWORD
  #define WIFI_AP_PASSWORD "teslacan123"
#endif

#ifndef WIFI_AP_CHANNEL
  #define WIFI_AP_CHANNEL  6
#endif

#ifndef WIFI_REST_PORT
  #define WIFI_REST_PORT   80
#endif

// ── Build Flags ──────────────────────────────────────────────────────────────
#ifndef BOARD_ENABLE_BLE
  #define BOARD_ENABLE_BLE 0
#endif

#ifndef BOARD_ENABLE_WIFI
  #define BOARD_ENABLE_WIFI 0
#endif

// Bus activation controlled by BUS_*_ACTIVE flags in protocol/can.h

#ifndef BOARD_CAN_CLOCK_MHZ
  #define BOARD_CAN_CLOCK_MHZ 8
#endif

// ── Board Identity ───────────────────────────────────────────────────────────
#define BOARD_HW_NAME "ESP32S_DevKit"
#define BOARD_CAN_NAME "MCP2515_3x"
#define BOARD_DRIVER_NAME "arduino-mcp2515"

#if BOARD_ENABLE_WIFI && BOARD_ENABLE_BLE
  #define BOARD_READY_MSG "ESP32-S DevKit ready (USB + WiFi + BLE)"
#elif BOARD_ENABLE_WIFI
  #define BOARD_READY_MSG "ESP32-S DevKit ready (USB + WiFi)"
#elif BOARD_ENABLE_BLE
  #define BOARD_READY_MSG "ESP32-S DevKit ready (USB + BLE)"
#else
  #define BOARD_READY_MSG "ESP32-S DevKit ready (USB only)"
#endif

// ── Timing ───────────────────────────────────────────────────────────────────
#define STATUS_INTERVAL_MS 2000
