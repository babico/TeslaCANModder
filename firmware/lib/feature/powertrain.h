#pragma once
#include "core/types.h"
#include "infra/can.h"

// ── Powertrain Telemetry (read-only) ────────────────────────────────────────
// Decode motor and drivetrain signals for performance monitoring.
//
// 0x257: Vehicle speed (signed, km/h)
//   Byte[2:3] = speed * 100, signed 16-bit
//
// 0x118: Gear + Accelerator pedal
//   Byte[0] bits[3:1] = gear state: 0=invalid, 1=P, 2=R, 3=N, 4=D, 5=SNA
//   Byte[1] = accelerator pedal % (0-100)
//
// 0x129: Steering angle
//   Byte[0:1] = steering angle * 10, signed 16-bit (degrees, + = right)
//
// 0x106: Rear motor RPM
//   Byte[4:5] = RPM * 1, signed 16-bit
//
// 0x115: Front motor RPM (dual motor)
//   Byte[4:5] = RPM * 1, signed 16-bit

inline float decodeVehicleSpeed(const uint8_t *data)
{
	int16_t raw = (int16_t)((data[2] << 8) | data[3]);
	return raw / 100.0f;
}

inline uint8_t decodeGearState(const uint8_t *data)
{
	return (data[0] >> 1) & 0x07;
}

inline uint8_t decodeAccelPedal(const uint8_t *data)
{
	return data[1];
}

// DI_brakePedalState: bits[20:19] of DI_STATE (0x118), 2-bit value
// 0=off, 1=applied, 2=hard applied. Return 0 or 1 (simple on/off).
inline uint8_t decodeBrakePedalState(const uint8_t *data)
{
	return (data[2] >> 3) & 0x03;
}

inline float decodeSteeringAngle(const uint8_t *data)
{
	int16_t raw = (int16_t)((data[0] << 8) | data[1]);
	return raw / 10.0f;
}

inline int16_t decodeMotorRpm(const uint8_t *data)
{
	return (int16_t)((data[4] << 8) | data[5]);
}

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

inline bool execPowertrainCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "powertrain") == 0)
	{
		// Handled in serial — just signals we want to print telemetry
		(void)s;
		return true;
	}
	return false;
}
