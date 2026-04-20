#pragma once
#include <string.h>
#include "core/forward.h"
#include "infra/can.h"
#include "infra/burst.h"

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
  
  startBurst(s, f, BUS_VEHICLE, 30, 20);
}

static void controlChildLock(State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setChildDoorLock(f, true);
  
  startBurst(s, f, BUS_VEHICLE, 30, 20);
}

static void controlHorn(State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setHornRequest(f, true);
  
  startBurst(s, f, BUS_VEHICLE, 30, 20);
}

// ── Lock Command Execution ───────────────────────────────────────────────────

static bool execLockCmd(const char* cmd, State& s) {
  if (!s.hasCtrl) return false;
  
  if (strcmp(cmd, "lock") == 0) {
    controlLock(LOCK, s);
    return true;
  }
  if (strcmp(cmd, "unlock") == 0) {
    controlLock(UNLOCK, s);
    return true;
  }
  if (strcmp(cmd, "lock:child") == 0) {
    controlChildLock(s);
    return true;
  }
  if (strcmp(cmd, "horn") == 0) {
    controlHorn(s);
    return true;
  }
  return false;
}

