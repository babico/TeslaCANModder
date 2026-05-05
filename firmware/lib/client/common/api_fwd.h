#pragma once
// Shared base for all WiFi REST API client modules.
// Include this to get the server context, core types, and logging without
// depending on a specific io/ driver header or a forced include chain.
#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "core/types.h"
#include "io/log.h"
#include "io/wifi/esp32/state.h"
