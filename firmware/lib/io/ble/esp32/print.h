#pragma once
#include "state.h"

void blePrint(const char *s)
{
	if (bleReady && bleDeviceConnected && pTxChar)
	{
		pTxChar->setValue((const uint8_t *)s, strlen(s));
		pTxChar->notify();
	}
}

void blePrint(const __FlashStringHelper *s)
{
	if (bleReady && bleDeviceConnected && pTxChar)
	{
		String str(s);
		pTxChar->setValue((const uint8_t *)str.c_str(), str.length());
		pTxChar->notify();
	}
}

void blePrintNum(long n)
{
	char buf[12];
	snprintf(buf, sizeof(buf), "%ld", n);
	blePrint(buf);
}

void blePrintHex(uint8_t b)
{
	char buf[4];
	snprintf(buf, sizeof(buf), "%02X", b);
	blePrint(buf);
}

void blePrintLn()
{
	blePrint("\r\n");
}

int bleAvailable()
{
	if (!bleReady)
		return 0;
	uint16_t head = bleRxHead.load(std::memory_order_acquire);
	uint16_t tail = bleRxTail.load(std::memory_order_relaxed);
	return (head - tail + sizeof(bleRxBuf)) % sizeof(bleRxBuf);
}

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
