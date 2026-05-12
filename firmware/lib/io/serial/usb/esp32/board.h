#pragma once

/**
 * @file firmware/lib/io/serial/usb/esp32/board.h
 * @brief Top-level include aggregator for the ESP32 USB serial transport
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <Arduino.h>
#include "core/config/esp32/board.h"
#include "core/types.h"
#include "vehicle/can/ids.h"
#include "client/command/features.h"
#include "state.h"
#include "output.h"
#include "client/command/messages.h"
#include "client/command/dispatch.h"
#include "loop.h"
