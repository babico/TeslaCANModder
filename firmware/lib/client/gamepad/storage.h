#pragma once

/**
 * @file firmware/lib/client/gamepad/storage.h
 * @brief NVS persistence for gamepad settings and button command bindings
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "client/gamepad/state.h"

#if BOARD_ENABLE_BLE

/**
 * @brief Load gamepad settings from NVS (enabled state, paired address, axis config).
 */
static void gpLoadNvs()
{
	gpPrefs.begin(GP_NVS_NS, true); // open read-only
	gpEnabled = gpPrefs.getBool("en", false);
	String a = gpPrefs.getString("addr", "");
	strncpy(gpPairedAddr, a.c_str(), sizeof(gpPairedAddr) - 1);
	gpPairedAddr[sizeof(gpPairedAddr) - 1] = '\0';
	String n = gpPrefs.getString("name", "");
	strncpy(gpLastSeenName, n.c_str(), sizeof(gpLastSeenName) - 1);
	gpLastSeenName[sizeof(gpLastSeenName) - 1] = '\0';
	gpAxisInvMask = gpPrefs.getUChar("axinv", 0);
	size_t got = gpPrefs.getBytes("axdz", gpAxisDz, sizeof(gpAxisDz));
	if (got != sizeof(gpAxisDz))
	{
		uint8_t def[6] = {6, 6, 6, 6, 8, 8}; // default deadzones: 6 for sticks, 8 for triggers
		memcpy(gpAxisDz, def, sizeof(def));
	}
	got = gpPrefs.getBytes("axexpo", gpAxisExpo, sizeof(gpAxisExpo));
	if (got != sizeof(gpAxisExpo))
		memset(gpAxisExpo, 0, sizeof(gpAxisExpo));
	gpPrefs.end();
}

/**
 * @brief Save current gamepad settings to NVS.
 */
static void gpSaveNvs()
{
	gpPrefs.begin(GP_NVS_NS, false); // open read-write
	gpPrefs.putBool("en", gpEnabled);
	gpPrefs.putString("addr", gpPairedAddr);
	gpPrefs.putString("name", gpLastSeenName);
	gpPrefs.putUChar("axinv", gpAxisInvMask);
	gpPrefs.putBytes("axdz", gpAxisDz, sizeof(gpAxisDz));
	gpPrefs.putBytes("axexpo", gpAxisExpo, sizeof(gpAxisExpo));
	gpPrefs.end();
}

/**
 * @brief Load all button command bindings (press and hold) from NVS.
 */
static void gpLoadBindings()
{
	gpPrefs.begin(GP_BIND_NS, true); // open read-only
	char k[5];
	for (int i = 0; i < GAMEPAD_BTN_COUNT; i++)
	{
		snprintf(k, sizeof(k), "b%d", i); // NVS key: "b0".."b15" for press bindings
		if (gpPrefs.isKey(k))
		{
			String v = gpPrefs.getString(k, "");
			strncpy(gpBinding[i], v.c_str(), GAMEPAD_CMD_MAXLEN - 1);
			gpBinding[i][GAMEPAD_CMD_MAXLEN - 1] = '\0';
		}
		snprintf(k, sizeof(k), "h%d", i); // NVS key: "h0".."h15" for hold bindings
		if (gpPrefs.isKey(k))
		{
			String v = gpPrefs.getString(k, "");
			strncpy(gpBindingHold[i], v.c_str(), GAMEPAD_CMD_MAXLEN - 1);
			gpBindingHold[i][GAMEPAD_CMD_MAXLEN - 1] = '\0';
		}
	}
	gpPrefs.end();
}

/**
 * @brief Save a single button press binding to NVS.
 * @param idx Button index (0-15).
 */
static void gpSaveBinding(int idx)
{
	if (idx < 0 || idx >= GAMEPAD_BTN_COUNT)
		return;
	char k[5];
	snprintf(k, sizeof(k), "b%d", idx);
	gpPrefs.begin(GP_BIND_NS, false); // open read-write
	gpPrefs.putString(k, gpBinding[idx]);
	gpPrefs.end();
}

/**
 * @brief Save a single button hold binding to NVS.
 * @param idx Button index (0-15).
 */
static void gpSaveBindingHold(int idx)
{
	if (idx < 0 || idx >= GAMEPAD_BTN_COUNT)
		return;
	char k[5];
	snprintf(k, sizeof(k), "h%d", idx);
	gpPrefs.begin(GP_BIND_NS, false); // open read-write
	gpPrefs.putString(k, gpBindingHold[idx]);
	gpPrefs.end();
}

#endif // BOARD_ENABLE_BLE
