#pragma once
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "core/config/esp32.h"
#include "core/types.h"
#include "dashboard.h"

// Forward declarations
void sendLog(const char* msg);
void sendLog(const __FlashStringHelper* msg);

#if BOARD_ENABLE_BLE
  // BLE forward declarations (defined in ble/esp32.h, included via serial/esp32.h)
  bool bleIsReady();
  bool bleIsConnected();
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
#define WIFI_MODE_AP      0
#define WIFI_MODE_STA     1
#define WIFI_STA_TIMEOUT  15000  // 15s to connect before fallback to AP
#define WIFI_CFG_MAGIC    0xA1

static void loadWifiConfig() {
  wifiPrefs.begin(WIFI_NVS_NS, true);
  uint8_t magic = wifiPrefs.getUChar("magic", 0);
  if (magic != WIFI_CFG_MAGIC) {
    // Default config
    wifiCfg.mode = WIFI_MODE_AP;
    strncpy(wifiCfg.apSSID, WIFI_AP_SSID, 32);
    strncpy(wifiCfg.apPassword, WIFI_AP_PASSWORD, 64);
    wifiCfg.staSSID[0] = '\0';
    wifiCfg.staPassword[0] = '\0';
    wifiPrefs.end();
    return;
  }
  wifiCfg.mode = wifiPrefs.getUChar("mode", WIFI_MODE_AP);
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
  StaticJsonDocument<1024> doc;
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
  doc["canOnline"] = s.canOnline;
  doc["standby"] = s.standby;
  doc["uptime"] = millis();

  JsonObject f = doc.createNestedObject("features");
  f["fsd"] = feat.fsd;
  f["profile"] = feat.profile;
  f["nag"] = feat.nag;
  f["speedOffset"] = feat.speedOffset;
  f["isaChime"] = feat.isaChime;
  f["summon"] = feat.summon;

  JsonObject hw = doc.createNestedObject("hardware");
  hw["board"] = BOARD_HW_NAME;
  hw["can"] = BOARD_CAN_NAME;
  hw["busFsd"] = (int)BUS_FSD_ACTIVE;
  hw["busVehicle"] = (int)BUS_VEHICLE_ACTIVE;
  hw["busBody"] = (int)BUS_BODY_ACTIVE;
#if BOARD_ENABLE_BLE
  hw["ble"] = true;
#else
  hw["ble"] = false;
#endif
  hw["wifi"] = true;
  hw["ip"] = (wifiCfg.mode == WIFI_MODE_STA) ? WiFi.localIP().toString() : WiFi.softAPIP().toString();

  String output;
  serializeJson(doc, output);
  return output;
}

// ── REST API Route Handlers ─────────────────────────────────────────────────

// Forward declare command executor (defined in serial/esp32.h)
void executeCommand(const char* cmd, State& s, unsigned long now);

static void handleCors() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
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
  doc["mode"] = (wifiCfg.mode == WIFI_MODE_STA) ? "sta" : "ap";

  if (wifiCfg.mode == WIFI_MODE_STA) {
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
    wifiCfg.mode = WIFI_MODE_STA;
    saveWifiConfig();

    // Try connecting to STA; fall back to AP if it fails
    if (!startSTA()) {
      wifiCfg.mode = WIFI_MODE_AP;
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
    wifiCfg.mode = WIFI_MODE_AP;
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

static void loadBleConfig() {
  blePrefs.begin(BLE_NVS_NS, true);
  bleEnabledCfg = blePrefs.getBool("enabled", true);
  blePrefs.end();
}

static void saveBleConfig() {
  blePrefs.begin(BLE_NVS_NS, false);
  blePrefs.putBool("enabled", bleEnabledCfg);
  blePrefs.end();
}

static String buildBleStatusJson() {
  StaticJsonDocument<128> doc;
  doc["enabled"] = bleIsReady();
  doc["connected"] = bleIsConnected();
  doc["deviceName"] = BLE_DEVICE_NAME;
  String out;
  serializeJson(doc, out);
  return out;
}

static void handleGetBleStatus() {
  sendJsonResponse(200, buildBleStatusJson());
}

static void handlePostBleConfig() {
  String body = server.arg("plain");
  if (body.length() == 0) {
    sendJsonResponse(400, "{\"error\":\"empty body\"}");
    return;
  }

  StaticJsonDocument<64> doc;
  DeserializationError err = deserializeJson(doc, body);
  if (err) {
    sendJsonResponse(400, "{\"error\":\"invalid json\"}");
    return;
  }

  if (!doc.containsKey("enabled")) {
    sendJsonResponse(400, "{\"error\":\"missing enabled field\"}");
    return;
  }

  bool en = doc["enabled"];
  bleEnabledCfg = en;
  saveBleConfig();

  if (en && !bleIsReady()) {
    bleRestart();
    sendLog("BLE enabled via REST");
  } else if (!en && bleIsReady()) {
    bleStop();
    sendLog("BLE disabled via REST");
  }

  sendJsonResponse(200, buildBleStatusJson());
}
#endif

// ── WiFi Init & Tick ────────────────────────────────────────────────────────

void wifiInit(State& s) {
  restState = &s;

  // Load saved WiFi configuration from NVS
  loadWifiConfig();

#if BOARD_ENABLE_BLE
  // Load saved BLE configuration from NVS
  loadBleConfig();
  if (!bleEnabledCfg && bleIsReady()) {
    bleStop();
    sendLog("BLE disabled (saved config)");
  }
#endif

  // Start WiFi in configured mode
  if (wifiCfg.mode == WIFI_MODE_STA && strlen(wifiCfg.staSSID) > 0) {
    if (!startSTA()) {
      wifiCfg.mode = WIFI_MODE_AP;
      startAP();
    }
  } else {
    startAP();
  }

  // Register routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/ping", HTTP_GET, handleGetPing);
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
