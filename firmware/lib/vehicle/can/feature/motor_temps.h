#pragma once
#include <stdint.h>

// ── Motor / Inverter Temperature Decoder ────────────────────────────────────
// Source: legacy/ColinM-sys-tesla-can-boost + DBC Model3CAN.dbc
//
// Rear motor (all Model 3/Y) — CAN ID 0x315 (VehicleBus):
//   DBC: BO_ 789 ID315RearInverterTemps: 8 VehicleBus
//   RearTempInverter315 :  8|8@1+  scale=1  offset=-40  °C
//   RearTempStator315   : 16|8@1+  scale=1  offset=-40  °C
//   RearTempInvHeatsink315 : 32|8@1+  scale=1  offset=-40  °C
//
// Front motor (dual-motor only) — CAN ID 0x376 (VehicleBus):
//   DBC: BO_ 886 ID376FrontInverterTemps: 8 VehicleBus
//   TempInverter376     :  8|8@1+  scale=1  offset=-40  °C
//   TempStator376       : 16|8@1+  scale=1  offset=-40  °C
//   TempInvHeatsink376  : 32|8@1+  scale=1  offset=-40  °C
//
// All signals: raw byte value − 40 → temperature in °C.
// Range: 0..255 raw → −40..+215 °C.

inline int8_t decodeRearInvTemp(const uint8_t *d)
{
	return (int8_t)((int16_t)d[1] - 40);
}
inline int8_t decodeRearStatorTemp(const uint8_t *d)
{
	return (int8_t)((int16_t)d[2] - 40);
}
inline int8_t decodeRearHeatsinkTemp(const uint8_t *d)
{
	return (int8_t)((int16_t)d[4] - 40);
}

inline int8_t decodeFrontInvTemp(const uint8_t *d)
{
	return (int8_t)((int16_t)d[1] - 40);
}
inline int8_t decodeFrontStatorTemp(const uint8_t *d)
{
	return (int8_t)((int16_t)d[2] - 40);
}
inline int8_t decodeFrontHeatsinkTemp(const uint8_t *d)
{
	return (int8_t)((int16_t)d[4] - 40);
}
