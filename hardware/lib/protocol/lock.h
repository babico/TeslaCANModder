#pragma once
#include "core/types.h"
#include "protocol/can.h"
void driverSend(const Frame& f, uint8_t bus);

// ── Lock Bit Helpers (0x273 UI_vehicleControl) ───────────────────────────────
enum LockRequest { LOCK_IDLE = 0, LOCK = 1, UNLOCK = 2 };

inline void setLockRequest(Frame& f, LockRequest req) {
  if (f.dlc < 3) return;
  f.data[2] = (f.data[2] & ~0x0E) | ((req & 0x07) << 1);  // bits 17-19
}

inline void setChildDoorLock(Frame& f, bool enable) {
  if (f.dlc < 3) return;
  if (enable) f.data[2] |= 0x01;   // bit 16
  else        f.data[2] &= ~0x01;
}

// Horn uses 0x273 bit 61 — kept here as it's lock-adjacent (security)
inline void setHornRequest(Frame& f, bool honk) {
  if (f.dlc < 8) return;
  if (honk) f.data[7] |= 0x20;   // bit 61
  else      f.data[7] &= ~0x20;
}

// ── Lock Control (0x273) ─────────────────────────────────────────────────────

static void controlLock(LockRequest req, State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setLockRequest(f, req);
  
  for (uint8_t i = 0; i < 30; i++) {
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}

static void controlChildLock(State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setChildDoorLock(f, true);
  
  for (uint8_t i = 0; i < 30; i++) {
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}

static void controlHorn(State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setHornRequest(f, true);
  
  for (uint8_t i = 0; i < 30; i++) {
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}
