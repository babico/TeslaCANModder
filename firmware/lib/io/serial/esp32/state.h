#pragma once
#include "features.h"

static const uint8_t SERIAL_CMD_BUFFER_SIZE = 32;
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
