#pragma once
#include "core/can/bus.h"

// ── Tesla CAN IDs ────────────────────────────────────────────────────────────
// 69 - Legacy stalk position (Model S/X pre-AP, FSD enable trigger)
#define CAN_ID_LEGACY_STALK 0x045
// 281 - VCLEFT_windowControl (vent open/close/position)
#define CAN_ID_WINDOW_VENT 0x119
// 627 - UI_vehicleControl (summon, lock/unlock, park commands)
#define CAN_ID_UI_VEHICLE_CTRL 0x273
// 644 - UI_sentryMode (sentry mode enable/disable)
#define CAN_ID_SENTRY 0x284
// 755 - HVAC_request (climate commands: temp, fan speed, mode)
#define CAN_ID_CLIMATE 0x2F3
// 819 - UI_chargeRequest (charge limit, port open, schedule)
#define CAN_ID_CHARGE 0x333
// 820 - UI_driveConfig (pedal mode, regen level, creep/roll/hold)
#define CAN_ID_DRIVE_CONFIG 0x334
// 947 - UI_trunkControl (frunk/trunk open; glovebox unlatch)
#define CAN_ID_TRUNK_CTRL 0x3B3
// 130 - UI_tripPlanning (preconditioning)
#define CAN_ID_PRECONDITION 0x082
// 306 - BMS_hvBusStatus (V/A/kW)
#define CAN_ID_BMS_HV_BUS 0x132
// 658 - BMS_socStatus (SoC%)
#define CAN_ID_BMS_SOC 0x292
// 786 - BMS_thermalStatus (battery pack, coolant, and cell temperatures)
#define CAN_ID_BMS_THERMAL 0x312
// 787 - UI_trackModeSettings (track mode enable, handling profile)
#define CAN_ID_TRACK_MODE 0x313
// 792 - GTW_carState (OTA update state: byte[6] bits[1:0]: 0=none, 1=available, 2=installing, 3=scheduled)
#define CAN_ID_GTW_CAR_STATE 0x318
// 826 - UI_energyGraphData (Wh/km)
#define CAN_ID_BMS_ENERGY 0x33A
// 818 - BMS_bmbMinMax (cell V min/max)
#define CAN_ID_BMS_MIN_MAX 0x332
// 850 - BMS_energyStatus (degradation)
#define CAN_ID_BMS_ENERGY_ST 0x352
// 594 - BMS_powerAvailable (regen/discharge limits)
#define CAN_ID_BMS_POWER_AV 0x252
// 530 - BMS_status (HV state, contactor, precondition flags)
#define CAN_ID_BMS_STATUS 0x212
// 722 - BMS_driveLimits (bus voltage/current limits)
#define CAN_ID_BMS_DRIVE_LIM 0x2D2
// 978 - BMS_kwhCounter (lifetime charge/discharge totals)
#define CAN_ID_BMS_KWH_CNT 0x3D2
// 1010 - BMS_kwhCountersMultiplexed (AC/DC/regen/drive)
#define CAN_ID_BMS_KWH_MUX 0x3F2
// 1025 - BMS_brickVoltages (individual cells, multiplexed)
#define CAN_ID_BMS_BRICK_V 0x401
// 880 - EPAS3P_sysStatus (nag killer + steering mode)
#define CAN_ID_EPAS_TORQUE 0x370
// 920 - GTW_carConfig (HW gen: byte[0] bits[7:6]: 0/1=Legacy, 2=HW3, 3=HW4)
#define CAN_ID_GTW_CAR_CFG 0x398
// 921 - DAS blind-spot rear left/right levels
#define CAN_ID_BLIND_SPOT 0x399
// 923 - DAS_status (hands-on state)
#define CAN_ID_DAS_STATUS 0x39B
// 697 - DAS_setSpeed (cruise control set-speed, ACC state)
#define CAN_ID_DAS_CONTROL 0x2B9
// 905 - DAS_accSpeedLimit (ACC speed cap from sign/map recognition)
#define CAN_ID_DAS_STATUS2 0x389
// 985 - UI_gpsVehicleSpeed / UI_mppSpeedLimit
#define CAN_ID_UI_GPS_SPEED 0x3D9
// 258 - VCLEFT door/latch status
#define CAN_ID_VCLEFT_DOOR_STATUS 0x102
// 259 - VCRIGHT door/latch/trunk status
#define CAN_ID_VCRIGHT_DOOR_STATUS 0x103
// 737 - VCFRONT status (frunk/any-door)
#define CAN_ID_VCFRONT_STATUS 0x2E1
// 929 - VCFRONT vehicle status (driver door)
#define CAN_ID_VCFRONT_VEH_STATUS 0x3A1
// 817 - DAS_autopilotConfig (tier readback, ~1 Hz — TLSSC target)
#define CAN_ID_DAS_AP_CONFIG 0x331
// 2047 - GTW_carConfig on mixed/Ethernet bridge (autopilot tier)
#define CAN_ID_GTW_CONFIG_ETH 0x7FF
// 537 - VCSEC_TPMSData (tire pressures, all four wheels)
#define CAN_ID_TPMS 0x219
// 1011 - ID3F3SeatbeltStatus (front/rear buckled flags)
#define CAN_ID_SEATBELT_STATUS 0x3F3
// 1013 - VCFRONT_vehicleLights (exterior light states)
#define CAN_ID_VCFRONT_LIGHTS 0x3F5
// 682 - HVAC_airRecircStatus (recirculation mode)
#define CAN_ID_AIR_RECIRC 0x2AA
// 599 - ID257VehicleSpeed (signed km/h, ChassisBus)
#define CAN_ID_VEHICLE_SPEED 0x257
// 280 - DI_state (gear position, accel pedal, brake)
#define CAN_ID_DI_STATE 0x118
// 297 - ID129SteeringAngle (signed 0.1°, ChassisBus)
#define CAN_ID_STEERING_ANGLE 0x129
// 262 - DI_motorRPM (rear motor RPM, all variants)
#define CAN_ID_REAR_MOTOR 0x106
// 277 - DI_frontMotorRPM (front motor RPM, dual-motor only)
#define CAN_ID_FRONT_MOTOR 0x115
// 553 - EPAS_internalHarness (CRC-8 protected)
#define CAN_ID_EPAS_HARNESS 0x229
// 585 - DI_steerAssist (drive mode readback, CRC-8 protected)
#define CAN_ID_DI_STEER 0x249
// 962 - VCLEFT_switchStatus (Palladium turn signal buttons)
#define CAN_ID_VCLEFT_SWITCH 0x3C2
// 921 - DAS_ISA speed alert (HW4-only; shares ID 0x399 with BLIND_SPOT — used as HW4 presence discriminator)
#define CAN_ID_ISA_SPEED 0x399
// 1006 - Legacy FSD mux (DAS_autopilotControl, Model S/X pre-HW3; presence confirms LEGACY variant)
#define CAN_ID_LEGACY_FSD_MUX 0x3EE
// 1016 - DAS_followDistance (HW3/HW4 follow-distance dial, settings 1–4)
#define CAN_ID_FOLLOW_DIST 0x3F8
// 1021 - DAS_FSDControl (HW3/HW4 FSD activation mux; intercepted and modified for FSD inject)
#define CAN_ID_FSD_MUX 0x3FD
// 373  - ID175WheelSpeed (FL/FR/RL/RR packed 13-bit LE, 0.04 km/h, ChassisBus)
#define CAN_ID_WHEEL_SPEED 0x175
// 789  - ID315RearInverterTemps (rear inverter/stator/heatsink °C, VehicleBus)
#define CAN_ID_REAR_INV_TEMPS 0x315
// 886  - ID376FrontInverterTemps (front inverter/stator/heatsink °C, VehicleBus, dual-motor only)
#define CAN_ID_FRONT_INV_TEMPS 0x376

// ── Drive Checksum (shared by pedal, regen, stop) ────────────────────────────
inline uint8_t driveChecksum(const uint8_t *data, uint8_t len)
{
	uint8_t sum = 0;
	for (uint8_t i = 0; i < len - 1; i++)
		sum += data[i];
	return sum & 0xFF;
}

// ── Shared Frame Decoders ─────────────────────────────────────────────────────
// Byte 5 bits[7:5]: follow-distance dial value (1–4; 0 = not present).
inline uint8_t readFollowDistance(const Frame &f)
{
	return f.dlc >= 6 ? ((f.data[5] & 0xE0) >> 5) : 0;
}

// Byte 4 bit 6: user has selected FSD in the UI (STEERING_MODE = 5).
inline bool isFSDSelectedInUI(const Frame &f)
{
	return f.dlc >= 5 ? ((f.data[4] >> 6) & 0x01) : false;
}
