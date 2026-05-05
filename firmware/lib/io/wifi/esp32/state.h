#pragma once
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "core/config/esp32/board.h"
#include "core/types.h"
#include "core/log/ring.h"
#include "io/log.h"
#include "client/dashboard/dashboard.h"
static void sendJsonResponse(int code, const String &json);

#if BOARD_ENABLE_BLE
// BLE forward declarations (defined in ble/board.h, included via serial/board.h)
bool bleIsReady();
bool bleIsConnected();
const char *bleGetDeviceName();
bool bleSetDeviceName(const char *name);
void bleStop();
void bleRestart();
#endif

static WebServer server(WIFI_REST_PORT);
static bool wifiReady = false;
static State *restState = nullptr;
