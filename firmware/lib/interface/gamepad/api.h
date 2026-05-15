#pragma once

/**
 * @file firmware/lib/interface/gamepad/api.h
 * @brief Public gamepad control API for scanning, pairing, binding, and tick-driven reconnection
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "interface/gamepad/ble.h"
#include "interface/gamepad/storage.h"

#if BOARD_ENABLE_BLE

/**
 * @brief Initialize the gamepad subsystem by loading NVS settings and button bindings
 */
static void gamepadInit()
{
	gpLoadNvs();
	gpLoadBindings();
}

/**
 * @brief Start a BLE scan for HID gamepad devices
 */
static void gamepadStartScan()
{
	if (gpScanning)
		return;
	gpDeviceCount = 0;
	gpScan = NimBLEDevice::getScan();
	gpScan->setAdvertisedDeviceCallbacks(&gpScanCB, false);
	gpScan->setInterval(45);  // Scan interval in 0.625ms units
	gpScan->setWindow(15);    // Scan window in 0.625ms units
	gpScan->setActiveScan(true);
	gpScan->start(6, nullptr, false);  // Scan for 6 seconds
	gpScanning = true;
}

/**
 * @brief Stop an in-progress BLE scan
 */
static void gamepadStopScan()
{
	if (!gpScanning)
		return;
	if (gpScan)
		gpScan->stop();
	gpScanning = false;
}

/**
 * @brief Pair with a gamepad at the given BLE address
 * @param addr BLE MAC address string (17 chars, e.g. "AA:BB:CC:DD:EE:FF")
 * @return true if pairing was accepted, false if address is invalid
 */
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

/**
 * @brief Unpair the currently paired gamepad and disconnect if connected
 */
static void gamepadUnpair()
{
	if (gpClient && gpClient->isConnected())
		gpClient->disconnect();
	gpPairedAddr[0] = '\0';
	gpEnabled = false;
	gpConnected = false;
	gpSaveNvs();
}

/**
 * @brief Enable or disable the gamepad subsystem
 * @param en true to enable, false to disable (disconnects if currently connected)
 */
static void gamepadSetEnabled(bool en)
{
	gpEnabled = en;
	if (!en && gpClient && gpClient->isConnected())
		gpClient->disconnect();
	gpSaveNvs();
}

/**
 * @brief Assign a command string to a button press binding
 * @param idx Button index (0 to GAMEPAD_BTN_COUNT-1)
 * @param cmd Null-terminated command string to bind
 */
static void gamepadSetBinding(int idx, const char *cmd)
{
	if (idx < 0 || idx >= GAMEPAD_BTN_COUNT)
		return;
	strncpy(gpBinding[idx], cmd, GAMEPAD_CMD_MAXLEN - 1);
	gpBinding[idx][GAMEPAD_CMD_MAXLEN - 1] = '\0';
	gpSaveBinding(idx);
}

/**
 * @brief Assign a command string to a button long-hold binding
 * @param idx Button index (0 to GAMEPAD_BTN_COUNT-1)
 * @param cmd Null-terminated command string to bind (nullptr clears the binding)
 */
static void gamepadSetBindingHold(int idx, const char *cmd)
{
	if (idx < 0 || idx >= GAMEPAD_BTN_COUNT)
		return;
	strncpy(gpBindingHold[idx], cmd ? cmd : "", GAMEPAD_CMD_MAXLEN - 1);
	gpBindingHold[idx][GAMEPAD_CMD_MAXLEN - 1] = '\0';
	gpSaveBindingHold(idx);
}

/**
 * @brief Configure axis tuning parameters (deadzone, expo curve, inversion)
 * @param idx Axis index (0-5: LX, LY, RX, RY, LT, RT)
 * @param dz Deadzone percentage (0-50, clamped)
 * @param expo Expo curve strength (0-100, clamped)
 * @param invert true to invert the axis output
 * @return true on success, false if idx is out of range
 */
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

/**
 * @brief Cancel any active DAS drive burst command
 */
static void gamepadCancel()
{
	dasSendCancelBurst();
}

/**
 * @brief Get the current BLE RSSI of the connected gamepad
 * @return RSSI in dBm, or 0 if not connected
 */
static inline int8_t gamepadGetRssi()
{
	return gpRssi;
}

/**
 * @brief Get the battery level of the connected gamepad
 * @return Battery percentage (0-100), or 0xFF if unavailable
 */
static inline uint8_t gamepadGetBattery()
{
	return gpBatteryPct;
}

/**
 * @brief Get the number of consecutive reconnection failures
 * @return Failure count since last successful connection
 */
static inline uint8_t gamepadReconnectFails()
{
	return gpReconnFails;
}

/**
 * @brief Get the display name of the last seen paired device
 * @return Null-terminated name string
 */
static inline const char *gamepadLastSeenName()
{
	return gpLastSeenName;
}

/**
 * @brief Main gamepad tick — handles scan completion, hold detection, and auto-reconnect
 * @param now Current timestamp in milliseconds (from millis())
 */
static void gamepadTick(unsigned long now)
{
	// Handle scan completion and auto-rescan by name if reconnect failed
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

	// Detect long-hold events on pressed buttons
	if (gpConnected && gpButtons != 0)
	{
		for (int i = 0; i < GAMEPAD_BTN_COUNT; i++)
		{
			uint16_t mask = (uint16_t)(1u << i);
			if ((gpButtons & mask) && gpBtnDownMs[i] != 0 && !(gpHoldFiredMask & mask))
			{
				if ((now - gpBtnDownMs[i]) >= GP_HOLD_MS)
				{
					gpEvtPush((uint8_t)(i | GP_EVT_HOLD_FLAG));  // Push hold event with flag bit
					gpHoldFiredMask |= mask;
				}
			}
		}
	}

	// Auto-reconnect logic when disconnected with a paired address
	if (gpConnected || gpScanning)
		return;
	if (strlen(gpPairedAddr) < 17)
		return;
	if (now - gpLastReconnMs >= GP_RECONNECT_MS)
	{
		gpLastReconnMs = now;
		gpConnect();
		// After 3 consecutive failures, trigger auto-rescan by device name
		if (gpReconnFails >= 3 && gpLastSeenName[0] != '\0' && !gpAutoRescanArmed)
		{
			sendLog(F("Gamepad reconnect failed 3x - auto-rescanning by name"));
			gpAutoRescanArmed = true;
			gpReconnFails = 0;
			gamepadStartScan();
		}
	}
}

/**
 * @brief Check if the gamepad subsystem is enabled
 * @return true if enabled
 */
static inline bool gamepadIsEnabled()
{
	return gpEnabled;
}

/**
 * @brief Check if a gamepad is currently connected
 * @return true if connected
 */
static inline bool gamepadIsConnected()
{
	return gpConnected;
}

/**
 * @brief Check if a BLE scan is currently in progress
 * @return true if scanning
 */
static inline bool gamepadIsScanning()
{
	return gpScanning;
}

#endif // BOARD_ENABLE_BLE
