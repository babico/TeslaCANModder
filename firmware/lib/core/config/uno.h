#pragma once

// ── Pin Configuration ────────────────────────────────────────────────────────
#define PIN_LED 13

// Bus 0: MCP2515_1 (primary, always required) — tap X179 pins 13/14 (VehicleBus)
// SPI shared: SCK=D13, MISO=D12, MOSI=D11
#define PIN_MCP2515_1_CS  10
#define PIN_MCP2515_1_INT 2   // Hardware INT0

// Bus 1: MCP2515_2 (optional)
#define PIN_MCP2515_2_CS  9
#define PIN_MCP2515_2_INT 3   // Hardware INT1

// Bus 2: MCP2515_3 (optional — polled, Uno has only 2 HW interrupts)
#define PIN_MCP2515_3_CS  8
#define PIN_MCP2515_3_INT 6   // Polled (no hardware interrupt)

#define PIN_BT_RX 4
#define PIN_BT_TX 5
#define BT_BAUD 9600

// ── Build Flags ──────────────────────────────────────────────────────────────
// Bus activation controlled by BUS_*_ACTIVE flags in protocol/can.h

#ifndef BOARD_ENABLE_BT
  #ifdef ARDUINO_ARCH_AVR
    #define BOARD_ENABLE_BT 1
  #else
    #define BOARD_ENABLE_BT 0
  #endif
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
