#pragma once
#include "core/types.h"

// ── Tesla X179 Connector — Fixed Bus Positions ───────────────────────────────
// Physical MCP2515 modules at fixed pin positions:
//   Bus 0 (MCP2515_1): X179 pins 13-14 → FSD / Autopilot CAN
//   Bus 1 (MCP2515_2): X179 pins 9-10  → Vehicle Control CAN
//   Bus 2 (MCP2515_3): X179 pins 2-3   → Body Control CAN

#define BUS_FSD      0
#define BUS_VEHICLE  1
#define BUS_BODY     2

// ── Active Bus Flags (set by build: 0=off, 1=on) ────────────────────────────
#ifndef BUS_FSD_ACTIVE
  #define BUS_FSD_ACTIVE 1
#endif
#ifndef BUS_VEHICLE_ACTIVE
  #define BUS_VEHICLE_ACTIVE 0
#endif
#ifndef BUS_BODY_ACTIVE
  #define BUS_BODY_ACTIVE 0
#endif

// ── Bus Helpers ─────────────────────────────────────────────────────────────
inline bool busActive(uint8_t bus) {
  switch (bus) {
    case BUS_FSD:     return BUS_FSD_ACTIVE;
    case BUS_VEHICLE: return BUS_VEHICLE_ACTIVE;
    case BUS_BODY:    return BUS_BODY_ACTIVE;
    default:          return false;
  }
}
#define BUS_MAX 3

// ── Tesla CAN IDs ────────────────────────────────────────────────────────────
#define CAN_ID_LEGACY_STALK   69
#define CAN_ID_WINDOW_VENT    0x119   // 281 - Window vent control
#define CAN_ID_UI_VEHICLE_CTRL 0x273  // 627 - UI_vehicleControl (summon etc.)
#define CAN_ID_SENTRY         0x284   // 644 - Sentry mode control
#define CAN_ID_CLIMATE        0x2F3   // 755 - Climate control
#define CAN_ID_CHARGE         0x333   // 819 - Charge control
#define CAN_ID_DRIVE_CONFIG   0x334   // 820 - Drive config (pedal/regen/stop)
#define CAN_ID_TRUNK_CTRL     0x3B3   // 947 - Trunk/Glovebox control
#define CAN_ID_ISA_SPEED      921
#define CAN_ID_LEGACY_FSD_MUX 1006
#define CAN_ID_FOLLOW_DIST    1016
#define CAN_ID_FSD_MUX        1021

// ── Frame Helpers ────────────────────────────────────────────────────────────
inline uint8_t readMuxID(const Frame& f) {
  return f.dlc >= 1 ? (f.data[0] & 0x07) : 0;
}

inline bool isFSDSelectedInUI(const Frame& f) {
  return f.dlc >= 5 ? ((f.data[4] >> 6) & 0x01) : false;
}

inline uint8_t readFollowDistance(const Frame& f) {
  return f.dlc >= 6 ? ((f.data[5] & 0xE0) >> 5) : 0;
}

inline void setBit(Frame& f, int bit, bool val) {
  int byteIdx = bit / 8;
  int bitIdx = bit % 8;
  if (byteIdx >= f.dlc) return;
  uint8_t mask = 1 << bitIdx;
  if (val) f.data[byteIdx] |= mask;
  else f.data[byteIdx] &= ~mask;
}
