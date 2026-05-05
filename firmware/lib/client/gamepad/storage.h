#pragma once
// ── Gamepad NVS Persistence ──────────────────────────────────────────────────

#include "client/gamepad/state.h"

#if BOARD_ENABLE_BLE

static void gpLoadNvs()
{
	gpPrefs.begin(GP_NVS_NS, true);
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
		uint8_t def[6] = {6, 6, 6, 6, 8, 8};
		memcpy(gpAxisDz, def, sizeof(def));
	}
	got = gpPrefs.getBytes("axexpo", gpAxisExpo, sizeof(gpAxisExpo));
	if (got != sizeof(gpAxisExpo))
		memset(gpAxisExpo, 0, sizeof(gpAxisExpo));
	gpPrefs.end();
}

static void gpSaveNvs()
{
	gpPrefs.begin(GP_NVS_NS, false);
	gpPrefs.putBool("en", gpEnabled);
	gpPrefs.putString("addr", gpPairedAddr);
	gpPrefs.putString("name", gpLastSeenName);
	gpPrefs.putUChar("axinv", gpAxisInvMask);
	gpPrefs.putBytes("axdz", gpAxisDz, sizeof(gpAxisDz));
	gpPrefs.putBytes("axexpo", gpAxisExpo, sizeof(gpAxisExpo));
	gpPrefs.end();
}

static void gpLoadBindings()
{
	gpPrefs.begin(GP_BIND_NS, true);
	char k[5];
	for (int i = 0; i < GAMEPAD_BTN_COUNT; i++)
	{
		snprintf(k, sizeof(k), "b%d", i);
		if (gpPrefs.isKey(k))
		{
			String v = gpPrefs.getString(k, "");
			strncpy(gpBinding[i], v.c_str(), GAMEPAD_CMD_MAXLEN - 1);
			gpBinding[i][GAMEPAD_CMD_MAXLEN - 1] = '\0';
		}
		snprintf(k, sizeof(k), "h%d", i);
		if (gpPrefs.isKey(k))
		{
			String v = gpPrefs.getString(k, "");
			strncpy(gpBindingHold[i], v.c_str(), GAMEPAD_CMD_MAXLEN - 1);
			gpBindingHold[i][GAMEPAD_CMD_MAXLEN - 1] = '\0';
		}
	}
	gpPrefs.end();
}

static void gpSaveBinding(int idx)
{
	if (idx < 0 || idx >= GAMEPAD_BTN_COUNT)
		return;
	char k[5];
	snprintf(k, sizeof(k), "b%d", idx);
	gpPrefs.begin(GP_BIND_NS, false);
	gpPrefs.putString(k, gpBinding[idx]);
	gpPrefs.end();
}

static void gpSaveBindingHold(int idx)
{
	if (idx < 0 || idx >= GAMEPAD_BTN_COUNT)
		return;
	char k[5];
	snprintf(k, sizeof(k), "h%d", idx);
	gpPrefs.begin(GP_BIND_NS, false);
	gpPrefs.putString(k, gpBindingHold[idx]);
	gpPrefs.end();
}

#endif // BOARD_ENABLE_BLE
