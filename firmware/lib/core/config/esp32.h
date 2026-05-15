#pragma once

/**
 * @file firmware/lib/core/config/esp32.h
 * @brief ESP32-S DevKit pin assignments, bus mapping, and build-flag defaults
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

// ── Pin Configuration ───────────────────────────────────────────────────────

#define PIN_LED 2 // ESP32 DevKit on-board LED (active-high)

// ── BUS_CHASSIS — Chassis / Party CAN, X179 pins 13-14 ─────────────────────
// DAS injection targets this bus (steering/braking/ACC ECUs listen here)
#define PIN_MCP2515_CHASSIS_CS 15
#define PIN_MCP2515_CHASSIS_INT 34 // GPIO34 is input-only, suitable for interrupt

// ── BUS_VEHICLE — Vehicle Control CAN, X179 pins 9-10 ──────────────────────
// BMS, climate, and body controller traffic
#define PIN_MCP2515_VEHICLE_CS 27
#define PIN_MCP2515_VEHICLE_INT 35 // GPIO35 is input-only, suitable for interrupt

// ── BUS_BODY — Body Control CAN, X179 pins 2-3 ─────────────────────────────
// Windows, sentry, trunk controller traffic
#define PIN_MCP2515_BODY_CS 26
#define PIN_MCP2515_BODY_INT 33

// ── SPI (shared between all MCP2515 modules) ────────────────────────────────
#define PIN_SPI_SCK 18
#define PIN_SPI_MISO 19
#define PIN_SPI_MOSI 23

// ── BLE (Bluetooth Low Energy) ──────────────────────────────────────────────
#define BLE_DEVICE_NAME "TeslaCANModder"

// ── WiFi REST API defaults ──────────────────────────────────────────────────
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

// ── Build-flag defaults (overridable via platformio.ini build_flags) ─────────
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
