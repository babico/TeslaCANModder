#pragma once
#include "state.h"

// ── Output Helpers ───────────────────────────────────────────────────────────
void printStr(const char* s) {
  Serial.print(s);
#if BOARD_ENABLE_BLE
  blePrint(s);
#endif
}

void printStr(const __FlashStringHelper* s) {
  Serial.print(s);
#if BOARD_ENABLE_BLE
  blePrint(s);
#endif
}

void printNum(long n) {
  Serial.print(n);
#if BOARD_ENABLE_BLE
  blePrintNum(n);
#endif
}

void printHex(uint8_t b) {
  if (b < 0x10) printStr("0");
  Serial.print(b, HEX);
#if BOARD_ENABLE_BLE
  blePrintHex(b);
#endif
}

void printLn() {
  Serial.println();
#if BOARD_ENABLE_BLE
  blePrintLn();
#endif
}

#include "common.h"
