#pragma once
#include "core/types.h"

// ── HW4 ISA Speed Chime Checksum ─────────────────────────────────────────────
// Computes the ISA speed message checksum (byte 7 = sum of bytes 0-6 + ID).

inline uint8_t computeHW4IsaChecksum(const Frame& f) {
  if (f.dlc < 8) return 0;
  uint8_t sum = 0;
  for (int i = 0; i < 7; i++) sum += f.data[i];
  sum += (f.id & 0xFF) + (f.id >> 8);
  return sum;
}
