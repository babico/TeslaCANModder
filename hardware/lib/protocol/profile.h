#pragma once
#include "core/types.h"

// ── Speed Profile Encoding ───────────────────────────────────────────────────
// Writes the speed profile into the FSD mux frame (HW3/Legacy v12/v13 or HW4).

inline void setSpeedProfileV12V13(Frame& f, int profile) {
  if (f.dlc < 7) return;
  f.data[6] = (f.data[6] & ~0x06) | ((profile & 0x03) << 1);
}

inline void writeHW4SpeedProfile(Frame& f, int profile) {
  if (f.dlc < 8) return;
  f.data[7] = (f.data[7] & ~0x70) | ((profile & 0x07) << 4);
}
