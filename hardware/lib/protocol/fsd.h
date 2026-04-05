#pragma once
#include "types.h"

// ── Profile Encoding ─────────────────────────────────────────────────────────
inline void setSpeedProfileV12V13(Frame& f, int profile) {
  if (f.dlc < 7) return;
  f.data[6] = (f.data[6] & ~0x06) | ((profile & 0x03) << 1);
}

inline void writeHW4SpeedProfile(Frame& f, int profile) {
  if (f.dlc < 8) return;
  f.data[7] = (f.data[7] & ~0x70) | ((profile & 0x07) << 4);
}

// ── HW3 Speed Offset ─────────────────────────────────────────────────────────
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

// ── HW4 ISA Checksum ─────────────────────────────────────────────────────────
inline uint8_t computeHW4IsaChecksum(const Frame& f) {
  if (f.dlc < 8) return 0;
  uint8_t sum = 0;
  for (int i = 0; i < 7; i++) sum += f.data[i];
  sum += (f.id & 0xFF) + (f.id >> 8);
  return sum;
}

// ── Follow Distance Mapping ──────────────────────────────────────────────────
inline int mapHW3FollowDistToProfile(uint8_t fd) {
  static const int8_t map[] = {-1, 2, 1, 0};
  return fd < 4 ? map[fd] : -1;
}

inline int mapHW4FollowDistToProfile(uint8_t fd) {
  static const int8_t map[] = {-1, 3, 2, 1, 0, 4};
  return fd < 6 ? map[fd] : -1;
}
