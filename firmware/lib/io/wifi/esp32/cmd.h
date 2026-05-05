#pragma once
#include "config.h"

// ── WiFi Wire Commands ────────────────────────────────────────────────────────
//
// Handles wifi:status and wifi:config dispatched from POST /api/command.
// Returns true if the command was consumed (response already sent).
// sendJsonResponse and server are declared in io/wifi/esp32/state.h (same TU).

static String buildWifiStatusJson()
{
	JsonDocument doc;
	doc["mode"] = (wifiCfg.mode == TCM_WIFI_MODE_STA) ? "sta" : "ap";

	if (wifiCfg.mode == TCM_WIFI_MODE_STA)
	{
		doc["ssid"]      = wifiCfg.staSSID;
		doc["ip"]        = WiFi.localIP().toString();
		doc["rssi"]      = WiFi.RSSI();
		doc["connected"] = (WiFi.status() == WL_CONNECTED);
		doc["gateway"]   = WiFi.gatewayIP().toString();
		doc["mac"]       = WiFi.macAddress();
	}
	else
	{
		doc["ssid"]    = wifiCfg.apSSID;
		doc["ip"]      = WiFi.softAPIP().toString();
		doc["clients"] = WiFi.softAPgetStationNum();
		doc["channel"] = WIFI_AP_CHANNEL;
		doc["mac"]     = WiFi.softAPmacAddress();
	}

	String out;
	serializeJson(doc, out);
	return out;
}

static bool executeWifiCmd(const char *cmd, const JsonDocument &doc)
{
	if (strcmp(cmd, "wifi:status") == 0)
	{
		sendJsonResponse(200, buildWifiStatusJson());
		return true;
	}

	if (strcmp(cmd, "wifi:config") == 0)
	{
		const char *mode     = doc["mode"]     | (const char *)nullptr;
		const char *ssid     = doc["ssid"]     | (const char *)nullptr;
		const char *password = doc["password"] | (const char *)nullptr;

		if (!mode || (strcmp(mode, "ap") != 0 && strcmp(mode, "sta") != 0))
		{
			sendJsonResponse(400, "{\"error\":\"mode must be ap or sta\"}");
			return true;
		}

		if (strcmp(mode, "sta") == 0)
		{
			if (!ssid || strlen(ssid) == 0 || strlen(ssid) > 32)
			{
				sendJsonResponse(400, "{\"error\":\"invalid sta ssid\"}");
				return true;
			}
			strncpy(wifiCfg.staSSID, ssid, 32);
			wifiCfg.staSSID[32] = '\0';
			if (password && strlen(password) > 0)
			{
				strncpy(wifiCfg.staPassword, password, 64);
				wifiCfg.staPassword[64] = '\0';
			}
			else
			{
				wifiCfg.staPassword[0] = '\0';
			}
			wifiCfg.mode = TCM_WIFI_MODE_STA;
			saveWifiConfig();
			if (!startSTA())
			{
				wifiCfg.mode = TCM_WIFI_MODE_AP;
				saveWifiConfig();
				startAP();
			}
		}
		else
		{
			if (ssid && strlen(ssid) > 0 && strlen(ssid) <= 32)
			{
				strncpy(wifiCfg.apSSID, ssid, 32);
				wifiCfg.apSSID[32] = '\0';
			}
			if (password)
			{
				if (strlen(password) == 0)
				{
					wifiCfg.apPassword[0] = '\0';
				}
				else if (strlen(password) >= 8 && strlen(password) <= 64)
				{
					strncpy(wifiCfg.apPassword, password, 64);
					wifiCfg.apPassword[64] = '\0';
				}
				else
				{
					sendJsonResponse(400, "{\"error\":\"AP password must be 8-64 chars or empty\"}");
					return true;
				}
			}
			wifiCfg.mode = TCM_WIFI_MODE_AP;
			saveWifiConfig();
			startAP();
		}
		server.begin();
		sendJsonResponse(200, buildWifiStatusJson());
		return true;
	}

	return false;
}
