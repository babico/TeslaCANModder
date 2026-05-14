#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/telemetry/powertrain.h
 * @brief Powertrain telemetry decoders for speed, gear, pedal, steering, and motor RPM
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/types.h"
#include "vehicle/can/ids.h"

/**
 * @brief Decode vehicle speed from CAN 0x257 payload
 * @param data Raw frame payload (bytes 2-3 contain signed speed * 100)
 * @return Speed in km/h as a floating-point value
 */
inline float decodeVehicleSpeed(const uint8_t *data)
{
	int16_t raw = (int16_t)((data[2] << 8) | data[3]);  // signed 16-bit, scale 100
	return raw / 100.0f;
}

/**
 * @brief Decode gear state from CAN 0x118 byte 0, bits 3:1
 * @param data Raw frame payload
 * @return Gear code: 0=invalid, 1=P, 2=R, 3=N, 4=D, 5=SNA
 */
inline uint8_t decodeGearState(const uint8_t *data)
{
	return (data[0] >> 1) & 0x07;  // 3-bit gear field at bits 3:1
}

/**
 * @brief Decode accelerator pedal percentage from CAN 0x118 byte 1
 * @param data Raw frame payload
 * @return Pedal position 0-100 percent
 */
inline uint8_t decodeAccelPedal(const uint8_t *data)
{
	return data[1];
}

/**
 * @brief Decode brake pedal state from CAN 0x118 (DI_STATE) bits 20:19
 * @param data Raw frame payload
 * @return 2-bit value: 0=off, 1=applied, 2=hard applied
 */
inline uint8_t decodeBrakePedalState(const uint8_t *data)
{
	return (data[2] >> 3) & 0x03;  // bits 20:19 mapped to byte 2 bits 4:3
}

/**
 * @brief Decode steering angle from CAN 0x129 bytes 0-1
 * @param data Raw frame payload (signed 16-bit, scale 10)
 * @return Steering angle in degrees (positive = right)
 */
inline float decodeSteeringAngle(const uint8_t *data)
{
	int16_t raw = (int16_t)((data[0] << 8) | data[1]);  // signed, scale factor 10
	return raw / 10.0f;
}

/**
 * @brief Decode motor RPM from CAN 0x106 or 0x115 bytes 4-5
 * @param data Raw frame payload (signed 16-bit RPM)
 * @return Motor RPM as signed integer
 */
inline int16_t decodeMotorRpm(const uint8_t *data)
{
	return (int16_t)((data[4] << 8) | data[5]);
}

/**
 * @brief Convert a numeric gear code to a human-readable single-character name
 * @param gear Gear code from decodeGearState()
 * @return Single-character string: "P", "R", "N", "D", or "?" for unknown
 */
inline const char *gearName(uint8_t gear)
{
	switch (gear)
	{
	case 1:
		return "P";
	case 2:
		return "R";
	case 3:
		return "N";
	case 4:
		return "D";
	default:
		return "?";
	}
}

/**
 * @brief Execute a powertrain telemetry command
 * @param cmd Null-terminated command string (expects "powertrain")
 * @param s Device state (unused; telemetry printing handled by serial layer)
 * @return true if the command was recognized
 */
static bool executePowertrainCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "powertrain") == 0)
	{
		(void)s;  // telemetry output handled in serial layer
		return true;
	}
	return false;
}
