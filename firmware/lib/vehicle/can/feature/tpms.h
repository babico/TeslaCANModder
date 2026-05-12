#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/tpms.h
 * @brief TPMS (Tire Pressure Monitoring System) decoder and command handler
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/types.h"

#define CAN_ID_TPMS 0x219	// VCSEC_TPMSData frame identifier

/**
 * @brief Decode a raw TPMS pressure byte to bar.
 *
 * Pressure is encoded as uint8 * 0.025 bar (range 0-6.375 bar).
 *
 * @param raw Raw pressure byte from the CAN frame.
 * @return Pressure in bar.
 */
inline float decodeTpmsPressureBar(uint8_t raw)
{
	return raw * 0.025f;
}

/**
 * @brief Decode a raw TPMS temperature byte to degrees Celsius.
 *
 * Temperature is encoded with a -40 °C offset (raw 0 = -40 °C).
 *
 * @param raw Raw temperature byte from the CAN frame.
 * @return Temperature in degrees Celsius.
 */
inline int8_t decodeTpmsTemp(uint8_t raw)
{
	return (int8_t)(raw - 40);
}

/**
 * @brief Decode a full TPMS frame (0x219) into pressure and temperature arrays.
 *
 * Byte layout: [FL_psi, FR_psi, RL_psi, RR_psi, FL_temp, FR_temp, RL_temp, RR_temp].
 * Requires at least 8 bytes of payload.
 *
 * @param data Pointer to the frame payload bytes.
 * @param dlc Data length code of the frame.
 * @param pressures Output array of 4 floats (FL, FR, RL, RR) in bar.
 * @param temps Output array of 4 int8_t values (FL, FR, RL, RR) in °C.
 */
inline void decodeTpmsFrame(const uint8_t *data, uint8_t dlc, float *pressures, int8_t *temps)
{
	if (dlc < 8)
		return;
	pressures[0] = decodeTpmsPressureBar(data[0]);	// Front Left
	pressures[1] = decodeTpmsPressureBar(data[1]);	// Front Right
	pressures[2] = decodeTpmsPressureBar(data[2]);	// Rear Left
	pressures[3] = decodeTpmsPressureBar(data[3]);	// Rear Right
	temps[0] = decodeTpmsTemp(data[4]);
	temps[1] = decodeTpmsTemp(data[5]);
	temps[2] = decodeTpmsTemp(data[6]);
	temps[3] = decodeTpmsTemp(data[7]);
}

/**
 * @brief Decode TPMS data from a Frame into the global state.
 * @param f CAN frame (expected ID 0x219 with dlc >= 8).
 * @param s Global state to populate with pressure and temperature values.
 */
inline void decodeTpms(const Frame &f, State &s)
{
	if (f.dlc < 8)
		return;
	decodeTpmsFrame(f.data, f.dlc, s.tpmsPressure, s.tpmsTemp);
	s.hasTpms = true;
}

/**
 * @brief Execute the TPMS query command.
 *
 * Responds to "tpms" by returning true if TPMS data is available,
 * signaling the serial output layer to emit TPMS JSON.
 *
 * @param cmd Null-terminated command string (must be exactly "tpms").
 * @param s Global state reference.
 * @return true if TPMS data has been received and is available for output.
 */
static bool executeTpmsCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "tpms") != 0)
		return false;
	return s.hasTpms;
}
