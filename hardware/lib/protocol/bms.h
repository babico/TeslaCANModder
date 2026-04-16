#pragma once
#include "core/types.h"
#include "protocol/can.h"

// ── BMS Battery Telemetry Decoder ────────────────────────────────────────────
// Decodes BMS CAN frames into human-readable battery telemetry.
// Sources: tuncasoftbildik, hypery11-flipper, J0811 legacy repos.

// CAN 0x132 (306) — BMS_hvBusStatus: Pack voltage & current
inline float decodeBmsVoltage(const uint8_t* data) {
  uint16_t raw = ((uint16_t)data[0] << 8) | data[1];
  return raw * 0.01f;  // Volts
}

inline float decodeBmsCurrent(const uint8_t* data) {
  int16_t raw = (int16_t)(((uint16_t)data[2] << 8) | data[3]);
  return raw * 0.1f;  // Amps (negative = discharging)
}

inline float decodeBmsPower(const uint8_t* data) {
  return decodeBmsVoltage(data) * decodeBmsCurrent(data) * 0.001f;  // kW
}

// CAN 0x292 (658) — BMS_socStatus: State of Charge
inline float decodeBmsSoc(const uint8_t* data) {
  uint16_t raw = ((uint16_t)(data[0] & 0x03) << 8) | data[1];
  return raw * 0.1f;  // Percent (0.0 - 100.0)
}

// CAN 0x312 (786) — BMS_thermalStatus: Battery temperatures
inline int8_t decodeBmsTempMin(const uint8_t* data) {
  return (int8_t)(data[0]) - 40;  // Celsius
}

inline int8_t decodeBmsTempMax(const uint8_t* data) {
  return (int8_t)(data[1]) - 40;  // Celsius
}

// CAN 0x33A (826) — UI_energyGraphData: Energy consumption
inline float decodeBmsWhPerKm(const uint8_t* data) {
  uint16_t raw = ((uint16_t)data[0] << 8) | data[1];
  return raw * 0.1f;  // Wh/km
}
