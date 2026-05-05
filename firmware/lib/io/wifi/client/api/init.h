#pragma once
#include "ble_config.h"
#include "core/can/recorder.h"

// ── WiFi Init & Tick ────────────────────────────────────────────────────────

void wifiInit(State& s) {
  restState = &s;
  canRecorderInit();

  // Load or generate API key
  loadOrCreateApiKey(s);

  // Load saved WiFi configuration from NVS
  loadWifiConfig();

#if BOARD_ENABLE_BLE
  // Load saved BLE configuration from NVS
  loadBleConfig();

  // Apply persisted BLE device name before serving BLE status endpoints.
  bleSetDeviceName(bleNameCfg);

  if (!bleEnabledCfg && bleIsReady()) {
    bleStop();
    sendLog("BLE disabled (saved config)");
  }
#endif

  // Start WiFi in configured mode
  if (wifiCfg.mode == TCM_WIFI_MODE_STA && strlen(wifiCfg.staSSID) > 0) {
    if (!startSTA()) {
      wifiCfg.mode = TCM_WIFI_MODE_AP;
      startAP();
    }
  } else {
    startAP();
  }

  // Register routes
  const char* headerKeys[] = {"X-API-Key"};
  server.collectHeaders(headerKeys, 1);
    server.on("/", HTTP_GET, handleRoot);
    server.on("/api/ping", HTTP_GET, handleGetPing);
    server.on("/api/auth/verify", HTTP_GET, []() {
    if (!requireAuth()) return;
      sendJsonResponse(200, "{\"ok\":true}");
  });
    server.on("/api/auth/verify", HTTP_OPTIONS, handleOptions);
    server.on("/api/auth/key", HTTP_GET, []() {
    // Only available from dashboard (same origin) — returns current API key
      if (!restState) { sendJsonResponse(500, "{\"error\":\"not initialized\"}"); return; }
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
  server.on("/api/wifi/status", HTTP_GET, handleGetWifiStatus);
  server.on("/api/wifi/status", HTTP_OPTIONS, handleOptions);
  server.on("/api/wifi/config", HTTP_POST, handlePostWifiConfig);
  server.on("/api/wifi/config", HTTP_OPTIONS, handleOptions);
#if BOARD_ENABLE_BLE
  server.on("/api/ble/status", HTTP_GET, handleGetBleStatus);
  server.on("/api/ble/status", HTTP_OPTIONS, handleOptions);
  server.on("/api/ble/config", HTTP_POST, handlePostBleConfig);
  server.on("/api/ble/config", HTTP_OPTIONS, handleOptions);
#endif
  // TPMS endpoint
  server.on("/api/tpms", HTTP_GET, []() {
    if (!restState) { sendJsonResponse(500, "{\"error\":\"not initialized\"}"); return; }
    JsonDocument doc;
    doc["ok"] = restState->hasTpms;
    doc["fl"] = (int)(restState->tpmsPressure[0] * 100);
    doc["fr"] = (int)(restState->tpmsPressure[1] * 100);
    doc["rl"] = (int)(restState->tpmsPressure[2] * 100);
    doc["rr"] = (int)(restState->tpmsPressure[3] * 100);
    doc["tfl"] = restState->tpmsTemp[0];
    doc["tfr"] = restState->tpmsTemp[1];
    doc["trl"] = restState->tpmsTemp[2];
    doc["trr"] = restState->tpmsTemp[3];
    String out;
    serializeJson(doc, out);
    sendJsonResponse(200, out);
  });
  server.on("/api/tpms", HTTP_OPTIONS, handleOptions);
  // Log endpoint
  server.on("/api/log", HTTP_GET, []() {
    JsonDocument doc;
    uint16_t cnt = logRingCount();
    doc["count"] = cnt;
    JsonArray arr = doc["entries"].to<JsonArray>();
    for (uint16_t i = 0; i < cnt && i < 64; i++) {
      const LogEntry* e = logRingGet(i);
      if (!e) break;
      JsonObject entry = arr.add<JsonObject>();
      entry["ms"] = e->timestamp;
      entry["m"] = e->msg;
    }
    String out;
    serializeJson(doc, out);
    sendJsonResponse(200, out);
  });
  server.on("/api/log", HTTP_OPTIONS, handleOptions);

  // CAN recorder endpoints
  server.on("/api/recorder/status", HTTP_GET, []() {
    JsonDocument doc;
    doc["enabled"] = canRecorderEnabled();
    doc["count"] = canRecorderCount();
    doc["capacity"] = canRecorderCapacity();
    doc["captured"] = canRecorderCapturedTotal();
    doc["dropped"] = canRecorderDroppedTotal();
    doc["lastCaptureMs"] = canRecorderLastCaptureMs();
    String out;
    serializeJson(doc, out);
    sendJsonResponse(200, out);
  });
  server.on("/api/recorder/status", HTTP_OPTIONS, handleOptions);

  server.on("/api/recorder/start", HTTP_POST, []() {
    if (!requireAuth()) return;
    canRecorderStart(true);
    sendJsonResponse(200, "{\"ok\":true,\"enabled\":true}");
  });
  server.on("/api/recorder/start", HTTP_OPTIONS, handleOptions);

  server.on("/api/recorder/stop", HTTP_POST, []() {
    if (!requireAuth()) return;
    canRecorderStop();
    sendJsonResponse(200, "{\"ok\":true,\"enabled\":false}");
  });
  server.on("/api/recorder/stop", HTTP_OPTIONS, handleOptions);

  server.on("/api/recorder/download", HTTP_GET, []() {
    if (!requireAuth()) return;
    String csv;
    csv.reserve(128 + (size_t)canRecorderCount() * 96);
    csv += "ms,bus,id,dlc,data0,data1,data2,data3,data4,data5,data6,data7\n";

    for (uint16_t i = 0; i < canRecorderCount(); i++) {
      const CanRecorderEntry* e = canRecorderGet(i);
      if (!e) break;
      char line[160];
      snprintf(
        line,
        sizeof(line),
        "%lu,%u,%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u\n",
        e->timestamp,
        e->bus,
        (unsigned long)e->id,
        e->dlc,
        e->data[0],
        e->data[1],
        e->data[2],
        e->data[3],
        e->data[4],
        e->data[5],
        e->data[6],
        e->data[7]
      );
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

void wifiTick() {
  if (wifiReady) {
    server.handleClient();
  }
}

bool wifiIsReady() { return wifiReady; }
