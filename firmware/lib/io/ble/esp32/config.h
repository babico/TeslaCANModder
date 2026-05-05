#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "core/config/esp32/board.h"

// ── BLE Persistent Configuration (NVS) ──────────────────────────────────────
//
// Owns the persisted BLE enabled flag and device name.
// Included by client/api/init.h (startup) and client/command/dispatch.h
// (wire-command handlers for ble:on, ble:off, ble:name:<n>).

#if BOARD_ENABLE_BLE
// Forward declarations — definitions live in io/ble/esp32/init.h.
void bleStop();
void bleRestart();
bool bleIsReady();
bool bleIsConnected();
bool bleSetDeviceName(const char *name);
const char *bleGetDeviceName();

static Preferences blePrefs;
#define BLE_NVS_NS "tcm_ble"

static bool bleEnabledCfg = true;
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
#endif // BOARD_ENABLE_BLE
