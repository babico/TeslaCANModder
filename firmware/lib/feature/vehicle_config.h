#pragma once
#include "core/types.h"

// ── 5.8 Vehicle-Specific Configuration ──────────────────────────────────────
// Support per-vehicle config profiles stored in NVS. Each profile stores
// the vehicle model, year, and feature compatibility flags.
//
// Vehicle models detected from CAN 0x398 GTW_carConfig:
//   - Model 3 (Highland/pre-Highland)
//   - Model Y
//   - Model S (Refresh/pre-Refresh)
//   - Model X
//
// NVS key: "vehModel" (uint8_t), "vehYear" (uint16_t)
// Command: vehicle (query — returns JSON with detected model/year/config)

enum VehicleModel : uint8_t {
  VEHICLE_UNKNOWN   = 0,
  VEHICLE_MODEL_3   = 1,
  VEHICLE_MODEL_Y   = 2,
  VEHICLE_MODEL_S   = 3,
  VEHICLE_MODEL_X   = 4,
  VEHICLE_CYBERTRUCK = 5
};

inline const char* vehicleModelName(VehicleModel m) {
  switch (m) {
    case VEHICLE_MODEL_3:    return "Model 3";
    case VEHICLE_MODEL_Y:    return "Model Y";
    case VEHICLE_MODEL_S:    return "Model S";
    case VEHICLE_MODEL_X:    return "Model X";
    case VEHICLE_CYBERTRUCK: return "Cybertruck";
    default:                 return "Unknown";
  }
}

// Decode vehicle model from GTW_carConfig 0x398
// Byte 1 bits [7:4] = platform ID:
//   0x1 = Model S, 0x2 = Model X, 0x3 = Model 3, 0x4 = Model Y, 0x5 = Cybertruck
// Byte 2 bits [7:0] = model year offset from 2016
inline void decodeVehicleConfig(const Frame& f, State& s) {
  if (f.dlc < 3) return;
  uint8_t platform = (f.data[1] >> 4) & 0x0F;
  switch (platform) {
    case 1: s.vehicleModel = VEHICLE_MODEL_S;   break;
    case 2: s.vehicleModel = VEHICLE_MODEL_X;   break;
    case 3: s.vehicleModel = VEHICLE_MODEL_3;   break;
    case 4: s.vehicleModel = VEHICLE_MODEL_Y;   break;
    case 5: s.vehicleModel = VEHICLE_CYBERTRUCK; break;
    default: s.vehicleModel = VEHICLE_UNKNOWN;   break;
  }
  s.vehicleYear = 2016 + (f.data[2] & 0xFF);
  s.hasVehicleConfig = true;
}

// Feature compatibility per vehicle model
struct VehicleCapabilities {
  bool supportsFsd;
  bool supportsTrackMode;
  bool supportsSummon;
  bool supportsMirrorAutoFold;
  bool supportsDualMotor;
};

inline VehicleCapabilities getVehicleCapabilities(VehicleModel model) {
  VehicleCapabilities cap = {true, false, true, true, false};
  switch (model) {
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

// Query command
inline bool execVehicleConfigCmd(const char* cmd, State& s) {
  return strcmp(cmd, "vehicle") == 0;
}
