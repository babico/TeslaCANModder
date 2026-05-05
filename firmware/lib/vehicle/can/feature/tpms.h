#pragma once
#include "core/types.h"

// ── TPMS (Tire Pressure Monitoring System) ───────────────────────────────────
// Decode CAN ID 0x219 (VCSEC_TPMSData) for 4-wheel tire pressure display.
// Pressure encoded as uint8 × 0.025 bar (0-6.375 bar range).
// Temperature encoded as uint8 - 40 °C offset.

#define CAN_ID_TPMS 0x219

// Decode tire pressures from 0x219 frame data
// Byte layout: [FL_psi, FR_psi, RL_psi, RR_psi, FL_temp, FR_temp, RL_temp, RR_temp]
// Pressure: raw × 0.025 bar = raw × 0.3625 psi
// Temperature: raw - 40 °C

inline float decodeTpmsPressureBar(uint8_t raw)
{
	return raw * 0.025f;
}

inline int8_t decodeTpmsTemp(uint8_t raw)
{
	return (int8_t)(raw - 40);
}

inline void decodeTpmsFrame(const uint8_t *data, uint8_t dlc, float *pressures, int8_t *temps)
{
	if (dlc < 8)
		return;
	pressures[0] = decodeTpmsPressureBar(data[0]); // Front Left
	pressures[1] = decodeTpmsPressureBar(data[1]); // Front Right
	pressures[2] = decodeTpmsPressureBar(data[2]); // Rear Left
	pressures[3] = decodeTpmsPressureBar(data[3]); // Rear Right
	temps[0] = decodeTpmsTemp(data[4]);
	temps[1] = decodeTpmsTemp(data[5]);
	temps[2] = decodeTpmsTemp(data[6]);
	temps[3] = decodeTpmsTemp(data[7]);
}

// ── TPMS Command ─────────────────────────────────────────────────────────────

// Overload: decode TPMS from Frame and populate State
inline void decodeTpms(const Frame &f, State &s)
{
	if (f.dlc < 8)
		return;
	decodeTpmsFrame(f.data, f.dlc, s.tpmsPressure, s.tpmsTemp);
	s.hasTpms = true;
}

static bool executeTpmsCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "tpms") != 0)
		return false;
	// Handled in serial output — just returns true to trigger TPMS JSON output
	return s.hasTpms;
}
