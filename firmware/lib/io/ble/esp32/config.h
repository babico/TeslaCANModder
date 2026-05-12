#pragma once

/**
 * @file firmware/lib/io/ble/esp32/config.h
 * @brief BLE persistent configuration stored in ESP32 NVS (Non-Volatile Storage)
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <Arduino.h>
#include <Preferences.h>
#include "core/config/esp32/board.h"

#if BOARD_ENABLE_BLE

/**
 * @brief Stop the BLE stack and de-initialise NimBLE
 */
void bleStop();

/**
 * @brief Restart the BLE stack if it was previously stopped
 */
void bleRestart();

/**
 * @brief Check whether the BLE stack is initialised and advertising
 * @return True if BLE is ready
 */
bool bleIsReady();

/**
 * @brief Check whether a BLE central is currently connected
 * @return True if a device is connected
 */
bool bleIsConnected();

/**
 * @brief Change the advertised BLE device name (triggers stack restart)
 * @param name Null-terminated name string (1-32 characters)
 * @return True if the name was accepted and applied
 */
bool bleSetDeviceName(const char *name);

/**
 * @brief Get the current BLE device name
 * @return Pointer to the internal name buffer
 */
const char *bleGetDeviceName();

static Preferences blePrefs;
#define BLE_NVS_NS "tcm_ble" // NVS namespace for BLE settings

static bool bleEnabledCfg = true;
static char bleNameCfg[33] = BLE_DEVICE_NAME;

/**
 * @brief Load BLE configuration (enabled flag and device name) from NVS
 */
static void loadBleConfig()
{
	blePrefs.begin(BLE_NVS_NS, true); // Read-only mode
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

/**
 * @brief Persist current BLE configuration (enabled flag and device name) to NVS
 */
static void saveBleConfig()
{
	blePrefs.begin(BLE_NVS_NS, false); // Read-write mode
	blePrefs.putBool("enabled", bleEnabledCfg);
	blePrefs.putString("name", bleNameCfg);
	blePrefs.end();
}
#endif // BOARD_ENABLE_BLE
