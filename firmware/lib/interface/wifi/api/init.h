#pragma once

/**
 * @file firmware/lib/interface/wifi/api/init.h
 * @brief REST API initialization, HTTP route registration, and tick loop for the WiFi server.
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "interface/wifi/api/fwd.h"
#include "auth.h"
#include "routes.h"
#include "interface/ble/config.h"
#include "transport/can/recorder.h"
#if BOARD_ENABLE_BLE
#include "interface/gamepad/gamepad.h"
#endif

/**
 * @brief Initialize the REST API server, register all HTTP routes, and start listening.
 *
 * Loads or generates the API key, configures BLE if enabled, registers all endpoint
 * handlers, and starts the HTTP server. Per-feature endpoints have been removed in
 * favour of the wire-command dispatch endpoint POST /api/command. The only
 * feature-specific route that remains is GET /api/recorder/download (binary CSV).
 *
 * @param s Global application state shared across all route handlers.
 */
void restApiInit(State &s)
{
	restState = &s;
	canRecorderInit();

	loadOrCreateApiKey(s);

#if BOARD_ENABLE_BLE
	loadBleConfig();
	bleSetDeviceName(bleNameCfg); // Apply persisted BLE device name before serving status

	if (!bleEnabledCfg && bleIsReady())
	{
		bleStop();
		sendLog("BLE disabled (saved config)");
	}

	gamepadInit();
#endif

	// Collect auth header so requireAuth() can read it from incoming requests
	const char *headerKeys[] = {"X-API-Key"};
	server.collectHeaders(headerKeys, 1);

	// Static page routes
	server.on("/", HTTP_GET, handleRoot);
	server.on("/api/ping", HTTP_GET, handleGetPing);

	// Auth verification endpoint
	server.on("/api/auth/verify", HTTP_GET,
			  []()
			  {
				  if (!requireAuth())
					  return;
				  sendJsonResponse(200, "{\"ok\":true}");
			  });
	server.on("/api/auth/verify", HTTP_OPTIONS, handleOptions);

	// API key retrieval — intended for same-origin dashboard access only
	server.on("/api/auth/key", HTTP_GET,
			  []()
			  {
				  if (!restState)
				  {
					  sendJsonResponse(500, "{\"error\":\"not initialized\"}");
					  return;
				  }
				  JsonDocument doc;
				  doc["key"] = restState->apiKey;
				  doc["required"] = restState->apiKeyRequired;
				  String out;
				  serializeJson(doc, out);
				  sendJsonResponse(200, out);
			  });
	server.on("/api/auth/key", HTTP_OPTIONS, handleOptions);

	// Core state and command endpoints
	server.on("/api/status", HTTP_GET, handleGetStatus);
	server.on("/api/command", HTTP_POST, handlePostCommand);
	server.on("/api/command", HTTP_OPTIONS, handleOptions);
	server.on("/api/status", HTTP_OPTIONS, handleOptions);
	server.on("/api/disable", HTTP_GET, handleDisable);

#if BOARD_ENABLE_BLE
	// BLE/gamepad operations use wire commands via POST /api/command
#endif

	// Log ring buffer endpoint — returns the most recent 64 entries
	server.on("/api/log", HTTP_GET,
			  []()
			  {
				  JsonDocument doc;
				  uint16_t cnt = logRingCount();
				  doc["count"] = cnt;
				  JsonArray arr = doc["entries"].to<JsonArray>();
				  for (uint16_t i = 0; i < cnt && i < 64; i++)
				  {
					  const LogEntry *e = logRingGet(i);
					  if (!e)
						  break;
					  JsonObject entry = arr.add<JsonObject>();
					  entry["ms"] = e->timestamp;
					  entry["m"] = e->msg;
				  }
				  String out;
				  serializeJson(doc, out);
				  sendJsonResponse(200, out);
			  });
	server.on("/api/log", HTTP_OPTIONS, handleOptions);

	// CAN recorder binary CSV download — the only feature-specific route remaining.
	// Control via wire commands: recorder:on|off|clear|status
	server.on("/api/recorder/download", HTTP_GET,
			  []()
			  {
				  if (!requireAuth())
					  return;
				  String csv;
				  csv.reserve(128 + (size_t)canRecorderCount() * 96);
				  csv += "ms,bus,id,dlc,data0,data1,data2,data3,data4,data5,data6,data7\n";

				  for (uint16_t i = 0; i < canRecorderCount(); i++)
				  {
					  const CanRecorderEntry *e = canRecorderGet(i);
					  if (!e)
						  break;
					  char line[160];
					  snprintf(line, sizeof(line), "%lu,%u,%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u\n", e->timestamp, e->bus,
							   (unsigned long)e->id, e->dlc, e->data[0], e->data[1], e->data[2], e->data[3], e->data[4],
							   e->data[5], e->data[6], e->data[7]);
					  csv += line;
				  }

				  handleCors();
				  server.sendHeader("Content-Disposition", "attachment; filename=can-recorder.csv");
				  server.send(200, "text/csv", csv);
			  });
	server.on("/api/recorder/download", HTTP_OPTIONS, handleOptions);

	server.onNotFound(handleNotFound);

	server.begin();
	wifiReady = true;
}

/**
 * @brief Process pending HTTP requests and run gamepad tick if BLE is enabled.
 *
 * Must be called from the main loop at a regular interval.
 */
void restApiTick()
{
	if (wifiReady)
	{
		server.handleClient();
	}
#if BOARD_ENABLE_BLE
	gamepadTick(millis());
#endif
}

/**
 * @brief Check whether the REST API server has been initialized and is accepting requests.
 * @return true if the server is running; false otherwise.
 */
bool restApiIsReady()
{
	return wifiReady;
}
