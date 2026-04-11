#pragma once
#include "core/types.h"
#include "protocol/can.h"
void driverSend(const Frame& f, uint8_t bus = 0);

// ── Wiper Bit Helpers (0x273 UI_vehicleControl) ─────────────────────────────
enum WiperRequest { WIPER_OFF = 0, WIPER_1 = 1, WIPER_2 = 2, WIPER_3 = 3 };

inline void setWiperRequest(Frame& f, WiperRequest speed) {
  if (f.dlc < 8) return;
  f.data[7] = (f.data[7] & ~0x07) | (speed & 0x07);  // bits 56-58
}

// ── Wiper Control (0x273) ────────────────────────────────────────────────────

static void controlWiper(WiperRequest speed, State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setWiperRequest(f, speed);
  
  for (uint8_t i = 0; i < 20; i++) {
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}
