#pragma once

/**
 * @file firmware/lib/io/ble/esp32/print.h
 * @brief BLE output and input functions for the NUS TX/RX characteristics
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "state.h"

/**
 * @brief Send a null-terminated string over BLE via the TX characteristic
 * @param s Null-terminated C string to transmit
 */
void blePrint(const char *s)
{
	if (bleReady && bleDeviceConnected && pTxChar)
	{
		pTxChar->setValue((const uint8_t *)s, strlen(s));
		pTxChar->notify();
	}
}

/**
 * @brief Send a flash-stored string over BLE via the TX characteristic
 * @param s Flash string helper pointer (PROGMEM on AVR, RAM on ESP32)
 */
void blePrint(const __FlashStringHelper *s)
{
	if (bleReady && bleDeviceConnected && pTxChar)
	{
		String str(s);
		pTxChar->setValue((const uint8_t *)str.c_str(), str.length());
		pTxChar->notify();
	}
}

/**
 * @brief Send a signed integer as its decimal string representation over BLE
 * @param n Integer value to transmit
 */
void blePrintNum(long n)
{
	char buf[12];
	snprintf(buf, sizeof(buf), "%ld", n);
	blePrint(buf);
}

/**
 * @brief Send a single byte as a two-digit uppercase hex string over BLE
 * @param b Byte value to transmit
 */
void blePrintHex(uint8_t b)
{
	char buf[4];
	snprintf(buf, sizeof(buf), "%02X", b);
	blePrint(buf);
}

/**
 * @brief Send a CRLF line ending over BLE
 */
void blePrintLn()
{
	blePrint("\r\n");
}

/**
 * @brief Return the number of unread bytes in the BLE receive ring buffer
 * @return Byte count available for reading, or 0 if BLE is not ready
 */
int bleAvailable()
{
	if (!bleReady)
		return 0;
	uint16_t head = bleRxHead.load(std::memory_order_acquire);
	uint16_t tail = bleRxTail.load(std::memory_order_relaxed);
	return (head - tail + sizeof(bleRxBuf)) % sizeof(bleRxBuf);
}

/**
 * @brief Read and consume one byte from the BLE receive ring buffer
 * @return The next byte, or 0 if the buffer is empty
 */
char bleRead()
{
	uint16_t head = bleRxHead.load(std::memory_order_acquire);
	uint16_t tail = bleRxTail.load(std::memory_order_relaxed);
	if (head == tail)
		return 0;
	char c = bleRxBuf[tail];
	bleRxTail.store((tail + 1) % sizeof(bleRxBuf), std::memory_order_release);
	return c;
}
