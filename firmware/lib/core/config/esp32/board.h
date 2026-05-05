#pragma once

// ── ESP32-S DevKit Pin Configuration ─────────────────────────────────────────
// ESP32-S DevKit with up to 3 CAN buses via MCP2515 modules over SPI.
// Pins are named by bus role (CHASSIS/VEHICLE/BODY), not by physical module
// index. The BUS_* IDs in core/can/bus.h provide the array-index map.
//
// Tesla X179 Connector Mapping (hardcoded):
//   BUS_CHASSIS : X179 pins 13-14 → Chassis bus (vehicle ECUs: rack, ESP, IBST).
//                 *DAS injection always targets this bus* — that's where
//                 steering/braking/ACC ECUs listen.
//   BUS_VEHICLE : X179 pins 9-10  → Vehicle Control CAN (BMS, climate, body)
//   BUS_BODY    : X179 pins 2-3   → Body Control CAN (windows, sentry, trunk)
//
// Bus activation is controlled by BUS_<NAME>_ACTIVE flags injected by the build.

#define PIN_LED 2 // ESP32 DevKit on-board LED

// ── BUS_CHASSIS — Chassis / Party CAN, X179 pins 13-14 ──────────────────────
#define PIN_MCP2515_CHASSIS_CS 15
#define PIN_MCP2515_CHASSIS_INT 34 // Input-only pin, good for interrupt

// ── BUS_VEHICLE — Vehicle Control CAN, X179 pins 9-10 ───────────────────────
#define PIN_MCP2515_VEHICLE_CS 27
#define PIN_MCP2515_VEHICLE_INT 35 // Input-only pin, good for interrupt

// ── BUS_BODY — Body Control CAN, X179 pins 2-3 ──────────────────────────────
#define PIN_MCP2515_BODY_CS 26
#define PIN_MCP2515_BODY_INT 33

// ── SPI (shared between MCP2515 modules) ────────────────────────────────────
#define PIN_SPI_SCK 18
#define PIN_SPI_MISO 19
#define PIN_SPI_MOSI 23

// ── BLE (Bluetooth Low Energy for iPhone) ────────────────────────────────────
#define BLE_DEVICE_NAME "TeslaCANModder"

// ── WiFi REST API ────────────────────────────────────────────────────────────
#ifndef WIFI_AP_SSID
#define WIFI_AP_SSID "TeslaCANModder"
#endif

#ifndef WIFI_AP_PASSWORD
#define WIFI_AP_PASSWORD "T3SL@c@n123."
#endif

#ifndef WIFI_AP_CHANNEL
#define WIFI_AP_CHANNEL 6
#endif

#ifndef WIFI_REST_PORT
#define WIFI_REST_PORT 80
#endif

// ── Build Flags ──────────────────────────────────────────────────────────────
#ifndef BOARD_HW_NAME
#define BOARD_HW_NAME "ESP32S_DevKit"
#endif

#ifndef BOARD_CAN_NAME
#define BOARD_CAN_NAME "MCP2515"
#endif

#ifndef BOARD_DRIVER_NAME
#define BOARD_DRIVER_NAME "arduino-mcp2515"
#endif

#ifndef BOARD_READY_MSG
#define BOARD_READY_MSG "TeslaCANModder ready"
#endif

#ifndef BOARD_ENABLE_BLE
#define BOARD_ENABLE_BLE 0
#endif

#ifndef BOARD_ENABLE_WIFI
#define BOARD_ENABLE_WIFI 0
#endif
