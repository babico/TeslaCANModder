#pragma once
#include "client/common/api_fwd.h"
#include "auth.h"
#include "routes.h"
#include "io/ble/esp32/config.h"
#include "core/can/recorder.h"
#if BOARD_ENABLE_BLE
#include "client/gamepad/gamepad.h"
#endif

// ── REST API Init & Tick ─────────────────────────────────────────────────────
//
// HTTP route registration table.
//
// Per-feature endpoints (BLE config, gamepad pairing, recorder start/stop)
// have been removed in favour of the wire-command dispatch endpoint
// `POST /api/command`. Use `ble:on|off|name:<n>|status`, `gamepad:*`, and
// `recorder:on|off|clear|status` instead. The only feature-specific
// endpoint that survives is `GET /api/recorder/download` (binary CSV file
// — has no wire-command equivalent).

void restApiInit(State &s)
{
	restState = &s;
	canRecorderInit();

	// Load or generate API key
	loadOrCreateApiKey(s);

#if BOARD_ENABLE_BLE
	// Load saved BLE configuration from NVS
	loadBleConfig();

	// Apply persisted BLE device name before serving BLE status endpoints.
	bleSetDeviceName(bleNameCfg);

	if (!bleEnabledCfg && bleIsReady())
	{
		bleStop();
		sendLog("BLE disabled (saved config)");
	}

	// Initialize BLE gamepad central role
	gamepadInit();
#endif

	// Register routes
	const char *headerKeys[] = {"X-API-Key"};
	server.collectHeaders(headerKeys, 1);
	server.on("/", HTTP_GET, handleRoot);
	server.on("/api/ping", HTTP_GET, handleGetPing);
	server.on("/api/auth/verify", HTTP_GET,
			  []()
			  {
				  if (!requireAuth())
					  return;
				  sendJsonResponse(200, "{\"ok\":true}");
			  });
	server.on("/api/auth/verify", HTTP_OPTIONS, handleOptions);
	server.on("/api/auth/key", HTTP_GET,
			  []()
			  {
				  // Only available from dashboard (same origin) — returns current API key
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
	server.on("/api/status", HTTP_GET, handleGetStatus);
	server.on("/api/command", HTTP_POST, handlePostCommand);
	server.on("/api/command", HTTP_OPTIONS, handleOptions);
	server.on("/api/status", HTTP_OPTIONS, handleOptions);
	server.on("/api/disable", HTTP_GET, handleDisable);
	// WiFi status/config are handled via POST /api/command with cmd=wifi:status or wifi:config.
#if BOARD_ENABLE_BLE
	// All BLE/gamepad operations are wire commands; see client/command/dispatch.h
#endif
	// Log endpoint
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

	// CAN recorder — only the binary CSV download remains a dedicated route.
	// Use wire commands `recorder:on|off|clear|status` for control.
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

bool restApiIsReady()
{
	return wifiReady;
}
