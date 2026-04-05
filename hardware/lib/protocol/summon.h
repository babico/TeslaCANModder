#pragma once
#include "types.h"

// ── ASS (Autopark Summon System) Commands ────────────────────────────────────
enum SummonDirection {
  SUMMON_FORWARD = 0,
  SUMMON_REVERSE = 1
};

enum SummonMode {
  SUMMON_STOP = 0,
  SUMMON_START = 1
};

// ── ASS Summon Control (0x273 UI_vehicleControl) ─────────────────────────────
inline void setSummonActive(Frame& f, bool active) {
  if (f.dlc < 1) return;
  if (active) f.data[0] |= 0x10;   // bit 4
  else        f.data[0] &= ~0x10;
}

inline void setSummonDirection(Frame& f, SummonDirection dir) {
  if (f.dlc < 1) return;
  if (dir == SUMMON_REVERSE) f.data[0] |= 0x20;   // bit 5
  else                       f.data[0] &= ~0x20;
}

inline void setSummonMode(Frame& f, SummonMode mode) {
  if (f.dlc < 1) return;
  if (mode == SUMMON_START) f.data[0] |= 0x01;   // bit 0
  else                      f.data[0] &= ~0x01;
}
