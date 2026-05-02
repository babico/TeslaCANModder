#pragma once
#include <NimBLEDevice.h>
#include <atomic>
#include "core/config/esp32/board.h"

static NimBLEServer *pServer = nullptr;
static NimBLECharacteristic *pTxChar = nullptr;
static NimBLECharacteristic *pRxChar = nullptr;
static bool bleDeviceConnected = false;
static bool bleReady = false;
static char bleDeviceName[33] = BLE_DEVICE_NAME;

static char bleRxBuf[256];
static std::atomic<uint16_t> bleRxHead{0};
static std::atomic<uint16_t> bleRxTail{0};
