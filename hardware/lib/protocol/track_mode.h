#pragma once
#include <cstring>
#include <Arduino.h>
#include "core/types.h"
#include "protocol/can.h"
void driverSend(const Frame& f, uint8_t bus = 0);

// ── Track Mode Inject ────────────────────────────────────────────────────────
// Modifies CAN 0x313 (UI_trackModeSettings) byte[0] bits 1:0 = 0x01 to enable.
// Sources: ev-open-can-tools, hypery11-flipper.

static void controlTrackMode(bool enable, State& s) {
  (void)s;
  Frame f;
  f.id = CAN_ID_TRACK_MODE;
  f.dlc = 8;
  memset(f.data, 0, 8);
  if (enable) {
    f.data[0] = (f.data[0] & 0xFC) | 0x01;  // bits 1:0 = 01
  }
  for (uint8_t i = 0; i < 20; i++) {
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}
