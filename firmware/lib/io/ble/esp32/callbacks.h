#pragma once
#include "state.h"

class BLEServerCB : public NimBLEServerCallbacks
{
	void onConnect(NimBLEServer *s) override
	{
		(void)s;
		bleDeviceConnected = true;
	}
	void onDisconnect(NimBLEServer *s) override
	{
		(void)s;
		bleDeviceConnected = false;
		NimBLEDevice::startAdvertising();
	}
};

class BLERxCallback : public NimBLECharacteristicCallbacks
{
	void onWrite(NimBLECharacteristic *pChar) override
	{
		std::string val = pChar->getValue();
		uint16_t head = bleRxHead.load(std::memory_order_relaxed);
		for (size_t i = 0; i < val.length(); i++)
		{
			uint16_t next = (head + 1) % sizeof(bleRxBuf);
			if (next != bleRxTail.load(std::memory_order_acquire))
			{
				bleRxBuf[head] = val[i];
				head = next;
			}
		}
		bleRxHead.store(head, std::memory_order_release);
	}
};
