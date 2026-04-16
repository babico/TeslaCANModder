#pragma once
#include "core/types.h"
#include "protocol/can.h"
void driverSend(const Frame& f, uint8_t bus);

// ── Sentry Mode Control (0x284) ──────────────────────────────────────────────

static void controlSentry(bool enable, State& s) {
  Frame f;
  f.id = CAN_ID_SENTRY;
  f.dlc = 5;
  f.data[0] = enable ? 0x20 : 0x00;
  f.data[1] = 0x00;
  f.data[2] = 0x00;
  f.data[3] = 0x00;
  f.data[4] = 0x00;
  
  for (uint8_t i = 0; i < 30; i++) {
    driverSend(f, BUS_BODY);
    delay(20);
  }
}
