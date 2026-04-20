#pragma once
#include <stdint.h>

// ── O(1) CAN ID Filter ──────────────────────────────────────────────────────
// Bitmask-based CAN ID accept/reject for IDs 0-2047 (standard 11-bit).
// 2048 bits = 256 bytes = 64 uint32_t words.
// Replaces linear search with constant-time bit test.

#define ID_FILTER_WORDS 64  // 2048 / 32

struct IdFilter {
  uint32_t bits[ID_FILTER_WORDS];
};

inline void idFilterClear(IdFilter& f) {
  for (uint8_t i = 0; i < ID_FILTER_WORDS; i++) f.bits[i] = 0;
}

inline void idFilterAdd(IdFilter& f, uint16_t id) {
  if (id < 2048) {
    f.bits[id >> 5] |= (1UL << (id & 0x1F));
  }
}

inline void idFilterRemove(IdFilter& f, uint16_t id) {
  if (id < 2048) {
    f.bits[id >> 5] &= ~(1UL << (id & 0x1F));
  }
}

inline bool idFilterTest(const IdFilter& f, uint16_t id) {
  if (id >= 2048) return false;
  return (f.bits[id >> 5] & (1UL << (id & 0x1F))) != 0;
}

// Software accept filter: returns true if frame ID should be processed
static IdFilter swFilterBus0;
static IdFilter swFilterBus1;

inline void swFilterInit() {
  idFilterClear(swFilterBus0);
  idFilterClear(swFilterBus1);
}

inline bool swFilterAccept(uint8_t bus, uint16_t id) {
  switch (bus) {
    case 0: return idFilterTest(swFilterBus0, id);
    case 1: return idFilterTest(swFilterBus1, id);
    default: return true;  // Bus 2 (body) passes all
  }
}
