#pragma once

/**
 * @file firmware/lib/io/serial/bluetooth/esp32/print.h
 * @brief Bluetooth Serial print and read utilities for ESP32
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "init.h"

/**
 * @brief Print a C-string over Bluetooth Serial
 * @param s Null-terminated string to transmit
 */
void btPrint(const char *s)
{
	if (btReady)
		btSerial.print(s);
}

/**
 * @brief Print a flash-stored string over Bluetooth Serial
 * @param s Flash string helper pointer to transmit
 */
void btPrint(const __FlashStringHelper *s)
{
	if (btReady)
		btSerial.print(s);
}

/**
 * @brief Print a numeric value over Bluetooth Serial
 * @param n Long integer to transmit as decimal text
 */
void btPrintNum(long n)
{
	if (btReady)
		btSerial.print(n);
}

/**
 * @brief Print a byte as two-digit uppercase hexadecimal over Bluetooth Serial
 * @param b Byte value to transmit
 */
void btPrintHex(uint8_t b)
{
	if (btReady)
	{
		if (b < 0x10)
			btSerial.print("0"); // Pad single-digit hex values with leading zero
		btSerial.print(b, HEX);
	}
}

/**
 * @brief Print a newline over Bluetooth Serial
 */
void btPrintLn()
{
	if (btReady)
		btSerial.println();
}

/**
 * @brief Check how many bytes are available to read from Bluetooth Serial
 * @return Number of bytes available, or 0 if Bluetooth is not ready
 */
int btAvailable()
{
	return btReady ? btSerial.available() : 0;
}

/**
 * @brief Read a single character from the Bluetooth Serial buffer
 * @return The next character from the receive buffer
 */
char btRead()
{
	return (char)btSerial.read();
}
