#pragma once

/**
 * @file firmware/lib/io/wifi/esp32/config.h
 * @brief WiFi configuration storage, NVS persistence, and AP/STA startup routines
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "state.h"

/**
 * @brief Runtime WiFi configuration stored in NVS.
 */
struct WifiConfig
{
	uint8_t mode;          // 0 = AP, 1 = STA
	char apSSID[33];       // Soft-AP SSID (max 32 chars + null)
	char apPassword[65];   // Soft-AP password (max 64 chars + null)
	char staSSID[33];      // Station-mode target SSID
	char staPassword[65];  // Station-mode password
};

static WifiConfig wifiCfg;
static Preferences wifiPrefs;

#define WIFI_NVS_NS "tcm_wifi"
#define TCM_WIFI_MODE_AP 0
#define TCM_WIFI_MODE_STA 1
static constexpr unsigned long WIFI_STA_TIMEOUT = 15000;  // Milliseconds to wait for STA connection before AP fallback
static constexpr uint32_t WIFI_CFG_MAGIC = 0xA1;          // NVS sentinel indicating a valid saved config

/**
 * @brief Load WiFi configuration from NVS, or apply defaults if no saved config exists.
 */
static void loadWifiConfig()
{
	wifiPrefs.begin(WIFI_NVS_NS, true);
	uint8_t magic = wifiPrefs.getUChar("magic", 0);
	if (magic != WIFI_CFG_MAGIC)
	{
		// No valid config in NVS — use compile-time defaults
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

/**
 * @brief Persist the current WiFi configuration to NVS.
 */
static void saveWifiConfig()
{
	wifiPrefs.begin(WIFI_NVS_NS, false);
	wifiPrefs.putUChar("magic", WIFI_CFG_MAGIC);
	wifiPrefs.putUChar("mode", wifiCfg.mode);
	wifiPrefs.putString("apSSID", wifiCfg.apSSID);
	wifiPrefs.putString("apPW", wifiCfg.apPassword);
	wifiPrefs.putString("staSSID", wifiCfg.staSSID);
	wifiPrefs.putString("staPW", wifiCfg.staPassword);
	wifiPrefs.end();
}

/**
 * @brief Start the WiFi radio in soft-AP mode using the current config.
 */
static void startAP()
{
	WiFi.mode(WIFI_AP);
	// Only set a password if it meets the WPA2 minimum of 8 characters
	WiFi.softAP(wifiCfg.apSSID, strlen(wifiCfg.apPassword) >= 8 ? wifiCfg.apPassword : NULL, WIFI_AP_CHANNEL, 0, 4);
	sendLog("WiFi AP started");
	char msg[64];
	snprintf(msg, sizeof(msg), "AP SSID: %s  IP: %s", wifiCfg.apSSID, WiFi.softAPIP().toString().c_str());
	sendLog(msg);
}

/**
 * @brief Attempt to connect to a WiFi network in station mode.
 * @return True if the connection succeeded within WIFI_STA_TIMEOUT, false otherwise.
 */
static bool startSTA()
{
	if (strlen(wifiCfg.staSSID) == 0)
		return false;
	WiFi.mode(WIFI_STA);
	WiFi.begin(wifiCfg.staSSID, strlen(wifiCfg.staPassword) > 0 ? wifiCfg.staPassword : NULL);
	sendLog("WiFi STA connecting...");
	char msg[64];
	snprintf(msg, sizeof(msg), "SSID: %s", wifiCfg.staSSID);
	sendLog(msg);

	unsigned long start = millis();
	while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_STA_TIMEOUT)
	{
		delay(250); // Poll connection status every 250 ms
	}
	if (WiFi.status() == WL_CONNECTED)
	{
		snprintf(msg, sizeof(msg), "Connected! IP: %s RSSI: %d", WiFi.localIP().toString().c_str(), WiFi.RSSI());
		sendLog(msg);
		return true;
	}
	sendLog("STA connection failed, falling back to AP");
	return false;
}
