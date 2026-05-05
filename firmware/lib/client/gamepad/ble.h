#pragma once
// ── NimBLE Gamepad Scan, Connect, and Notification Plumbing ─────────────────

#include "client/gamepad/events.h"
#include "client/gamepad/state.h"

#if BOARD_ENABLE_BLE

static void gpNotifyCB(NimBLERemoteCharacteristic *c, uint8_t *d, size_t l, bool n)
{
	(void)c;
	(void)n;
	if (l > 0)
		gpDecodeReport(d, l);
}

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
		gpBatteryPct = 0xFF;
	}
};
static GpClientCB gpClientCB;

class GpScanCB : public NimBLEAdvertisedDeviceCallbacks
{
	void onResult(NimBLEAdvertisedDevice *adv) override
	{
		if (gpDeviceCount >= GP_MAX_SCAN)
			return;
		if (!adv->haveServiceUUID())
			return;
		if (!adv->isAdvertisingService(NimBLEUUID((uint16_t)0x1812)))
			return;
		std::string a = adv->getAddress().toString();
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

static bool gpSubscribeReports()
{
	if (!gpClient)
		return false;
	NimBLERemoteService *svc = gpClient->getService(NimBLEUUID((uint16_t)0x1812));
	if (!svc)
		return false;
	std::vector<NimBLERemoteCharacteristic *> *cs = svc->getCharacteristics(true);
	if (!cs || cs->empty())
		return false;
	bool ok = false;
	for (auto *c : *cs)
	{
		if (c->getUUID() == NimBLEUUID((uint16_t)0x2A4D) && c->canNotify())
		{
			c->subscribe(true, gpNotifyCB, false);
			ok = true;
		}
	}
	return ok;
}

static void gpReadPeerInfo()
{
	if (!gpClient || !gpClient->isConnected())
		return;
	gpRssi = (int8_t)gpClient->getRssi();
	NimBLERemoteService *bs = gpClient->getService(NimBLEUUID((uint16_t)0x180F));
	if (!bs)
	{
		gpBatteryPct = 0xFF;
		return;
	}
	NimBLERemoteCharacteristic *bc = bs->getCharacteristic(NimBLEUUID((uint16_t)0x2A19));
	if (!bc || !bc->canRead())
	{
		gpBatteryPct = 0xFF;
		return;
	}
	std::string v = bc->readValue();
	gpBatteryPct = v.empty() ? 0xFF : (uint8_t)v[0];
}

static bool gpConnect()
{
	if (strlen(gpPairedAddr) < 17)
		return false;
	if (!gpClient)
	{
		gpClient = NimBLEDevice::createClient();
		gpClient->setClientCallbacks(&gpClientCB, false);
		gpClient->setConnectionParams(12, 12, 0, 51);
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
