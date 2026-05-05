#pragma once
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "core/config/esp32/board.h"
#include "core/types.h"
#include "core/log/ring.h"
#include "client/dashboard/dashboard.h"
#include "state.h"
#include "config.h"
#include "client/api/init.h"

// ── WiFi Driver Entry Points ─────────────────────────────────────────────────

inline void wifiInit(State &s)
{
	loadWifiConfig();

	if (wifiCfg.mode == TCM_WIFI_MODE_STA && strlen(wifiCfg.staSSID) > 0)
	{
		if (!startSTA())
		{
			wifiCfg.mode = TCM_WIFI_MODE_AP;
			startAP();
		}
	}
	else
	{
		startAP();
	}

	restApiInit(s);
}

inline void wifiTick()      { restApiTick(); }
inline bool wifiIsReady()   { return restApiIsReady(); }
