#pragma once
// WiFi-driver-level REST API handlers.
// These live in io/wifi/esp32/ because they directly manipulate wifiCfg
// (NVS persistence, AP/STA switching) — board-driver concerns.
// Included by io/wifi/esp32/board.h after config.h is processed.
#include "client/common/api_fwd.h"
#include "config.h"

// ── WiFi Status & Config Endpoints ──────────────────────────────────────────

static String buildWifiStatusJson()
{
	JsonDocument doc;
	doc["mode"] = (wifiCfg.mode == TCM_WIFI_MODE_STA) ? "sta" : "ap";

	if (wifiCfg.mode == TCM_WIFI_MODE_STA)
	{
		doc["ssid"] = wifiCfg.staSSID;
		doc["ip"] = WiFi.localIP().toString();
		doc["rssi"] = WiFi.RSSI();
		doc["connected"] = (WiFi.status() == WL_CONNECTED);
		doc["gateway"] = WiFi.gatewayIP().toString();
		doc["mac"] = WiFi.macAddress();
	}
	else
	{
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

static void handleGetWifiStatus()
{
	sendJsonResponse(200, buildWifiStatusJson());
}

static void handlePostWifiConfig()
{
	if (!requireAuth())
		return;
	String body = server.arg("plain");
	if (body.length() == 0)
	{
		sendJsonResponse(400, "{\"error\":\"empty body\"}");
		return;
	}

	JsonDocument doc;
	DeserializationError err = deserializeJson(doc, body);
	if (err)
	{
		sendJsonResponse(400, "{\"error\":\"invalid json\"}");
		return;
	}

	const char *mode = doc["mode"];
	const char *ssid = doc["ssid"];
	const char *password = doc["password"];

	if (!mode || (strcmp(mode, "ap") != 0 && strcmp(mode, "sta") != 0))
	{
		sendJsonResponse(400, "{\"error\":\"mode must be ap or sta\"}");
		return;
	}

	if (strcmp(mode, "sta") == 0)
	{
		if (!ssid || strlen(ssid) == 0 || strlen(ssid) > 32)
		{
			sendJsonResponse(400, "{\"error\":\"invalid sta ssid\"}");
			return;
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

		// Try connecting to STA; fall back to AP if it fails
		if (!startSTA())
		{
			wifiCfg.mode = TCM_WIFI_MODE_AP;
			saveWifiConfig();
			startAP();
		}
	}
	else
	{
		// AP mode
		if (ssid && strlen(ssid) > 0 && strlen(ssid) <= 32)
		{
			strncpy(wifiCfg.apSSID, ssid, 32);
			wifiCfg.apSSID[32] = '\0';
		}
		if (password)
		{
			if (strlen(password) == 0)
			{
				wifiCfg.apPassword[0] = '\0'; // Open AP
			}
			else if (strlen(password) >= 8 && strlen(password) <= 64)
			{
				strncpy(wifiCfg.apPassword, password, 64);
				wifiCfg.apPassword[64] = '\0';
			}
			else
			{
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
