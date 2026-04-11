#pragma once
#include "core/types.h"

// ── HW3 Speed Offset ─────────────────────────────────────────────────────────
// Reads UI offset steps from CAN and writes computed speed offset.

inline void writeHW3SpeedOffset(Frame& f, int offset) {
  if (f.dlc < 2) return;
  f.data[0] = (f.data[0] & ~0xC0) | ((offset & 0x03) << 6);
  f.data[1] = (f.data[1] & ~0x3F) | (offset >> 2);
}

inline int readHW3UiOffsetSteps(const Frame& f) {
  return f.dlc >= 4 ? (int)((f.data[3] >> 1) & 0x3F) - 30 : 0;
}

inline int calculateHW3SpeedOffset(int steps) {
  int val = steps * 5;
  if (val < 0) return 0;
  if (val > 100) return 100;
  return val;
}
