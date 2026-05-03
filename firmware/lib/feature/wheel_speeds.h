#pragma once
#include <stdint.h>

// ── Wheel Speed Decoder — CAN ID 0x175 (ChassisBus, 8 bytes) ────────────────
// DBC: BO_ 373 ID175WheelSpeed — four 13-bit little-endian signals packed
//   WheelSpeedFL175 : 0|13@1+  scale=0.04  offset=0  unit=km/h  SNA=8191
//   WheelSpeedFR175 :13|13@1+  scale=0.04  offset=0  unit=km/h  SNA=8191
//   WheelSpeedRL175 :26|13@1+  scale=0.04  offset=0  unit=km/h  SNA=8191
//   WheelSpeedRR175 :39|13@1+  scale=0.04  offset=0  unit=km/h  SNA=8191
//
// Bit layout (little-endian, LSB first):
//   FL: bits  0–12  → data[0] bits0-7 + data[1] bits0-4
//   FR: bits 13–25  → data[1] bits5-7 + data[2] bits0-7 + data[3] bits0-1
//   RL: bits 26–38  → data[3] bits2-7 + data[4] bits0-6
//   RR: bits 39–51  → data[4] bit7    + data[5] bits0-7 + data[6] bits0-2
//
// A raw value of 8191 (0x1FFF) means signal not available; return 0.0f.

inline float decodeWheelSpeedFL(const uint8_t *d)
{
	uint16_t raw = (uint16_t)d[0] | ((uint16_t)(d[1] & 0x1F) << 8);
	return (raw == 8191) ? 0.0f : raw * 0.04f;
}

inline float decodeWheelSpeedFR(const uint8_t *d)
{
	uint16_t raw = ((uint16_t)(d[1] >> 5)) |
	               ((uint16_t)d[2] << 3) |
	               ((uint16_t)(d[3] & 0x03) << 11);
	return (raw == 8191) ? 0.0f : raw * 0.04f;
}

inline float decodeWheelSpeedRL(const uint8_t *d)
{
	uint16_t raw = ((uint16_t)(d[3] >> 2)) |
	               ((uint16_t)(d[4] & 0x7F) << 6);
	return (raw == 8191) ? 0.0f : raw * 0.04f;
}

inline float decodeWheelSpeedRR(const uint8_t *d)
{
	uint16_t raw = ((uint16_t)(d[4] >> 7)) |
	               ((uint16_t)d[5] << 1) |
	               ((uint16_t)(d[6] & 0x07) << 9);
	return (raw == 8191) ? 0.0f : raw * 0.04f;
}
