#pragma once
#include "core/types.h"

// ── Follow Distance → Speed Profile Mapping ──────────────────────────────────
// Maps the follow distance CAN value to a speed profile index (-1 = invalid).

inline int mapHW3FollowDistToProfile(uint8_t fd) {
  static const int8_t map[] = {-1, 2, 1, 0};
  return fd < 4 ? map[fd] : -1;
}

inline int mapHW4FollowDistToProfile(uint8_t fd) {
  static const int8_t map[] = {-1, 3, 2, 1, 0, 4};
  return fd < 6 ? map[fd] : -1;
}
