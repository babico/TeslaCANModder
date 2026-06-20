#pragma once

/**
 * @file firmware/lib/io/ble/esp32/state.h
 * @brief Shared mutable state for the BLE peripheral (server, characteristics, ring buffer)
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <NimBLEDevice.h>
#include <atomic>
#include "core/config/esp32/board.h"
#include "vehicle/ble/distance.h"

static NimBLEServer *pServer = nullptr;              // NimBLE server instance
static NimBLECharacteristic *pTxChar = nullptr;      // TX (notify) characteristic handle
static NimBLECharacteristic *pRxChar = nullptr;      // RX (write) characteristic handle
static bool bleDeviceConnected = false;              // True while a central is connected
static bool bleReady = false;                        // True after bleInit() completes
static char bleDeviceName[33] = BLE_DEVICE_NAME;     // Advertised device name (max 32 chars + null)

static char bleRxBuf[256];                           // Lock-free ring buffer for incoming bytes
static std::atomic<uint16_t> bleRxHead{0};           // Producer index (written by RX callback)
static std::atomic<uint16_t> bleRxTail{0};           // Consumer index (read by bleRead)

/** @brief Persistent distance estimator (holds Kalman filter state) */
static BleDistanceEstimator bleDistanceEstimator;

/**
 * @brief Read RSSI from the first connected NimBLE central client.
 * @return RSSI in dBm, or 0 if no client is connected.
 *
 * Used by the BLE key distance estimator. If multiple clients are connected,
 * this returns the RSSI of the first one in NimBLE's client list.
 */
static inline int readBleKeyRssi()
{
#if BOARD_ENABLE_BLE
	if (!NimBLEDevice::getInitialized())
		return 0;
	std::list<NimBLEClient *> *clients = NimBLEDevice::getClientList();
	if (!clients)
		return 0;
	for (NimBLEClient *client : *clients)
	{
		if (client && client->isConnected())
			return client->getRssi();
	}
#endif
	return 0;
}

/**
 * @brief Poll peer RSSI and update the distance estimate in global state.
 * @param s Global state.
 *
 * Call from the main loop whenever BLE is enabled. The estimator updates its
 * internal Kalman state and writes s.bleRssi and s.bleDistanceMeters.
 */
static inline void bleDistanceTick(State &s)
{
#if BOARD_ENABLE_BLE
	int rssi = readBleKeyRssi();
	if (rssi == 0 && s.bleRssi == 0 && s.bleDistanceMode != BLE_DISTANCE_OFF)
	{
		// No connection yet; keep unknown sentinel until real samples arrive.
		s.bleDistanceMeters = -1.0f;
		return;
	}
	bleDistanceEstimator.update(rssi, s);
#else
	(void)s;
#endif
}
