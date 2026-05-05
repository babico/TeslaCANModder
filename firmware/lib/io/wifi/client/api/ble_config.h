#pragma once
#include "routes.h"

// ── BLE Status & Config Endpoints ───────────────────────────────────────────

#if BOARD_ENABLE_BLE
static Preferences blePrefs;
#define BLE_NVS_NS "tcm_ble"

static bool bleEnabledCfg = true; // default: enabled
static char bleNameCfg[33] = BLE_DEVICE_NAME;

static void loadBleConfig()
{
	blePrefs.begin(BLE_NVS_NS, true);
	bleEnabledCfg = blePrefs.getBool("enabled", true);
	String savedName = blePrefs.getString("name", BLE_DEVICE_NAME);
	if (savedName.length() == 0 || savedName.length() > 32)
	{
		strncpy(bleNameCfg, BLE_DEVICE_NAME, sizeof(bleNameCfg) - 1);
		bleNameCfg[sizeof(bleNameCfg) - 1] = '\0';
	}
	else
	{
		strncpy(bleNameCfg, savedName.c_str(), sizeof(bleNameCfg) - 1);
		bleNameCfg[sizeof(bleNameCfg) - 1] = '\0';
	}
	blePrefs.end();
}

static void saveBleConfig()
{
	blePrefs.begin(BLE_NVS_NS, false);
	blePrefs.putBool("enabled", bleEnabledCfg);
	blePrefs.putString("name", bleNameCfg);
	blePrefs.end();
}

static String buildBleStatusJson()
{
	JsonDocument doc;
	doc["enabled"] = bleIsReady();
	doc["connected"] = bleIsConnected();
	doc["deviceName"] = bleGetDeviceName();
	String out;
	serializeJson(doc, out);
	return out;
}

static void handleGetBleStatus()
{
	sendJsonResponse(200, buildBleStatusJson());
}

static void handlePostBleConfig()
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

	bool hasEnabled = doc.containsKey("enabled");
	bool hasName = doc.containsKey("name") || doc.containsKey("deviceName");

	if (!hasEnabled && !hasName)
	{
		sendJsonResponse(400, "{\"error\":\"missing enabled or name field\"}");
		return;
	}

	if (hasName)
	{
		const char *requestedName = doc.containsKey("name") ? doc["name"] : doc["deviceName"];
		size_t nameLen = requestedName ? strlen(requestedName) : 0;
		if (nameLen == 0 || nameLen > 32)
		{
			sendJsonResponse(400, "{\"error\":\"BLE name must be 1-32 characters\"}");
			return;
		}
		strncpy(bleNameCfg, requestedName, sizeof(bleNameCfg) - 1);
		bleNameCfg[sizeof(bleNameCfg) - 1] = '\0';
		if (!bleSetDeviceName(bleNameCfg))
		{
			sendJsonResponse(400, "{\"error\":\"failed to apply BLE name\"}");
			return;
		}
		sendLog("BLE name updated via REST");
	}

	if (hasEnabled)
	{
		bool en = doc["enabled"];
		bleEnabledCfg = en;

		if (en && !bleIsReady())
		{
			bleRestart();
			sendLog("BLE enabled via REST");
		}
		else if (!en && bleIsReady())
		{
			bleStop();
			sendLog("BLE disabled via REST");
		}
	}

	saveBleConfig();

	sendJsonResponse(200, buildBleStatusJson());
}
#endif
