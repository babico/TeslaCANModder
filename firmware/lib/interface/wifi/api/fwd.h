#pragma once

/**
 * @file firmware/lib/interface/wifi/api/fwd.h
 * @brief Shared forward-include base for all WiFi REST API client modules
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 *
 * Include this header to pull in the WebServer context, core types, JSON
 * support, persistent storage, and logging without depending on a specific
 * io/ driver header or introducing a forced include chain.
 */

#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "core/types.h"
#include "core/forward.h"
#include "interface/wifi/state.h"
