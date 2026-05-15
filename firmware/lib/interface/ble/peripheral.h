#pragma once

/**
 * @file firmware/lib/interface/ble/peripheral.h
 * @brief Aggregate include for the BLE peripheral subsystem on ESP32
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <NimBLEDevice.h>
#include <atomic>
#include "core/config/esp32.h"
#include "core/types.h"
#include "uuids.h"
#include "state.h"
#include "callbacks.h"
#include "init.h"
#include "print.h"
