#pragma once

/**
 * @file firmware/lib/interface/ble/init.h
 * @brief BLE peripheral initialisation, shutdown, and device name management
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "uuids.h"
#include "state.h"
#include "callbacks.h"

/**
 * @brief Stop the BLE stack, disconnect any client, and de-initialise NimBLE
 */
void bleStop()
{
	if (!bleReady)
		return;
	NimBLEDevice::stopAdvertising();
	if (pServer)
	{
		pServer->disconnect(0);
	}
	NimBLEDevice::deinit(true); // true = release all BLE resources
	bleReady = false;
	bleDeviceConnected = false;
	pServer = nullptr;
	pTxChar = nullptr;
	pRxChar = nullptr;
}

/**
 * @brief Copy a device name into the internal buffer with null-termination
 * @param name Null-terminated name to store (max 32 characters)
 */
static void setBleDeviceNameValue(const char *name)
{
	if (!name)
		return;
	strncpy(bleDeviceName, name, sizeof(bleDeviceName) - 1);
	bleDeviceName[sizeof(bleDeviceName) - 1] = '\0';
}

/**
 * @brief Initialise the NimBLE stack, create GATT service, and start advertising
 */
void bleInit()
{
	NimBLEDevice::init(bleDeviceName);
	NimBLEDevice::setPower(ESP_PWR_LVL_P9); // +9 dBm transmit power

	// Enable bonding, MITM protection, and secure connections
	NimBLEDevice::setSecurityAuth(true, true, true);
	NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT); // "Just Works" pairing

	pServer = NimBLEDevice::createServer();
	pServer->setCallbacks(new BLEServerCB());

	NimBLEService *pService = pServer->createService(BLE_SERVICE_UUID);

	// TX characteristic: server notifies client with outgoing data
	pTxChar = pService->createCharacteristic(BLE_TX_UUID, NIMBLE_PROPERTY::NOTIFY);

	// RX characteristic: client writes incoming data (encrypted writes supported)
	pRxChar = pService->createCharacteristic(
		BLE_RX_UUID, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR | NIMBLE_PROPERTY::WRITE_ENC);
	pRxChar->setCallbacks(new BLERxCallback());

	pService->start();

	NimBLEAdvertising *pAdv = NimBLEDevice::getAdvertising();
	pAdv->addServiceUUID(BLE_SERVICE_UUID);
	pAdv->setScanResponse(true);
	pAdv->start();

	bleReady = true;
}

/**
 * @brief Restart the BLE stack if it was previously stopped
 */
void bleRestart()
{
	if (bleReady)
		return;
	bleInit();
}

/**
 * @brief Check whether the BLE stack is initialised and advertising
 * @return True if BLE is ready
 */
bool bleIsReady() { return bleReady; }

/**
 * @brief Check whether a BLE central is currently connected
 * @return True if a device is connected
 */
bool bleIsConnected() { return bleDeviceConnected; }

/**
 * @brief Get the current BLE device name
 * @return Pointer to the internal name buffer
 */
const char *bleGetDeviceName() { return bleDeviceName; }

/**
 * @brief Change the advertised BLE device name, restarting the stack if needed
 * @param name Null-terminated name string (1-32 characters)
 * @return True if the name was accepted and applied
 */
bool bleSetDeviceName(const char *name)
{
	if (!name)
		return false;
	size_t len = strlen(name);
	if (len == 0 || len > 32)
		return false;
	if (strcmp(bleDeviceName, name) == 0)
		return true; // Already set, no restart needed

	bool wasReady = bleReady;
	if (wasReady)
		bleStop();

	setBleDeviceNameValue(name);

	if (wasReady)
		bleInit();

	return true;
}
