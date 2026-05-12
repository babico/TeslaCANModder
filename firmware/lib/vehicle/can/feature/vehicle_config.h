#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/vehicle_config.h
 * @brief Vehicle-specific configuration profiles decoded from CAN frame 0x398 (GTW_carConfig)
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/types.h"

/**
 * @brief Enumeration of supported Tesla vehicle models
 */
enum VehicleModel : uint8_t
{
	VEHICLE_UNKNOWN = 0,
	VEHICLE_MODEL_3 = 1,
	VEHICLE_MODEL_Y = 2,
	VEHICLE_MODEL_S = 3,
	VEHICLE_MODEL_X = 4,
	VEHICLE_CYBERTRUCK = 5
};

/**
 * @brief Return a human-readable name string for the given vehicle model
 * @param m The vehicle model enum value.
 * @return Pointer to a static string with the model name, or "Unknown" if unrecognized.
 */
inline const char *vehicleModelName(VehicleModel m)
{
	switch (m)
	{
	case VEHICLE_MODEL_3:
		return "Model 3";
	case VEHICLE_MODEL_Y:
		return "Model Y";
	case VEHICLE_MODEL_S:
		return "Model S";
	case VEHICLE_MODEL_X:
		return "Model X";
	case VEHICLE_CYBERTRUCK:
		return "Cybertruck";
	default:
		return "Unknown";
	}
}

/**
 * @brief Decode vehicle model and year from GTW_carConfig frame (CAN ID 0x398)
 * @param f The received CAN frame (must have dlc >= 3).
 * @param s Device state to populate with decoded vehicle config.
 * @note Byte 1 bits [7:4] encode the platform ID; byte 2 is the model year offset from 2016.
 */
inline void decodeVehicleConfig(const Frame &f, State &s)
{
	if (f.dlc < 3)
		return;
	// Platform ID is in the upper nibble of byte 1
	uint8_t platform = (f.data[1] >> 4) & 0x0F;
	switch (platform)
	{
	case 1:
		s.vehicleModel = VEHICLE_MODEL_S;
		break;
	case 2:
		s.vehicleModel = VEHICLE_MODEL_X;
		break;
	case 3:
		s.vehicleModel = VEHICLE_MODEL_3;
		break;
	case 4:
		s.vehicleModel = VEHICLE_MODEL_Y;
		break;
	case 5:
		s.vehicleModel = VEHICLE_CYBERTRUCK;
		break;
	default:
		s.vehicleModel = VEHICLE_UNKNOWN;
		break;
	}
	// Year is stored as offset from base year 2016
	s.vehicleYear = 2016 + (f.data[2] & 0xFF);
	s.hasVehicleConfig = true;
}

/**
 * @brief Feature compatibility flags for a given vehicle model
 */
struct VehicleCapabilities
{
	bool supportsFsd;             // Full Self-Driving hardware support
	bool supportsTrackMode;       // Track Mode availability
	bool supportsSummon;          // Smart Summon capability
	bool supportsMirrorAutoFold;  // Auto-folding side mirrors
	bool supportsDualMotor;       // Dual motor drivetrain
};

/**
 * @brief Return the feature capability set for a given vehicle model
 * @param model The vehicle model to query.
 * @return A VehicleCapabilities struct with flags set per model.
 */
inline VehicleCapabilities getVehicleCapabilities(VehicleModel model)
{
	VehicleCapabilities cap = {true, false, true, true, false};
	switch (model)
	{
	case VEHICLE_MODEL_3:
		cap.supportsTrackMode = true;
		cap.supportsDualMotor = true;
		break;
	case VEHICLE_MODEL_Y:
		cap.supportsDualMotor = true;
		break;
	case VEHICLE_MODEL_S:
		cap.supportsTrackMode = true;
		cap.supportsDualMotor = true;
		break;
	case VEHICLE_MODEL_X:
		cap.supportsDualMotor = true;
		break;
	case VEHICLE_CYBERTRUCK:
		cap.supportsTrackMode = true;
		cap.supportsDualMotor = true;
		break;
	default:
		break;
	}
	return cap;
}

/**
 * @brief Execute the "vehicle" query command to report detected config
 * @param cmd The command string to match.
 * @param s Device state (unused beyond match check).
 * @return True if the command matched "vehicle", false otherwise.
 */
static bool executeVehicleConfigCmd(const char *cmd, State &s)
{
	return strcmp(cmd, "vehicle") == 0;
}
