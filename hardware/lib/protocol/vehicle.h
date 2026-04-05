#pragma once
#include "core/types.h"
#include "protocol/can.h"

// ── 0x273 UI_vehicleControl Features ─────────────────────────────────────────
// DBC: BO_ 627 ID273UI_vehicleControl: 8 VehicleBus
// This frame controls most vehicle functions via CAN injection

// ── Mirror Control ───────────────────────────────────────────────────────────
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

// ── Lock Control ─────────────────────────────────────────────────────────────
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

// ── Trunk/Frunk Control ──────────────────────────────────────────────────────
inline void setFrunkRequest(Frame& f, bool open) {
  if (f.dlc < 1) return;
  if (open) f.data[0] |= 0x20;   // bit 5
  else      f.data[0] &= ~0x20;
}

inline void setHornRequest(Frame& f, bool honk) {
  if (f.dlc < 8) return;
  if (honk) f.data[7] |= 0x20;   // bit 61
  else      f.data[7] &= ~0x20;
}

// ── Lighting Control ─────────────────────────────────────────────────────────
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

// ── Wiper Control ────────────────────────────────────────────────────────────
enum WiperRequest { WIPER_OFF = 0, WIPER_1 = 1, WIPER_2 = 2, WIPER_3 = 3 };

inline void setWiperRequest(Frame& f, WiperRequest speed) {
  if (f.dlc < 8) return;
  f.data[7] = (f.data[7] & ~0x07) | (speed & 0x07);  // bits 56-58
}

// ── Seat Heating Control ─────────────────────────────────────────────────────
enum SeatHeatLevel { SEAT_OFF = 0, SEAT_LOW = 1, SEAT_MED = 2, SEAT_HIGH = 3 };

inline void setSeatHeatFL(Frame& f, SeatHeatLevel level) {
  if (f.dlc < 6) return;
  f.data[5] = (f.data[5] & ~0x0C) | ((level & 0x03) << 2);  // bits 42-43
}

inline void setSeatHeatFR(Frame& f, SeatHeatLevel level) {
  if (f.dlc < 6) return;
  f.data[5] = (f.data[5] & ~0x30) | ((level & 0x03) << 4);  // bits 44-45
}

inline void setSeatHeatRL(Frame& f, SeatHeatLevel level) {
  if (f.dlc < 6) return;
  f.data[5] = (f.data[5] & ~0xC0) | ((level & 0x03) << 6);  // bits 46-47
}

inline void setSeatHeatRR(Frame& f, SeatHeatLevel level) {
  if (f.dlc < 7) return;
  f.data[6] = (f.data[6] & ~0x0C) | ((level & 0x03) << 2);  // bits 50-51
}

inline void setSeatHeatRC(Frame& f, SeatHeatLevel level) {
  if (f.dlc < 7) return;
  f.data[6] = (f.data[6] & ~0x03) | (level & 0x03);  // bits 48-49
}

// ── Display Control ──────────────────────────────────────────────────────────
inline void setDisplayBrightness(Frame& f, uint8_t level) {
  if (f.dlc < 5) return;
  f.data[4] = level;  // bits 32-39, factor 0.5
}

// ── Power Control ────────────────────────────────────────────────────────────
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
