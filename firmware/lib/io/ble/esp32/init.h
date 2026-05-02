#pragma once
#include "uuids.h"
#include "state.h"
#include "callbacks.h"

void bleStop()
{
	if (!bleReady)
		return;
	NimBLEDevice::stopAdvertising();
	if (pServer)
	{
		pServer->disconnect(0);
	}
	NimBLEDevice::deinit(true);
	bleReady = false;
	bleDeviceConnected = false;
	pServer = nullptr;
	pTxChar = nullptr;
	pRxChar = nullptr;
}

static void setBleDeviceNameValue(const char *name)
{
	if (!name)
		return;
	strncpy(bleDeviceName, name, sizeof(bleDeviceName) - 1);
	bleDeviceName[sizeof(bleDeviceName) - 1] = '\0';
}

void bleInit()
{
	NimBLEDevice::init(bleDeviceName);
	NimBLEDevice::setPower(ESP_PWR_LVL_P9);

	NimBLEDevice::setSecurityAuth(true, true, true);
	NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);

	pServer = NimBLEDevice::createServer();
	pServer->setCallbacks(new BLEServerCB());

	NimBLEService *pService = pServer->createService(BLE_SERVICE_UUID);

	pTxChar = pService->createCharacteristic(BLE_TX_UUID, NIMBLE_PROPERTY::NOTIFY);

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

void bleRestart()
{
	if (bleReady)
		return;
	bleInit();
}

bool bleIsReady() { return bleReady; }
bool bleIsConnected() { return bleDeviceConnected; }
const char *bleGetDeviceName() { return bleDeviceName; }

bool bleSetDeviceName(const char *name)
{
	if (!name)
		return false;
	size_t len = strlen(name);
	if (len == 0 || len > 32)
		return false;
	if (strcmp(bleDeviceName, name) == 0)
		return true;

	bool wasReady = bleReady;
	if (wasReady)
		bleStop();

	setBleDeviceNameValue(name);

	if (wasReady)
		bleInit();

	return true;
}
