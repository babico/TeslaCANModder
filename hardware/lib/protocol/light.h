#pragma once
#include "core/types.h"
#include "protocol/can.h"
void driverSend(const Frame& f, uint8_t bus);

// ── Lighting Bit Helpers (0x273 UI_vehicleControl) ───────────────────────────
inline void setFrontFogSwitch(Frame& f, bool enable) {
  if (f.dlc < 1) return;
  if (enable) f.data[0] |= 0x08;   // bit 3
  else        f.data[0] &= ~0x08;
}

inline void setRearFogSwitch(Frame& f, bool enable) {
  if (f.dlc < 3) return;
  if (enable) f.data[2] |= 0x80;   // bit 23
  else        f.data[2] &= ~0x80;
}

inline void setAutoHighBeam(Frame& f, bool enable) {
  if (f.dlc < 6) return;
  if (enable) f.data[5] |= 0x02;   // bit 41
  else        f.data[5] &= ~0x02;
}

inline void setAmbientLighting(Frame& f, bool enable) {
  if (f.dlc < 6) return;
  if (enable) f.data[5] |= 0x01;   // bit 40
  else        f.data[5] &= ~0x01;
}

inline void setSeeYouHomeLighting(Frame& f, bool enable) {
  if (f.dlc < 4) return;
  if (enable) f.data[3] |= 0x40;   // bit 30
  else        f.data[3] &= ~0x40;
}

enum DomeLightSwitch { DOME_OFF = 0, DOME_ON = 1, DOME_AUTO = 2 };

inline void setDomeLightSwitch(Frame& f, DomeLightSwitch mode) {
  if (f.dlc < 8) return;
  f.data[7] = (f.data[7] & ~0x18) | ((mode & 0x03) << 3);  // bits 59-60
}

// ── Light Control (0x273) ────────────────────────────────────────────────────

static void controlFrontFog(State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setFrontFogSwitch(f, true);
  
  for (uint8_t i = 0; i < 30; i++) {
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}

static void controlRearFog(State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setRearFogSwitch(f, true);
  
  for (uint8_t i = 0; i < 30; i++) {
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}

static void controlAutoHighBeam(State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setAutoHighBeam(f, true);
  
  for (uint8_t i = 0; i < 30; i++) {
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}

static void controlAmbientLight(State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setAmbientLighting(f, true);
  
  for (uint8_t i = 0; i < 30; i++) {
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}

static void controlHomeLight(State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setSeeYouHomeLighting(f, true);
  
  for (uint8_t i = 0; i < 30; i++) {
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}

static void controlDomeLight(DomeLightSwitch mode, State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setDomeLightSwitch(f, mode);
  
  for (uint8_t i = 0; i < 30; i++) {
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}
