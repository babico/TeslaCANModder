#pragma once
#include "core/types.h"
#include "protocol/can.h"
void driverSend(const Frame& f, uint8_t bus = 0);

// ── Mirror Bit Helpers (0x273 UI_vehicleControl) ─────────────────────────────
enum MirrorFoldRequest { MIRROR_IDLE = 0, MIRROR_FOLD = 1, MIRROR_UNFOLD = 2 };

inline void setMirrorFold(Frame& f, MirrorFoldRequest req) {
  if (f.dlc < 4) return;
  f.data[3] = (f.data[3] & ~0x03) | (req & 0x03);  // bits 24-25
}

inline void setMirrorHeat(Frame& f, bool enable) {
  if (f.dlc < 4) return;
  if (enable) f.data[3] |= 0x04;   // bit 26
  else        f.data[3] &= ~0x04;
}

inline void setAutoFoldMirrors(Frame& f, bool enable) {
  if (f.dlc < 7) return;
  if (enable) f.data[6] |= 0x10;   // bit 52
  else        f.data[6] &= ~0x10;
}

inline void setMirrorDipOnReverse(Frame& f, bool enable) {
  if (f.dlc < 7) return;
  if (enable) f.data[6] |= 0x20;   // bit 53
  else        f.data[6] &= ~0x20;
}

// ── Mirror Control (0x273) ───────────────────────────────────────────────────

static void controlMirrorFold(MirrorFoldRequest req, State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setMirrorFold(f, req);
  
  for (uint8_t i = 0; i < 50; i++) {
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}

static void controlMirrorHeat(State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setMirrorHeat(f, true);
  
  for (uint8_t i = 0; i < 30; i++) {
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}

static void controlAutoFoldMirrors(State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setAutoFoldMirrors(f, true);
  
  for (uint8_t i = 0; i < 30; i++) {
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}

static void controlMirrorDip(State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setMirrorDipOnReverse(f, true);
  
  for (uint8_t i = 0; i < 30; i++) {
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}
