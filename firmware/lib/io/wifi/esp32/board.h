#pragma once

/**
 * @file firmware/lib/io/wifi/esp32/board.h
 * @brief WiFi transport driver entry points: initialization, tick, and readiness check
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

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

/**
 * @brief Initialize the WiFi transport and REST API server.
 * @param s Reference to the shared firmware state.
 *
 * Loads saved WiFi config from NVS, attempts STA connection if configured,
 * and falls back to AP mode on failure. Starts the REST API once the network is up.
 */
inline void wifiInit(State &s)
{
	loadWifiConfig();

	if (wifiCfg.mode == TCM_WIFI_MODE_STA && strlen(wifiCfg.staSSID) > 0)
	{
		if (!startSTA())
		{
			wifiCfg.mode = TCM_WIFI_MODE_AP; // STA failed — fall back to AP
			startAP();
		}
	}
	else
	{
		startAP();
	}

	restApiInit(s);
}

/**
 * @brief Process pending WiFi/HTTP work each main-loop iteration.
 */
inline void wifiTick()      { restApiTick(); }

/**
 * @brief Check whether the WiFi transport is fully initialized and serving.
 * @return True if the REST API is ready to handle requests.
 */
inline bool wifiIsReady()   { return restApiIsReady(); }
