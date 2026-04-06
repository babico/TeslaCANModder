#pragma once

// ── Pin Configuration ────────────────────────────────────────────────────────
#define PIN_LED 13

// Bus 0 MCP2515 (primary, always required) — tap X179 pins 13/14 (VehicleBus)
#define PIN_MCP2515_CS 10
#define PIN_MCP2515_INT 2

// Bus 1 MCP2515 (secondary, optional) — compile-time flag to enable support.
// Wire a second module to these pins to access a different CAN bus simultaneously
// (e.g. PowertrainBus for pedal/regen/stop-mode controls). Share SPI (D11/D12/D13).
// Note: Legacy code used CS=19, INT=22 on Feather boards. Arduino Uno uses D9/D3.
#define PIN_MCP2515_2_CS 9
#define PIN_MCP2515_2_INT 3

#define PIN_BT_RX 4
#define PIN_BT_TX 5
#define BT_BAUD 9600

// ── Build Flags ──────────────────────────────────────────────────────────────
#ifndef BOARD_ENABLE_BT
  #ifdef ARDUINO_ARCH_AVR
    #define BOARD_ENABLE_BT 1
  #else
    #define BOARD_ENABLE_BT 0
  #endif
#endif

#ifndef BOARD_ENABLE_MCP2515_2
  #define BOARD_ENABLE_MCP2515_2 0
#endif

#ifndef BOARD_CAN_CLOCK_MHZ
  #define BOARD_CAN_CLOCK_MHZ 8
#endif

// ── Board Identity ───────────────────────────────────────────────────────────
#define BOARD_HW_NAME "ArduinoUnoR3CH340"
#define BOARD_CAN_NAME "MCP2515_TJA1050_8MHz"
#define BOARD_DRIVER_NAME "arduino-mcp2515"
#define BOARD_BT_NAME "HC-05"

#if BOARD_ENABLE_BT
  #define BOARD_READY_MSG "Arduino Uno R3 + MCP2515 8MHz ready (USB + HC-05)"
#else
  #define BOARD_READY_MSG "Arduino Uno R3 + MCP2515 8MHz ready (USB only)"
#endif

// ── Timing ───────────────────────────────────────────────────────────────────
#define STATUS_INTERVAL_MS 2000
