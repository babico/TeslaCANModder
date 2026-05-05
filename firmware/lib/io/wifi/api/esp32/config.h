#pragma once
#include "state.h"

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
