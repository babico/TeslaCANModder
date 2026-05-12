#pragma once

/**
 * @file firmware/lib/io/ble/esp32/uuids.h
 * @brief BLE GATT service and characteristic UUIDs for the TCM peripheral
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

// Nordic UART Service (NUS) UUID — industry-standard BLE serial transport
#define BLE_SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"

// NUS RX characteristic — peripheral receives writes from the central (phone/client)
#define BLE_RX_UUID      "6e400002-b5a3-f393-e0a9-e50e24dcca9e"

// NUS TX characteristic — peripheral notifies the central with outgoing data
#define BLE_TX_UUID      "6e400003-b5a3-f393-e0a9-e50e24dcca9e"
