#pragma once

/**
 * @file firmware/lib/io/serial/bluetooth/esp32/init.h
 * @brief Bluetooth Serial initialization and readiness check for ESP32
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <BluetoothSerial.h>
#include "core/config/esp32/board.h"

static BluetoothSerial btSerial;
static bool btReady = false;

/**
 * @brief Initialize the Bluetooth Serial interface with the configured device name
 */
void btInit()
{
	btSerial.begin(BLE_DEVICE_NAME);
	btReady = true;
}

/**
 * @brief Check whether the Bluetooth Serial interface is ready
 * @return true if btInit() has completed successfully
 */
bool btIsReady() { return btReady; }
