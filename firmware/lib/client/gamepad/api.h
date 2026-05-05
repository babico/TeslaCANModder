#pragma once
// ── Public Gamepad Control API ───────────────────────────────────────────────

#include "client/gamepad/ble.h"
#include "client/gamepad/storage.h"

#if BOARD_ENABLE_BLE

static void gamepadInit()
{
	gpLoadNvs();
	gpLoadBindings();
}

static void gamepadStartScan()
{
	if (gpScanning)
		return;
	gpDeviceCount = 0;
	gpScan = NimBLEDevice::getScan();
	gpScan->setAdvertisedDeviceCallbacks(&gpScanCB, false);
	gpScan->setInterval(45);
	gpScan->setWindow(15);
	gpScan->setActiveScan(true);
	gpScan->start(6, nullptr, false);
	gpScanning = true;
}

static void gamepadStopScan()
{
	if (!gpScanning)
		return;
	if (gpScan)
		gpScan->stop();
	gpScanning = false;
}

static bool gamepadSetPaired(const char *addr)
{
	if (!addr || strlen(addr) < 17)
		return false;
	if (gpClient && gpClient->isConnected())
		gpClient->disconnect();
	strncpy(gpPairedAddr, addr, sizeof(gpPairedAddr) - 1);
	gpPairedAddr[sizeof(gpPairedAddr) - 1] = '\0';
	for (uint8_t i = 0; i < gpDeviceCount; i++)
	{
		if (strncmp(gpDevices[i].addr, addr, 17) == 0)
		{
			strncpy(gpLastSeenName, gpDevices[i].name, sizeof(gpLastSeenName) - 1);
			gpLastSeenName[sizeof(gpLastSeenName) - 1] = '\0';
			break;
		}
	}
	gpEnabled = true;
	gpReconnFails = 0;
	gpAutoRescanArmed = false;
	gpSaveNvs();
	return true;
}

static void gamepadUnpair()
{
	if (gpClient && gpClient->isConnected())
		gpClient->disconnect();
	gpPairedAddr[0] = '\0';
	gpEnabled = false;
	gpConnected = false;
	gpSaveNvs();
}

static void gamepadSetEnabled(bool en)
{
	gpEnabled = en;
	if (!en && gpClient && gpClient->isConnected())
		gpClient->disconnect();
	gpSaveNvs();
}

static void gamepadSetBinding(int idx, const char *cmd)
{
	if (idx < 0 || idx >= GAMEPAD_BTN_COUNT)
		return;
	strncpy(gpBinding[idx], cmd, GAMEPAD_CMD_MAXLEN - 1);
	gpBinding[idx][GAMEPAD_CMD_MAXLEN - 1] = '\0';
	gpSaveBinding(idx);
}

static void gamepadSetBindingHold(int idx, const char *cmd)
{
	if (idx < 0 || idx >= GAMEPAD_BTN_COUNT)
		return;
	strncpy(gpBindingHold[idx], cmd ? cmd : "", GAMEPAD_CMD_MAXLEN - 1);
	gpBindingHold[idx][GAMEPAD_CMD_MAXLEN - 1] = '\0';
	gpSaveBindingHold(idx);
}

static bool gamepadSetAxisTune(uint8_t idx, uint8_t dz, uint8_t expo, bool invert)
{
	if (idx >= 6)
		return false;
	if (dz > 50)
		dz = 50;
	if (expo > 100)
		expo = 100;
	gpAxisDz[idx] = dz;
	gpAxisExpo[idx] = expo;
	if (invert)
		gpAxisInvMask |= (uint8_t)(1u << idx);
	else
		gpAxisInvMask &= (uint8_t)~(1u << idx);
	gpSaveNvs();
	return true;
}

static void gamepadCancel()
{
	dasSendCancelBurst();
}

static inline int8_t gamepadGetRssi()
{
	return gpRssi;
}
static inline uint8_t gamepadGetBattery()
{
	return gpBatteryPct;
}
static inline uint8_t gamepadReconnectFails()
{
	return gpReconnFails;
}
static inline const char *gamepadLastSeenName()
{
	return gpLastSeenName;
}

static void gamepadTick(unsigned long now)
{
	if (gpScanning && gpScan && !gpScan->isScanning())
	{
		gpScanning = false;
		if (gpAutoRescanArmed && gpLastSeenName[0] != '\0')
		{
			gpAutoRescanArmed = false;
			for (uint8_t i = 0; i < gpDeviceCount; i++)
			{
				if (strcmp(gpDevices[i].name, gpLastSeenName) == 0)
				{
					gamepadSetPaired(gpDevices[i].addr);
					sendLog(F("Gamepad auto-rescan: re-paired by name"));
					break;
				}
			}
		}
	}
	if (!gpEnabled)
		return;

	if (gpConnected && gpButtons != 0)
	{
		for (int i = 0; i < GAMEPAD_BTN_COUNT; i++)
		{
			uint16_t mask = (uint16_t)(1u << i);
			if ((gpButtons & mask) && gpBtnDownMs[i] != 0 && !(gpHoldFiredMask & mask))
			{
				if ((now - gpBtnDownMs[i]) >= GP_HOLD_MS)
				{
					gpEvtPush((uint8_t)(i | GP_EVT_HOLD_FLAG));
					gpHoldFiredMask |= mask;
				}
			}
		}
	}

	if (gpConnected || gpScanning)
		return;
	if (strlen(gpPairedAddr) < 17)
		return;
	if (now - gpLastReconnMs >= GP_RECONNECT_MS)
	{
		gpLastReconnMs = now;
		gpConnect();
		if (gpReconnFails >= 3 && gpLastSeenName[0] != '\0' && !gpAutoRescanArmed)
		{
			sendLog(F("Gamepad reconnect failed 3x - auto-rescanning by name"));
			gpAutoRescanArmed = true;
			gpReconnFails = 0;
			gamepadStartScan();
		}
	}
}

static inline bool gamepadIsEnabled()
{
	return gpEnabled;
}
static inline bool gamepadIsConnected()
{
	return gpConnected;
}
static inline bool gamepadIsScanning()
{
	return gpScanning;
}

#endif // BOARD_ENABLE_BLE
