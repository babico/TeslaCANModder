#pragma once

/**
 * @file firmware/lib/io/ble/esp32/callbacks.h
 * @brief NimBLE server and characteristic callback implementations for BLE peripheral
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "state.h"

/**
 * @brief Server-level callbacks for BLE connection lifecycle events
 */
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
		NimBLEDevice::startAdvertising(); // Resume advertising so another central can connect
	}
};

/**
 * @brief RX characteristic callback that enqueues incoming bytes into the ring buffer
 */
class BLERxCallback : public NimBLECharacteristicCallbacks
{
	void onWrite(NimBLECharacteristic *pChar) override
	{
		std::string val = pChar->getValue();
		uint16_t head = bleRxHead.load(std::memory_order_relaxed);
		for (size_t i = 0; i < val.length(); i++)
		{
			uint16_t next = (head + 1) % sizeof(bleRxBuf);
			// Drop byte if ring buffer is full (tail would be overwritten)
			if (next != bleRxTail.load(std::memory_order_acquire))
			{
				bleRxBuf[head] = val[i];
				head = next;
			}
		}
		bleRxHead.store(head, std::memory_order_release); // Publish all enqueued bytes atomically
	}
};
