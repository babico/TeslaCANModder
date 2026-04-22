#pragma once
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "core/config/esp32.h"
#include "core/types.h"
#include "infra/log_ring.h"
#include "dashboard.h"

// Forward declarations
void sendLog(const char* msg);
void sendLog(const __FlashStringHelper* msg);
static void sendJsonResponse(int code, const String& json);

#if BOARD_ENABLE_BLE
  // BLE forward declarations (defined in ble/esp32.h, included via serial/esp32.h)
  bool bleIsReady();
  bool bleIsConnected();
  const char* bleGetDeviceName();
  bool bleSetDeviceName(const char* name);
  void bleStop();
  void bleRestart();
#endif

static WebServer server(WIFI_REST_PORT);
static bool wifiReady = false;
static State* restState = nullptr;

// ── WiFi Configuration ──────────────────────────────────────────────────────
struct WifiConfig {
  uint8_t mode;          // 0 = AP, 1 = STA
  char apSSID[33];
  char apPassword[65];
  char staSSID[33];
  char staPassword[65];
};

static WifiConfig wifiCfg;
static Preferences wifiPrefs;

#define WIFI_NVS_NS       "tcm_wifi"
#define TCM_WIFI_MODE_AP      0
#define TCM_WIFI_MODE_STA     1
#define WIFI_STA_TIMEOUT  15000  // 15s to connect before fallback to AP
#define WIFI_CFG_MAGIC    0xA1

static void loadWifiConfig() {
  wifiPrefs.begin(WIFI_NVS_NS, true);
  uint8_t magic = wifiPrefs.getUChar("magic", 0);
  if (magic != WIFI_CFG_MAGIC) {
    // Default config
    wifiCfg.mode = TCM_WIFI_MODE_AP;
    strncpy(wifiCfg.apSSID, WIFI_AP_SSID, 32);
    strncpy(wifiCfg.apPassword, WIFI_AP_PASSWORD, 64);
    wifiCfg.staSSID[0] = '\0';
    wifiCfg.staPassword[0] = '\0';
    wifiPrefs.end();
    return;
  }
  wifiCfg.mode = wifiPrefs.getUChar("mode", TCM_WIFI_MODE_AP);
  wifiPrefs.getString("apSSID", wifiCfg.apSSID, 33);
  wifiPrefs.getString("apPW", wifiCfg.apPassword, 65);
  wifiPrefs.getString("staSSID", wifiCfg.staSSID, 33);
  wifiPrefs.getString("staPW", wifiCfg.staPassword, 65);
  wifiPrefs.end();
}

static void saveWifiConfig() {
  wifiPrefs.begin(WIFI_NVS_NS, false);
  wifiPrefs.putUChar("magic", WIFI_CFG_MAGIC);
  wifiPrefs.putUChar("mode", wifiCfg.mode);
  wifiPrefs.putString("apSSID", wifiCfg.apSSID);
  wifiPrefs.putString("apPW", wifiCfg.apPassword);
  wifiPrefs.putString("staSSID", wifiCfg.staSSID);
  wifiPrefs.putString("staPW", wifiCfg.staPassword);
  wifiPrefs.end();
}

static void startAP() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(wifiCfg.apSSID, strlen(wifiCfg.apPassword) >= 8 ? wifiCfg.apPassword : NULL,
               WIFI_AP_CHANNEL, 0, 4);
  sendLog("WiFi AP started");
  char msg[64];
  snprintf(msg, sizeof(msg), "AP SSID: %s  IP: %s", wifiCfg.apSSID, WiFi.softAPIP().toString().c_str());
  sendLog(msg);
}

static bool startSTA() {
  if (strlen(wifiCfg.staSSID) == 0) return false;
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiCfg.staSSID, strlen(wifiCfg.staPassword) > 0 ? wifiCfg.staPassword : NULL);
  sendLog("WiFi STA connecting...");
  char msg[64];
  snprintf(msg, sizeof(msg), "SSID: %s", wifiCfg.staSSID);
  sendLog(msg);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_STA_TIMEOUT) {
    delay(250);
  }
  if (WiFi.status() == WL_CONNECTED) {
    snprintf(msg, sizeof(msg), "Connected! IP: %s RSSI: %d", WiFi.localIP().toString().c_str(), WiFi.RSSI());
    sendLog(msg);
    return true;
  }
  sendLog("STA connection failed, falling back to AP");
  return false;
}

// ── JSON State Builder ──────────────────────────────────────────────────────
static String buildStateJson(State& s) {
  StaticJsonDocument<2048> doc;
  Features feat = getFeatures(s.variant);

  doc["variant"] = variantName(s.variant);
  doc["fsd"] = s.fsdEnabled;
  doc["nag"] = s.nagSuppress;
  doc["profile"] = s.speedProfile;
  doc["profilePin"] = s.profileOverride;
  doc["offset"] = s.speedOffset;
  doc["offsetPin"] = s.offsetOverride;
  doc["isaChime"] = s.isaChimeSuppress;
  doc["stream"] = s.streamEnabled;
  doc["rawCan"] = s.rawCanListen;
  doc["chassisOnline"] = s.chassisOnline;
  doc["standby"] = s.standby;
  doc["uptime"] = millis();
  doc["driveMode"] = s.driveModeOverride;
  doc["currentDriveMode"] = s.currentDriveMode;
  doc["eceR79"] = s.eceR79Bypass;
  doc["regionCode"] = s.regionCode;
  doc["hasRegion"] = s.hasRegion;
  doc["cnLocked"] = s.chineseGatewayLocked;
  doc["hasTpms"] = s.hasTpms;
  doc["seatbeltEmulation"] = s.seatbeltEmulation;
  doc["wiperPersist"] = s.wiperPersistEnabled;
  doc["mirrorAutoFold"] = s.mirrorAutoFoldEnabled;
  doc["canSim"] = s.canSimEnabled;
  doc["hasPowertrain"] = s.hasPowertrain;
  doc["otaInProgress"] = s.otaInProgress;
  doc["txPaused"] = s.txPaused;
  doc["turnSignalLeft"] = s.turnSignalLeft;
  doc["turnSignalRight"] = s.turnSignalRight;
  doc["bsmLeftLevel"] = s.bsmLeftLevel;
  doc["bsmRightLevel"] = s.bsmRightLevel;
  doc["doorFrontLeftOpen"] = s.doorFrontLeftOpen;
  doc["doorFrontRightOpen"] = s.doorFrontRightOpen;
  doc["doorRearLeftOpen"] = s.doorRearLeftOpen;
  doc["doorRearRightOpen"] = s.doorRearRightOpen;
  doc["driverDoorOpen"] = s.driverDoorOpen;
  doc["anyDoorOpen"] = s.anyDoorOpen;
  doc["frunkOpen"] = s.frunkOpen;
  doc["trunkOpen"] = s.trunkOpen;
  doc["cruiseSetSpeed"] = (int)(s.cruiseSetSpeedKph * 10);
  doc["accSpeedLimit"] = (int)(s.accSpeedLimitKph * 10);
  doc["mapSpeedLimit"] = (int)(s.mapSpeedLimitKph * 10);
  doc["maxSpeed"] = (int)(s.maxSpeedKph * 10);

  if (s.hasTpms) {
    JsonObject tpms = doc.createNestedObject("tpms");
    tpms["fl"] = (int)(s.tpmsPressure[0] * 100);
    tpms["fr"] = (int)(s.tpmsPressure[1] * 100);
    tpms["rl"] = (int)(s.tpmsPressure[2] * 100);
    tpms["rr"] = (int)(s.tpmsPressure[3] * 100);
    tpms["tfl"] = s.tpmsTemp[0];
    tpms["tfr"] = s.tpmsTemp[1];
    tpms["trl"] = s.tpmsTemp[2];
    tpms["trr"] = s.tpmsTemp[3];
  }

  if (s.hasPowertrain) {
    JsonObject pt = doc.createNestedObject("powertrain");
    pt["speed"] = (int)(s.vehicleSpeed * 100);
    pt["gear"] = s.gearState;
    pt["pedal"] = s.accelPedal;
    pt["steer"] = (int)(s.steeringAngle * 10);
    pt["rpmR"] = s.rearMotorRpm;
    pt["rpmF"] = s.frontMotorRpm;
  }

  JsonObject f = doc.createNestedObject("features");
  f["fsd"] = feat.fsd;
  f["profile"] = feat.profile;
  f["nag"] = feat.nag;
  f["offset"] = feat.offset;
  f["isaChime"] = feat.isaChime;
  f["summon"] = feat.summon;

  JsonObject hw = doc.createNestedObject("hardware");
  hw["board"] = BOARD_HW_NAME;
  hw["can"] = BOARD_CAN_NAME;
  hw["busChassis"] = (int)BUS_CHASSIS_ACTIVE;
  hw["busVehicle"] = (int)BUS_VEHICLE_ACTIVE;
  hw["busBody"] = (int)BUS_BODY_ACTIVE;
#if BOARD_ENABLE_BLE
  hw["ble"] = true;
#else
  hw["ble"] = false;
#endif
  hw["wifi"] = true;
  hw["ip"] = (wifiCfg.mode == TCM_WIFI_MODE_STA) ? WiFi.localIP().toString() : WiFi.softAPIP().toString();

  String output;
  serializeJson(doc, output);
  return output;
}

// ── REST API Route Handlers ─────────────────────────────────────────────────

// Forward declare command executor (defined in serial/esp32.h)
void executeCommand(const char* cmd, State& s, unsigned long now);

// ── API Key Generation & Validation ─────────────────────────────────────────
static Preferences authPrefs;
#define AUTH_NVS_NS "tcm_auth"

static void generateApiKey(char* key, size_t len) {
  static const char hex[] = "0123456789abcdef";
  for (size_t i = 0; i < len - 1; i++) {
    key[i] = hex[esp_random() % 16];
  }
  key[len - 1] = '\0';
}

static void loadOrCreateApiKey(State& s) {
  authPrefs.begin(AUTH_NVS_NS, true);
  String saved = authPrefs.getString("key", "");
  s.apiKeyRequired = authPrefs.getBool("required", false);
  authPrefs.end();

  if (saved.length() == 32) {
    strncpy(s.apiKey, saved.c_str(), sizeof(s.apiKey) - 1);
    s.apiKey[sizeof(s.apiKey) - 1] = '\0';
  } else {
    generateApiKey(s.apiKey, sizeof(s.apiKey));
    authPrefs.begin(AUTH_NVS_NS, false);
    authPrefs.putString("key", s.apiKey);
    authPrefs.putBool("required", s.apiKeyRequired);
    authPrefs.end();
    char msg[80];
    snprintf(msg, sizeof(msg), "API key generated: %.8s...", s.apiKey);
    sendLog(msg);
  }
}

static bool requireAuth() {
  if (!restState || !restState->apiKeyRequired) return true;
  String provided = server.header("X-API-Key");
  if (provided.length() == 0) provided = server.arg("apiKey");
  if (provided.length() == 0) {
    sendJsonResponse(401, "{\"error\":\"missing X-API-Key header\"}");
    return false;
  }
  if (strncmp(provided.c_str(), restState->apiKey, 32) != 0) {
    sendJsonResponse(403, "{\"error\":\"invalid API key\"}");
    return false;
  }
  return true;
}

// ── CORS Handling ───────────────────────────────────────────────────────────
static void handleCors() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type, X-API-Key");
}

static void sendJsonResponse(int code, const String& json) {
  handleCors();
  server.send(code, "application/json", json);
}

static void handleOptions() {
  handleCors();
  server.send(204);
}

static void handleRoot() {
  server.send_P(200, "text/html", DASH_HTML);
}

static void handleGetStatus() {
  if (!restState) { sendJsonResponse(500, "{\"error\":\"not initialized\"}"); return; }
  sendJsonResponse(200, buildStateJson(*restState));
}

static void handlePostCommand() {
  if (!restState) { sendJsonResponse(500, "{\"error\":\"not initialized\"}"); return; }
  if (!requireAuth()) return;

  String body = server.arg("plain");
  if (body.length() == 0) {
    sendJsonResponse(400, "{\"error\":\"empty body\"}");
    return;
  }

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, body);
  if (err) {
    sendJsonResponse(400, "{\"error\":\"invalid json\"}");
    return;
  }

  const char* cmd = doc["cmd"];
  if (!cmd || strlen(cmd) == 0 || strlen(cmd) > 31) {
    sendJsonResponse(400, "{\"error\":\"missing or invalid cmd\"}");
    return;
  }

  // Validate command characters (same as serial parser)
  for (size_t i = 0; i < strlen(cmd); i++) {
    char c = cmd[i];
    bool valid = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                 (c >= '0' && c <= '9') || c == ':' || c == '-' || c == '_';
    if (!valid) {
      sendJsonResponse(400, "{\"error\":\"invalid characters in cmd\"}");
      return;
    }
  }

  executeCommand(cmd, *restState, millis());

  sendJsonResponse(200, buildStateJson(*restState));
}

static void handleGetPing() {
  sendJsonResponse(200, "{\"t\":\"pong\",\"v\":1}");
}

static void handleDisable() {
  if (!restState) { sendJsonResponse(500, "{\"error\":\"not initialized\"}"); return; }
  if (!requireAuth()) return;
  restState->fsdEnabled = false;
  restState->summonRemaining = 0;
  sendJsonResponse(200, "{\"ok\":true,\"msg\":\"All injections disabled\"}");
}

static void handleNotFound() {
  handleCors();
  server.send(404, "application/json", "{\"error\":\"not found\"}");
}

// ── WiFi Status & Config Endpoints ──────────────────────────────────────────

static String buildWifiStatusJson() {
  StaticJsonDocument<384> doc;
  doc["mode"] = (wifiCfg.mode == TCM_WIFI_MODE_STA) ? "sta" : "ap";

  if (wifiCfg.mode == TCM_WIFI_MODE_STA) {
    doc["ssid"] = wifiCfg.staSSID;
    doc["ip"] = WiFi.localIP().toString();
    doc["rssi"] = WiFi.RSSI();
    doc["connected"] = (WiFi.status() == WL_CONNECTED);
    doc["gateway"] = WiFi.gatewayIP().toString();
    doc["mac"] = WiFi.macAddress();
  } else {
    doc["ssid"] = wifiCfg.apSSID;
    doc["ip"] = WiFi.softAPIP().toString();
    doc["clients"] = WiFi.softAPgetStationNum();
    doc["channel"] = WIFI_AP_CHANNEL;
    doc["mac"] = WiFi.softAPmacAddress();
  }

  String out;
  serializeJson(doc, out);
  return out;
}

static void handleGetWifiStatus() {
  sendJsonResponse(200, buildWifiStatusJson());
}

static void handlePostWifiConfig() {
  if (!requireAuth()) return;
  String body = server.arg("plain");
  if (body.length() == 0) {
    sendJsonResponse(400, "{\"error\":\"empty body\"}");
    return;
  }

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, body);
  if (err) {
    sendJsonResponse(400, "{\"error\":\"invalid json\"}");
    return;
  }

  const char* mode = doc["mode"];
  const char* ssid = doc["ssid"];
  const char* password = doc["password"];

  if (!mode || (strcmp(mode, "ap") != 0 && strcmp(mode, "sta") != 0)) {
    sendJsonResponse(400, "{\"error\":\"mode must be ap or sta\"}");
    return;
  }

  if (strcmp(mode, "sta") == 0) {
    if (!ssid || strlen(ssid) == 0 || strlen(ssid) > 32) {
      sendJsonResponse(400, "{\"error\":\"invalid sta ssid\"}");
      return;
    }
    strncpy(wifiCfg.staSSID, ssid, 32);
    wifiCfg.staSSID[32] = '\0';
    if (password && strlen(password) > 0) {
      strncpy(wifiCfg.staPassword, password, 64);
      wifiCfg.staPassword[64] = '\0';
    } else {
      wifiCfg.staPassword[0] = '\0';
    }
    wifiCfg.mode = TCM_WIFI_MODE_STA;
    saveWifiConfig();

    // Try connecting to STA; fall back to AP if it fails
    if (!startSTA()) {
      wifiCfg.mode = TCM_WIFI_MODE_AP;
      saveWifiConfig();
      startAP();
    }
  } else {
    // AP mode
    if (ssid && strlen(ssid) > 0 && strlen(ssid) <= 32) {
      strncpy(wifiCfg.apSSID, ssid, 32);
      wifiCfg.apSSID[32] = '\0';
    }
    if (password) {
      if (strlen(password) == 0) {
        wifiCfg.apPassword[0] = '\0';  // Open AP
      } else if (strlen(password) >= 8 && strlen(password) <= 64) {
        strncpy(wifiCfg.apPassword, password, 64);
        wifiCfg.apPassword[64] = '\0';
      } else {
        sendJsonResponse(400, "{\"error\":\"AP password must be 8-64 chars or empty\"}");
        return;
      }
    }
    wifiCfg.mode = TCM_WIFI_MODE_AP;
    saveWifiConfig();
    startAP();
  }

  // Restart the web server on the new connection
  server.begin();
  sendJsonResponse(200, buildWifiStatusJson());
}

// ── BLE Status & Config Endpoints ───────────────────────────────────────────

#if BOARD_ENABLE_BLE
static Preferences blePrefs;
#define BLE_NVS_NS "tcm_ble"

static bool bleEnabledCfg = true;  // default: enabled
static char bleNameCfg[33] = BLE_DEVICE_NAME;

static void loadBleConfig() {
  blePrefs.begin(BLE_NVS_NS, true);
  bleEnabledCfg = blePrefs.getBool("enabled", true);
  String savedName = blePrefs.getString("name", BLE_DEVICE_NAME);
  if (savedName.length() == 0 || savedName.length() > 32) {
    strncpy(bleNameCfg, BLE_DEVICE_NAME, sizeof(bleNameCfg) - 1);
    bleNameCfg[sizeof(bleNameCfg) - 1] = '\0';
  } else {
    strncpy(bleNameCfg, savedName.c_str(), sizeof(bleNameCfg) - 1);
    bleNameCfg[sizeof(bleNameCfg) - 1] = '\0';
  }
  blePrefs.end();
}

static void saveBleConfig() {
  blePrefs.begin(BLE_NVS_NS, false);
  blePrefs.putBool("enabled", bleEnabledCfg);
  blePrefs.putString("name", bleNameCfg);
  blePrefs.end();
}

static String buildBleStatusJson() {
  StaticJsonDocument<192> doc;
  doc["enabled"] = bleIsReady();
  doc["connected"] = bleIsConnected();
  doc["deviceName"] = bleGetDeviceName();
  String out;
  serializeJson(doc, out);
  return out;
}

static void handleGetBleStatus() {
  sendJsonResponse(200, buildBleStatusJson());
}

static void handlePostBleConfig() {
  if (!requireAuth()) return;
  String body = server.arg("plain");
  if (body.length() == 0) {
    sendJsonResponse(400, "{\"error\":\"empty body\"}");
    return;
  }

  StaticJsonDocument<128> doc;
  DeserializationError err = deserializeJson(doc, body);
  if (err) {
    sendJsonResponse(400, "{\"error\":\"invalid json\"}");
    return;
  }

  bool hasEnabled = doc.containsKey("enabled");
  bool hasName = doc.containsKey("name") || doc.containsKey("deviceName");

  if (!hasEnabled && !hasName) {
    sendJsonResponse(400, "{\"error\":\"missing enabled or name field\"}");
    return;
  }

  if (hasName) {
    const char* requestedName = doc.containsKey("name") ? doc["name"] : doc["deviceName"];
    size_t nameLen = requestedName ? strlen(requestedName) : 0;
    if (nameLen == 0 || nameLen > 32) {
      sendJsonResponse(400, "{\"error\":\"BLE name must be 1-32 characters\"}");
      return;
    }
    strncpy(bleNameCfg, requestedName, sizeof(bleNameCfg) - 1);
    bleNameCfg[sizeof(bleNameCfg) - 1] = '\0';
    if (!bleSetDeviceName(bleNameCfg)) {
      sendJsonResponse(400, "{\"error\":\"failed to apply BLE name\"}");
      return;
    }
    sendLog("BLE name updated via REST");
  }

  if (hasEnabled) {
    bool en = doc["enabled"];
    bleEnabledCfg = en;

    if (en && !bleIsReady()) {
      bleRestart();
      sendLog("BLE enabled via REST");
    } else if (!en && bleIsReady()) {
      bleStop();
      sendLog("BLE disabled via REST");
    }
  }

  saveBleConfig();

  sendJsonResponse(200, buildBleStatusJson());
}
#endif

// ── WiFi Init & Tick ────────────────────────────────────────────────────────

void wifiInit(State& s) {
  restState = &s;

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
    StaticJsonDocument<128> doc;
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
    StaticJsonDocument<256> doc;
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
    StaticJsonDocument<2048> doc;
    uint16_t cnt = logRingCount();
    doc["count"] = cnt;
    JsonArray arr = doc.createNestedArray("entries");
    for (uint16_t i = 0; i < cnt && i < 64; i++) {
      const LogEntry* e = logRingGet(i);
      if (!e) break;
      JsonObject entry = arr.createNestedObject();
      entry["ms"] = e->timestamp;
      entry["m"] = e->msg;
    }
    String out;
    serializeJson(doc, out);
    sendJsonResponse(200, out);
  });
  server.on("/api/log", HTTP_OPTIONS, handleOptions);
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
