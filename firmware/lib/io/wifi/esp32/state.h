#pragma once

/**
 * @file firmware/lib/io/wifi/esp32/state.h
 * @brief Shared WiFi transport state: server instance, connection helpers, and forward declarations
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "core/config/esp32/board.h"
#include "core/types.h"
#include "core/log/ring.h"
#include "io/log.h"
#include "client/dashboard/dashboard.h"

/**
 * @brief Send an HTTP JSON response with the given status code and body.
 * @param code HTTP status code (e.g. 200, 400).
 * @param json Serialized JSON string to send as the response body.
 */
static void sendJsonResponse(int code, const String &json);

#if BOARD_ENABLE_BLE
/**
 * @brief Check whether the BLE subsystem has completed initialization.
 * @return True if BLE is ready to accept connections.
 */
bool bleIsReady();

/**
 * @brief Check whether a BLE client is currently connected.
 * @return True if a peer device is connected.
 */
bool bleIsConnected();

/**
 * @brief Get the advertised BLE device name.
 * @return Pointer to the null-terminated device name string.
 */
const char *bleGetDeviceName();

/**
 * @brief Set a new BLE device name and restart advertising.
 * @param name Null-terminated string for the new device name.
 * @return True if the name was accepted and applied.
 */
bool bleSetDeviceName(const char *name);

/**
 * @brief Stop the BLE stack and release resources.
 */
void bleStop();

/**
 * @brief Restart the BLE stack after a stop.
 */
void bleRestart();
#endif

static WebServer server(WIFI_REST_PORT); // HTTP server bound to the configured REST port
static bool wifiReady = false;           // True once the WiFi transport is fully initialized
static State *restState = nullptr;       // Pointer to the shared firmware state used by REST handlers

/**
 * @brief Get the active IP address regardless of AP or STA mode.
 * @return IP address string from STA if available, otherwise the soft-AP IP.
 */
static inline String wifiCurrentIP()
{
	IPAddress ip = WiFi.localIP();
	if ((uint32_t)ip != 0) return ip.toString(); // STA has a valid IP assigned
	return WiFi.softAPIP().toString();
}
