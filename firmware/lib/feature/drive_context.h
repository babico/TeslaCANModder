#pragma once
#include "core/types.h"
#include "infra/can.h"

// ── Drive context decode helpers (D-11 / D-13) ─────────────────────────────
// Door/frunk/trunk open-state and cruise/speed-limit context extracted from
// legacy-documented vehicle/chassis frames.

inline uint32_t readBitsLE(const uint8_t* data, uint8_t start, uint8_t len) {
  uint32_t out = 0;
  for (uint8_t i = 0; i < len; ++i) {
    const uint8_t bit = start + i;
    const uint8_t byteIdx = bit / 8;
    const uint8_t bitIdx = bit % 8;
    if (data[byteIdx] & (1u << bitIdx)) out |= (1u << i);
  }
  return out;
}

inline bool decodeLatchOpen(uint8_t status) {
  // 2 = closed, 0 = unknown/SNA; treat all other latch states as open/ajar/moving.
  return status != 2 && status != 0;
}

inline bool decodeDoorFrontLeftOpen(const Frame& f) {
  return decodeLatchOpen((uint8_t)(readBitsLE(f.data, 0, 4) & 0x0F));
}

inline bool decodeDoorRearLeftOpen(const Frame& f) {
  return decodeLatchOpen((uint8_t)(readBitsLE(f.data, 4, 4) & 0x0F));
}

inline bool decodeDoorFrontRightOpen(const Frame& f) {
  return decodeLatchOpen((uint8_t)(readBitsLE(f.data, 0, 4) & 0x0F));
}

inline bool decodeDoorRearRightOpen(const Frame& f) {
  return decodeLatchOpen((uint8_t)(readBitsLE(f.data, 4, 4) & 0x0F));
}

inline bool decodeTrunkOpen(const Frame& f) {
  return decodeLatchOpen((uint8_t)(readBitsLE(f.data, 56, 4) & 0x0F));
}

inline bool decodeFrunkOpen(const Frame& f) {
  const uint8_t mux = (uint8_t)(readBitsLE(f.data, 0, 3) & 0x07);
  if (mux != 0) return false;
  return decodeLatchOpen((uint8_t)(readBitsLE(f.data, 3, 4) & 0x0F));
}

inline bool decodeAnyDoorOpen(const Frame& f) {
  const uint8_t mux = (uint8_t)(readBitsLE(f.data, 0, 3) & 0x07);
  if (mux != 0) return false;
  return readBitsLE(f.data, 50, 1) != 0;
}

inline bool decodeDriverDoorOpen(const Frame& f) {
  // DBC: 1 = closed, 0 = open.
  return readBitsLE(f.data, 31, 1) == 0;
}

inline float decodeCruiseSetSpeedKph(const Frame& f) {
  uint16_t raw = (uint16_t)(readBitsLE(f.data, 0, 12) & 0x0FFF);
  if (raw == 0x0FFF) return 0.0f;
  return raw * 0.1f;
}

inline float decodeAccSpeedLimitKph(const Frame& f) {
  uint16_t raw = (uint16_t)(readBitsLE(f.data, 0, 10) & 0x03FF);
  if (raw == 0 || raw == 0x03FF) return 0.0f;
  const float mph = raw * 0.2f;
  return mph * 1.60934f;
}

inline float decodeMapSpeedLimitKph(const Frame& f) {
  uint8_t raw = (uint8_t)(readBitsLE(f.data, 48, 5) & 0x1F);
  if (raw == 0 || raw == 31) return 0.0f;
  float v = raw * 5.0f;
  const bool unitsKph = readBitsLE(f.data, 46, 1) != 0;
  if (!unitsKph) v *= 1.60934f;
  return v;
}
