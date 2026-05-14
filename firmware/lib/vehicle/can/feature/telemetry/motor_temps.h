#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/telemetry/motor_temps.h
 * @brief Decodes motor and inverter temperature signals from Tesla CAN frames
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <stdint.h>

/**
 * @brief Decode rear inverter temperature from CAN ID 0x315 frame data.
 * @param d Pointer to the 8-byte CAN frame payload.
 * @return Temperature in degrees Celsius (-40 to +215 range).
 */
inline int8_t decodeRearInvTemp(const uint8_t *d)
{
	// Byte 1: raw value with -40 offset per DBC signal RearTempInverter315
	return (int8_t)((int16_t)d[1] - 40);
}

/**
 * @brief Decode rear stator temperature from CAN ID 0x315 frame data.
 * @param d Pointer to the 8-byte CAN frame payload.
 * @return Temperature in degrees Celsius (-40 to +215 range).
 */
inline int8_t decodeRearStatorTemp(const uint8_t *d)
{
	// Byte 2: raw value with -40 offset per DBC signal RearTempStator315
	return (int8_t)((int16_t)d[2] - 40);
}

/**
 * @brief Decode rear inverter heatsink temperature from CAN ID 0x315 frame data.
 * @param d Pointer to the 8-byte CAN frame payload.
 * @return Temperature in degrees Celsius (-40 to +215 range).
 */
inline int8_t decodeRearHeatsinkTemp(const uint8_t *d)
{
	// Byte 4: raw value with -40 offset per DBC signal RearTempInvHeatsink315
	return (int8_t)((int16_t)d[4] - 40);
}

/**
 * @brief Decode front inverter temperature from CAN ID 0x376 frame data (dual-motor only).
 * @param d Pointer to the 8-byte CAN frame payload.
 * @return Temperature in degrees Celsius (-40 to +215 range).
 */
inline int8_t decodeFrontInvTemp(const uint8_t *d)
{
	// Byte 1: raw value with -40 offset per DBC signal TempInverter376
	return (int8_t)((int16_t)d[1] - 40);
}

/**
 * @brief Decode front stator temperature from CAN ID 0x376 frame data (dual-motor only).
 * @param d Pointer to the 8-byte CAN frame payload.
 * @return Temperature in degrees Celsius (-40 to +215 range).
 */
inline int8_t decodeFrontStatorTemp(const uint8_t *d)
{
	// Byte 2: raw value with -40 offset per DBC signal TempStator376
	return (int8_t)((int16_t)d[2] - 40);
}

/**
 * @brief Decode front inverter heatsink temperature from CAN ID 0x376 frame data (dual-motor only).
 * @param d Pointer to the 8-byte CAN frame payload.
 * @return Temperature in degrees Celsius (-40 to +215 range).
 */
inline int8_t decodeFrontHeatsinkTemp(const uint8_t *d)
{
	// Byte 4: raw value with -40 offset per DBC signal TempInvHeatsink376
	return (int8_t)((int16_t)d[4] - 40);
}
