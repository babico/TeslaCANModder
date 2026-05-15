#pragma once

/**
 * @file firmware/lib/interface/ble/state.h
 * @brief Shared mutable state for the BLE peripheral (server, characteristics, ring buffer)
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <NimBLEDevice.h>
#include <atomic>
#include "core/config/esp32.h"

static NimBLEServer *pServer = nullptr;              // NimBLE server instance
static NimBLECharacteristic *pTxChar = nullptr;      // TX (notify) characteristic handle
static NimBLECharacteristic *pRxChar = nullptr;      // RX (write) characteristic handle
static bool bleDeviceConnected = false;              // True while a central is connected
static bool bleReady = false;                        // True after bleInit() completes
static char bleDeviceName[33] = BLE_DEVICE_NAME;     // Advertised device name (max 32 chars + null)

static char bleRxBuf[256];                           // Lock-free ring buffer for incoming bytes
static std::atomic<uint16_t> bleRxHead{0};           // Producer index (written by RX callback)
static std::atomic<uint16_t> bleRxTail{0};           // Consumer index (read by bleRead)
