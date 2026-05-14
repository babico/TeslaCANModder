#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/telemetry/wheel_speeds.h
 * @brief Wheel speed decoder for CAN ID 0x175 (ChassisBus, 8 bytes)
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <stdint.h>

/**
 * @brief Decode front-left wheel speed from CAN frame 0x175
 * @param d Pointer to the 8-byte CAN payload.
 * @return Speed in km/h, or 0.0f if the signal is unavailable (raw == 8191).
 * @note 13-bit little-endian signal at bit offset 0, scale 0.04 km/h per count.
 */
inline float decodeWheelSpeedFL(const uint8_t *d)
{
	// Bits 0-12: full byte 0 + lower 5 bits of byte 1
	uint16_t raw = (uint16_t)d[0] | ((uint16_t)(d[1] & 0x1F) << 8);
	return (raw == 8191) ? 0.0f : raw * 0.04f;  // 8191 (0x1FFF) = signal not available
}

/**
 * @brief Decode front-right wheel speed from CAN frame 0x175
 * @param d Pointer to the 8-byte CAN payload.
 * @return Speed in km/h, or 0.0f if the signal is unavailable (raw == 8191).
 * @note 13-bit little-endian signal at bit offset 13, scale 0.04 km/h per count.
 */
inline float decodeWheelSpeedFR(const uint8_t *d)
{
	// Bits 13-25: upper 3 bits of byte 1 + full byte 2 + lower 2 bits of byte 3
	uint16_t raw = ((uint16_t)(d[1] >> 5)) | ((uint16_t)d[2] << 3) | ((uint16_t)(d[3] & 0x03) << 11);
	return (raw == 8191) ? 0.0f : raw * 0.04f;
}

/**
 * @brief Decode rear-left wheel speed from CAN frame 0x175
 * @param d Pointer to the 8-byte CAN payload.
 * @return Speed in km/h, or 0.0f if the signal is unavailable (raw == 8191).
 * @note 13-bit little-endian signal at bit offset 26, scale 0.04 km/h per count.
 */
inline float decodeWheelSpeedRL(const uint8_t *d)
{
	// Bits 26-38: upper 6 bits of byte 3 + lower 7 bits of byte 4
	uint16_t raw = ((uint16_t)(d[3] >> 2)) | ((uint16_t)(d[4] & 0x7F) << 6);
	return (raw == 8191) ? 0.0f : raw * 0.04f;
}

/**
 * @brief Decode rear-right wheel speed from CAN frame 0x175
 * @param d Pointer to the 8-byte CAN payload.
 * @return Speed in km/h, or 0.0f if the signal is unavailable (raw == 8191).
 * @note 13-bit little-endian signal at bit offset 39, scale 0.04 km/h per count.
 */
inline float decodeWheelSpeedRR(const uint8_t *d)
{
	// Bits 39-51: bit 7 of byte 4 + full byte 5 + lower 3 bits of byte 6
	uint16_t raw = ((uint16_t)(d[4] >> 7)) | ((uint16_t)d[5] << 1) | ((uint16_t)(d[6] & 0x07) << 9);
	return (raw == 8191) ? 0.0f : raw * 0.04f;
}
