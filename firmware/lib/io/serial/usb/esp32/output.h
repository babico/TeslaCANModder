#pragma once

/**
 * @file firmware/lib/io/serial/usb/esp32/output.h
 * @brief Low-level serial output primitives that bridge USB and BLE transports
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "state.h"

/**
 * @brief Print a null-terminated C string to USB serial and optionally BLE
 * @param s Null-terminated string to transmit
 */
void printStr(const char* s) {
	Serial.print(s);
#if BOARD_ENABLE_BLE
	blePrint(s);
#endif
}

/**
 * @brief Print a flash-stored string to USB serial and optionally BLE
 * @param s Flash string pointer (F() macro result)
 */
void printStr(const __FlashStringHelper* s) {
	Serial.print(s);
#if BOARD_ENABLE_BLE
	blePrint(s);
#endif
}

/**
 * @brief Print a numeric value as decimal text to USB serial and optionally BLE
 * @param n Integer value to print
 */
void printNum(long n) {
	Serial.print(n);
#if BOARD_ENABLE_BLE
	blePrintNum(n);
#endif
}

/**
 * @brief Print a byte as two-character uppercase hex to USB serial and optionally BLE
 * @param b Byte value to print
 */
void printHex(uint8_t b) {
	if (b < 0x10) printStr("0"); // Pad single-digit hex with leading zero
	Serial.print(b, HEX);
#if BOARD_ENABLE_BLE
	blePrintHex(b);
#endif
}

/**
 * @brief Print a newline (CRLF) to USB serial and optionally BLE
 */
void printLn() {
	Serial.println();
#if BOARD_ENABLE_BLE
	blePrintLn();
#endif
}

#include "common.h"
