#pragma once
#include "io/serial/client/command/features.h"

// RpcRequest JSON envelope {"cmd":"..."} needs room for the longest method string
// plus ~10 bytes of JSON overhead. 64 bytes covers all defined RPC methods.
#ifndef SERIAL_CMD_BUFFER_SIZE
#  define SERIAL_CMD_BUFFER_SIZE 64
#endif

static char usbBuf[SERIAL_CMD_BUFFER_SIZE];
static uint8_t usbLen = 0;
#if BOARD_ENABLE_BLE
static char bleBuf[SERIAL_CMD_BUFFER_SIZE];
static uint8_t bleLen = 0;
#endif
static unsigned long lastStatusMs = 0;
static bool statusLiveEnabled = false;
static const unsigned long STATUS_LIVE_INTERVAL_MS = 250;
static const unsigned long STATUS_INTERVAL_MS = 5000;
