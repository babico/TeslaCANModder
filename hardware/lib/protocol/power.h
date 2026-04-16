#pragma once
#include "core/types.h"
#include "protocol/can.h"
void driverSend(const Frame& f, uint8_t bus);

// ── Power Bit Helpers (0x273 UI_vehicleControl) ─────────────────────────────
inline void setAccessoryPower(Frame& f, bool enable) {
  if (f.dlc < 1) return;
  if (enable) f.data[0] |= 0x01;   // bit 0
  else        f.data[0] &= ~0x01;
}

inline void setPowerOff(Frame& f, bool off) {
  if (f.dlc < 4) return;
  if (off) f.data[3] |= 0x80;   // bit 31
  else     f.data[3] &= ~0x80;
}

inline void setDriveStateRequest(Frame& f, bool enable) {
  if (f.dlc < 8) return;
  if (enable) f.data[7] |= 0x40;   // bit 62
  else        f.data[7] &= ~0x40;
}

// ── Power Control (0x273) ────────────────────────────────────────────────────

static void controlPowerOff(State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setPowerOff(f, true);

  for (uint8_t i = 0; i < 30; i++) {
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}

static void controlAccessoryPower(bool enable, State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setAccessoryPower(f, enable);
  
  for (uint8_t i = 0; i < 30; i++) {
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}

static void controlDriveState(State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setDriveStateRequest(f, true);
  
  for (uint8_t i = 0; i < 30; i++) {
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}
