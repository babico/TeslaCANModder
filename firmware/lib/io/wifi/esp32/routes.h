#pragma once
#include "auth.h"
#include "infra/can_recorder.h"

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
  doc["apGateEnabled"] = s.apInjectionGateEnabled;
  doc["apGateAp"] = s.apGateApActive;
  doc["apGatePark"] = s.apGateParked;
  doc["apGateSummon"] = s.apGateSummoning;
  doc["apGateOpen"] = s.apGateOpen();
  doc["enhancedAutopilot"] = s.enhancedAutopilot;
  doc["canRecorderEnabled"] = canRecorderEnabled();
  doc["canRecorderCount"] = canRecorderCount();
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
    pt["brake"] = s.brakePedalState;
    pt["steer"] = (int)(s.steeringAngle * 10);
    pt["rpmR"] = s.rearMotorRpm;
    pt["rpmF"] = s.frontMotorRpm;
  }
  if (s.hasWheelSpeeds) {
    JsonObject ws = doc.createNestedObject("wheelSpeeds");
    ws["fl"] = (int)(s.wheelSpeedFL * 100);
    ws["fr"] = (int)(s.wheelSpeedFR * 100);
    ws["rl"] = (int)(s.wheelSpeedRL * 100);
    ws["rr"] = (int)(s.wheelSpeedRR * 100);
  }
  if (s.hasMotorTemps) {
    JsonObject mt = doc.createNestedObject("motorTemps");
    mt["rInv"] = s.rearInvTemp;
    mt["rStat"] = s.rearStatorTemp;
    mt["rHs"] = s.rearHeatsinkTemp;
    mt["fInv"] = s.frontInvTemp;
    mt["fStat"] = s.frontStatorTemp;
    mt["fHs"] = s.frontHeatsinkTemp;
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

// Forward declare command executor (defined in serial/board.h)
void executeCommand(const char* cmd, State& s, unsigned long now);

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
