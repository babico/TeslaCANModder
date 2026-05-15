#pragma once

/**
 * @file firmware/lib/interface/serial/output.h
 * @brief Low-level serial output primitives that bridge USB and BLE transports
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "state.h"

#if BOARD_ENABLE_BLE
void blePrint(const char *s);
void blePrint(const __FlashStringHelper *s);
void blePrintNum(long n);
void blePrintHex(uint8_t b);
void blePrintLn();
#else
static void blePrint(const char *) {}
static void blePrint(const __FlashStringHelper *) {}
static void blePrintNum(long) {}
static void blePrintHex(uint8_t) {}
static void blePrintLn() {}
#endif

/**
 * @brief Print a null-terminated C string to USB serial and optionally BLE
 * @param s Null-terminated string to transmit
 */
void printStr(const char *s)
{
	Serial.print(s);
	blePrint(s);
}

/**
 * @brief Print a flash-stored string to USB serial and optionally BLE
 * @param s Flash string pointer (F() macro result)
 */
void printStr(const __FlashStringHelper *s)
{
	Serial.print(s);
	blePrint(s);
}

/**
 * @brief Print a numeric value as decimal text to USB serial and optionally BLE
 * @param n Integer value to print
 */
void printNum(long n)
{
	Serial.print(n);
	blePrintNum(n);
}

/**
 * @brief Print a byte as two-character uppercase hex to USB serial and optionally BLE
 * @param b Byte value to print
 */
void printHex(uint8_t b)
{
	if (b < 0x10)
		printStr("0"); // Pad single-digit hex with leading zero
	Serial.print(b, HEX);
	blePrintHex(b);
}

/**
 * @brief Print a newline (CRLF) to USB serial and optionally BLE
 */
void printLn()
{
	Serial.println();
	blePrintLn();
}

#include "interface/common/json.h"
