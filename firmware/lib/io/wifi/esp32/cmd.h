#pragma once

/**
 * @file firmware/lib/io/wifi/esp32/cmd.h
 * @brief WiFi wire command handlers for wifi:status and wifi:config dispatched from the REST API
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "config.h"

/**
 * @brief Build a JSON string describing the current WiFi status.
 * @return Serialized JSON with mode, SSID, IP, and connection details.
 */
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

/**
 * @brief Execute a WiFi-related command received via the REST API.
 * @param cmd Command name string (e.g. "wifi:status", "wifi:config").
 * @param doc Parsed JSON document containing command parameters.
 * @return True if the command was recognized and handled, false otherwise.
 */
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
			wifiCfg.staSSID[32] = '\0'; // Ensure null termination after strncpy
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
				// STA failed — revert to AP mode so the device remains reachable
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
					wifiCfg.apPassword[0] = '\0'; // Clear password to create an open AP
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
		server.begin(); // Restart the HTTP server on the new network interface
		sendJsonResponse(200, buildWifiStatusJson());
		return true;
	}

	return false;
}
