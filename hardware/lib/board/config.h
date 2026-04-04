#pragma once

#ifndef BOARD_ENABLE_BT
#if defined(ARDUINO_ARCH_AVR)
#define BOARD_ENABLE_BT 1
#else
#define BOARD_ENABLE_BT 0
#endif
#endif

#if BOARD_ENABLE_BT && !defined(ARDUINO_ARCH_AVR)
#error "BOARD_ENABLE_BT requires an AVR build with SoftwareSerial support."
#endif

#ifndef BOARD_CAN_CLOCK_MHZ
#define BOARD_CAN_CLOCK_MHZ 8
#endif

#if BOARD_CAN_CLOCK_MHZ != 8 && BOARD_CAN_CLOCK_MHZ != 16
#error "BOARD_CAN_CLOCK_MHZ must be 8 or 16."
#endif

#ifndef BOARD_CAN_CLOCK_ALLOW_FALLBACK
#define BOARD_CAN_CLOCK_ALLOW_FALLBACK 1
#endif

#ifndef BOARD_BT_RX_PIN
#define BOARD_BT_RX_PIN 4
#endif

#ifndef BOARD_BT_TX_PIN
#define BOARD_BT_TX_PIN 5
#endif

#ifndef BOARD_BT_BAUD
#define BOARD_BT_BAUD 9600
#endif

#ifndef BOARD_HW_NAME
#define BOARD_HW_NAME "ArduinoUnoR3CH340"
#endif

#ifndef BOARD_CAN_NAME
#define BOARD_CAN_NAME "MCP2515_TJA1050_8MHz"
#endif

#ifndef BOARD_DRIVER_NAME
#define BOARD_DRIVER_NAME "arduino-mcp2515"
#endif

#ifndef BOARD_BT_NAME
#define BOARD_BT_NAME "HC-05"
#endif

#ifndef BOARD_HW4_ENABLE_EMERGENCY_BIT59
#define BOARD_HW4_ENABLE_EMERGENCY_BIT59 0
#endif

#ifndef BOARD_STATUS_POLL_INTERVAL_MS
#define BOARD_STATUS_POLL_INTERVAL_MS 500
#endif

namespace board
{

#if BOARD_ENABLE_BT
    static const bool kBluetoothEnabled = true;
    static const char *const kArduinoReadyMessage = "Arduino Uno R3 CH340 + MCP2515 8MHz JSON API ready (USB + HC-05 enabled)";
#else
    static const bool kBluetoothEnabled = false;
    static const char *const kArduinoReadyMessage = "Arduino Uno R3 CH340 + MCP2515 8MHz JSON API ready (USB only)";
#endif

    static const int kCanClockMHz = BOARD_CAN_CLOCK_MHZ;

} // namespace board
