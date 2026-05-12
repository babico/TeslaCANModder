#pragma once

/**
 * @file firmware/lib/client/gamepad/ble.h
 * @brief NimBLE gamepad scan, connect, and HID notification plumbing
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "client/gamepad/events.h"
#include "client/gamepad/state.h"

#if BOARD_ENABLE_BLE

/**
 * @brief NimBLE notification callback for HID report characteristics
 * @param c Remote characteristic that sent the notification (unused)
 * @param d Pointer to the notification data buffer
 * @param l Length of the notification data in bytes
 * @param n Whether this is a notification vs indication (unused)
 */
static void gpNotifyCB(NimBLERemoteCharacteristic *c, uint8_t *d, size_t l, bool n)
{
	(void)c;
	(void)n;
	if (l > 0)
		gpDecodeReport(d, l);
}

/**
 * @brief BLE client callbacks for connection and disconnection events
 */
class GpClientCB : public NimBLEClientCallbacks
{
	void onConnect(NimBLEClient *) override
	{
		gpConnected = true;
		gpReconnFails = 0;
		gpAutoRescanArmed = false;
	}

	void onDisconnect(NimBLEClient *) override
	{
		gpConnected = false;
		gpButtons = 0;
		memset(gpAxes, 0, sizeof(gpAxes));
		memset(gpBtnDownMs, 0, sizeof(gpBtnDownMs));
		gpHoldFiredMask = 0;
		gpRssi = 0;
		gpBatteryPct = 0xFF;  // 0xFF indicates battery level unknown
	}
};
static GpClientCB gpClientCB;

/**
 * @brief BLE scan callbacks that filter for HID gamepad devices (UUID 0x1812)
 */
class GpScanCB : public NimBLEAdvertisedDeviceCallbacks
{
	void onResult(NimBLEAdvertisedDevice *adv) override
	{
		if (gpDeviceCount >= GP_MAX_SCAN)
			return;
		if (!adv->haveServiceUUID())
			return;
		if (!adv->isAdvertisingService(NimBLEUUID((uint16_t)0x1812)))  // HID service UUID
			return;
		std::string a = adv->getAddress().toString();
		// Skip duplicates already in the device list
		for (uint8_t i = 0; i < gpDeviceCount; i++)
			if (strncmp(gpDevices[i].addr, a.c_str(), 17) == 0)
				return;
		GpDevice &r = gpDevices[gpDeviceCount++];
		strncpy(r.addr, a.c_str(), 17);
		r.addr[17] = '\0';
		const char *nm = adv->haveName() ? adv->getName().c_str() : "Unknown";
		strncpy(r.name, nm, 32);
		r.name[32] = '\0';
	}
};
static GpScanCB gpScanCB;

/**
 * @brief Subscribe to all HID Report characteristics (UUID 0x2A4D) on the connected device
 * @return true if at least one report characteristic was subscribed
 */
static bool gpSubscribeReports()
{
	if (!gpClient)
		return false;
	NimBLERemoteService *svc = gpClient->getService(NimBLEUUID((uint16_t)0x1812));  // HID service
	if (!svc)
		return false;
	std::vector<NimBLERemoteCharacteristic *> *cs = svc->getCharacteristics(true);
	if (!cs || cs->empty())
		return false;
	bool ok = false;
	for (auto *c : *cs)
	{
		if (c->getUUID() == NimBLEUUID((uint16_t)0x2A4D) && c->canNotify())  // HID Report char
		{
			c->subscribe(true, gpNotifyCB, false);
			ok = true;
		}
	}
	return ok;
}

/**
 * @brief Read RSSI and battery level from the connected gamepad
 */
static void gpReadPeerInfo()
{
	if (!gpClient || !gpClient->isConnected())
		return;
	gpRssi = (int8_t)gpClient->getRssi();
	NimBLERemoteService *bs = gpClient->getService(NimBLEUUID((uint16_t)0x180F));  // Battery service
	if (!bs)
	{
		gpBatteryPct = 0xFF;
		return;
	}
	NimBLERemoteCharacteristic *bc = bs->getCharacteristic(NimBLEUUID((uint16_t)0x2A19));  // Battery Level
	if (!bc || !bc->canRead())
	{
		gpBatteryPct = 0xFF;
		return;
	}
	std::string v = bc->readValue();
	gpBatteryPct = v.empty() ? 0xFF : (uint8_t)v[0];
}

/**
 * @brief Attempt to connect to the paired gamepad address
 * @return true if connection and HID subscription succeeded
 */
static bool gpConnect()
{
	if (strlen(gpPairedAddr) < 17)
		return false;
	if (!gpClient)
	{
		gpClient = NimBLEDevice::createClient();
		gpClient->setClientCallbacks(&gpClientCB, false);
		gpClient->setConnectionParams(12, 12, 0, 51);  // 15ms interval, 0 latency, 510ms timeout
	}
	if (!gpClient->connect(NimBLEAddress(gpPairedAddr), true))
	{
		NimBLEDevice::deleteClient(gpClient);
		gpClient = nullptr;
		gpReconnFails++;
		return false;
	}
	if (!gpSubscribeReports())
	{
		gpClient->disconnect();
		gpReconnFails++;
		return false;
	}
	gpReadPeerInfo();
	gpReconnFails = 0;
	return true;
}

#endif // BOARD_ENABLE_BLE
