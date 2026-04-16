#pragma once
#include "core/types.h"
#include "protocol/can.h"
void driverSend(const Frame& f, uint8_t bus);

// ── Display Bit Helpers (0x273 UI_vehicleControl) ────────────────────────────
inline void setDisplayBrightness(Frame& f, uint8_t level) {
  if (f.dlc < 5) return;
  f.data[4] = level;  // bits 32-39, factor 0.5
}

// ── Display Control (0x273) ──────────────────────────────────────────────────

static void controlDisplayBrightness(uint8_t level, State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setDisplayBrightness(f, level);
  
  for (uint8_t i = 0; i < 20; i++) {
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}
