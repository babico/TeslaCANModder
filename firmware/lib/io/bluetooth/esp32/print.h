#pragma once
#include "init.h"

void btPrint(const char *s)
{
	if (btReady)
		btSerial.print(s);
}

void btPrint(const __FlashStringHelper *s)
{
	if (btReady)
		btSerial.print(s);
}

void btPrintNum(long n)
{
	if (btReady)
		btSerial.print(n);
}

void btPrintHex(uint8_t b)
{
	if (btReady)
	{
		if (b < 0x10)
			btSerial.print("0");
		btSerial.print(b, HEX);
	}
}

void btPrintLn()
{
	if (btReady)
		btSerial.println();
}

int btAvailable()
{
	return btReady ? btSerial.available() : 0;
}

char btRead()
{
	return (char)btSerial.read();
}
