#pragma once

/**
 * @file firmware/lib/io/serial/usb/esp32/state.h
 * @brief Serial transport runtime state: command buffers, timing, and status flags
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "client/command/features.h"

// Maximum bytes for an incoming RPC JSON envelope (e.g. {"cmd":"..."})
#ifndef SERIAL_CMD_BUFFER_SIZE
#  define SERIAL_CMD_BUFFER_SIZE 64
#endif

static char usbBuf[SERIAL_CMD_BUFFER_SIZE];   // USB serial receive buffer
static uint8_t usbLen = 0;                    // Current byte count in USB buffer
#if BOARD_ENABLE_BLE
static char bleBuf[SERIAL_CMD_BUFFER_SIZE];   // BLE serial receive buffer
static uint8_t bleLen = 0;                    // Current byte count in BLE buffer
#endif
static unsigned long lastStatusMs = 0;        // Timestamp of last status broadcast
static bool statusLiveEnabled = false;        // Whether high-frequency status is active
static const unsigned long STATUS_LIVE_INTERVAL_MS = 250;  // Live status period (ms)
static const unsigned long STATUS_INTERVAL_MS = 5000;      // Normal status period (ms)
