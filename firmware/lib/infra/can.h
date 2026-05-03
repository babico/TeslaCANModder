#pragma once
#include "core/types.h"

// ── Tesla X179 Connector — Fixed Bus Positions ───────────────────────────────
// Physical MCP2515 modules at fixed pin positions:
//   Bus 0 (MCP2515_1): X179 pins 13-14 → Chassis / Autopilot CAN
//   Bus 1 (MCP2515_2): X179 pins 9-10  → Vehicle Control CAN
//   Bus 2 (MCP2515_3): X179 pins 2-3   → Body Control CAN

#define BUS_CHASSIS 0
#define BUS_VEHICLE 1
#define BUS_BODY 2

// ── Active Bus Flags (set by build: 0=off, 1=on) ────────────────────────────
// If a flag is not provided by the build, default it to off.
#ifndef BUS_CHASSIS_ACTIVE
#define BUS_CHASSIS_ACTIVE 0
#endif
#ifndef BUS_VEHICLE_ACTIVE
#define BUS_VEHICLE_ACTIVE 0
#endif
#ifndef BUS_BODY_ACTIVE
#define BUS_BODY_ACTIVE 0
#endif

// ── Bus Helpers ─────────────────────────────────────────────────────────────
inline bool busActive(uint8_t bus)
{
	switch (bus)
	{
	case BUS_CHASSIS:
		return BUS_CHASSIS_ACTIVE;
	case BUS_VEHICLE:
		return BUS_VEHICLE_ACTIVE;
	case BUS_BODY:
		return BUS_BODY_ACTIVE;
	default:
		return false;
	}
}
#define BUS_MAX 3

// ── Tesla CAN IDs ────────────────────────────────────────────────────────────
#define CAN_ID_LEGACY_STALK 69
#define CAN_ID_WINDOW_VENT 0x119		 // 281 - Window vent control
#define CAN_ID_UI_VEHICLE_CTRL 0x273	 // 627 - UI_vehicleControl (summon etc.)
#define CAN_ID_SENTRY 0x284				 // 644 - Sentry mode control
#define CAN_ID_CLIMATE 0x2F3			 // 755 - Climate control
#define CAN_ID_CHARGE 0x333				 // 819 - Charge control
#define CAN_ID_DRIVE_CONFIG 0x334		 // 820 - Drive config (pedal/regen/stop)
#define CAN_ID_TRUNK_CTRL 0x3B3			 // 947 - Trunk/Glovebox control
#define CAN_ID_PRECONDITION 0x082		 // 130 - UI_tripPlanning (preconditioning)
#define CAN_ID_BMS_HV_BUS 0x132			 // 306 - BMS_hvBusStatus (V/A/kW)
#define CAN_ID_BMS_SOC 0x292			 // 658 - BMS_socStatus (SoC%)
#define CAN_ID_BMS_THERMAL 0x312		 // 786 - BMS_thermalStatus (temp)
#define CAN_ID_TRACK_MODE 0x313			 // 787 - UI_trackModeSettings
#define CAN_ID_GTW_CAR_STATE 0x318		 // 792 - GTW_carState (OTA detect)
#define CAN_ID_BMS_ENERGY 0x33A			 // 826 - UI_energyGraphData (Wh/km)
#define CAN_ID_BMS_MIN_MAX 0x332		 // 818 - BMS_bmbMinMax (cell V min/max)
#define CAN_ID_BMS_ENERGY_ST 0x352		 // 850 - BMS_energyStatus (degradation)
#define CAN_ID_BMS_POWER_AV 0x252		 // 594 - BMS_powerAvailable (regen/discharge limits)
#define CAN_ID_BMS_STATUS 0x212			 // 530 - BMS_status (HV state, contactor, precondition flags)
#define CAN_ID_BMS_DRIVE_LIM 0x2D2		 // 722 - BMS_driveLimits (bus voltage/current limits)
#define CAN_ID_BMS_KWH_CNT 0x3D2		 // 978 - BMS_kwhCounter (lifetime charge/discharge totals)
#define CAN_ID_BMS_KWH_MUX 0x3F2		 // 1010 - BMS_kwhCountersMultiplexed (AC/DC/regen/drive)
#define CAN_ID_BMS_BRICK_V 0x401		 // 1025 - BMS_brickVoltages (individual cells, multiplexed)
#define CAN_ID_EPAS_TORQUE 0x370		 // 880 - EPAS3P_sysStatus (nag killer + steering mode)
#define CAN_ID_GTW_CAR_CFG 0x398		 // 920 - GTW_carConfig (auto HW detect)
#define CAN_ID_BLIND_SPOT 0x399			 // 921 - DAS blind-spot rear left/right levels
#define CAN_ID_DAS_STATUS 0x39B			 // 923 - DAS_status (hands-on state)
#define CAN_ID_DAS_CONTROL 0x2B9		 // 697 - DAS_setSpeed (cruise set-speed)
#define CAN_ID_DAS_STATUS2 0x389		 // 905 - DAS_accSpeedLimit
#define CAN_ID_UI_GPS_SPEED 0x3D9		 // 985 - UI_gpsVehicleSpeed / UI_mppSpeedLimit
#define CAN_ID_VCLEFT_DOOR_STATUS 0x102	 // 258 - VCLEFT door/latch status
#define CAN_ID_VCRIGHT_DOOR_STATUS 0x103 // 259 - VCRIGHT door/latch/trunk status
#define CAN_ID_VCFRONT_STATUS 0x2E1		 // 737 - VCFRONT status (frunk/any-door)
#define CAN_ID_VCFRONT_VEH_STATUS 0x3A1	 // 929 - VCFRONT vehicle status (driver door)
#define CAN_ID_DAS_AP_CONFIG 0x331		 // 817  - DAS_autopilotConfig (tier readback, ~1 Hz — TLSSC target)
#define CAN_ID_GTW_CONFIG_ETH 0x7FF		 // 2047 - GTW_carConfig on mixed/Ethernet bridge (autopilot tier)
#define CAN_ID_TPMS 0x219
#define CAN_ID_SEATBELT_STATUS 0x3F3
#define CAN_ID_VCFRONT_LIGHTS 0x3F5
#define CAN_ID_AIR_RECIRC 0x2AA
#define CAN_ID_VEHICLE_SPEED 0x257
#define CAN_ID_DI_STATE 0x118
#define CAN_ID_STEERING_ANGLE 0x129
#define CAN_ID_REAR_MOTOR 0x106
#define CAN_ID_FRONT_MOTOR 0x115   // 537 - VCSEC_TPMSData (tire pressures)
#define CAN_ID_EPAS_HARNESS 0x229  // 553 - EPAS_internalHarness (CRC-8 protected)
#define CAN_ID_DI_STEER 0x249	   // 585 - DI_steerAssist (CRC-8 protected)
#define CAN_ID_VCLEFT_SWITCH 0x3C2 // 962 - VCLEFT_switchStatus (Palladium turn signal buttons)
#define CAN_ID_ISA_SPEED 921
#define CAN_ID_LEGACY_FSD_MUX 1006
#define CAN_ID_FOLLOW_DIST 1016
#define CAN_ID_FSD_MUX 1021
#define CAN_ID_WHEEL_SPEED 0x175     // 373  - ID175WheelSpeed (FL/FR/RL/RR packed 13-bit LE, 0.04 km/h, ChassisBus)
#define CAN_ID_REAR_INV_TEMPS 0x315  // 789  - ID315RearInverterTemps (rear inverter/stator/heatsink °C, VehicleBus)
#define CAN_ID_FRONT_INV_TEMPS 0x376 // 886  - ID376FrontInverterTemps (front inverter/stator/heatsink °C, VehicleBus, dual-motor only)

// ── Drive Checksum (shared by pedal, regen, stop) ────────────────────────────
inline uint8_t driveChecksum(const uint8_t *data, uint8_t len)
{
	uint8_t sum = 0;
	for (uint8_t i = 0; i < len - 1; i++)
		sum += data[i];
	return sum & 0xFF;
}

// ── Frame Helpers ────────────────────────────────────────────────────────────
inline uint8_t readMuxID(const Frame &f)
{
	return f.dlc >= 1 ? (f.data[0] & 0x07) : 0;
}

inline bool isFSDSelectedInUI(const Frame &f)
{
	return f.dlc >= 5 ? ((f.data[4] >> 6) & 0x01) : false;
}

inline uint8_t readFollowDistance(const Frame &f)
{
	return f.dlc >= 6 ? ((f.data[5] & 0xE0) >> 5) : 0;
}

inline uint8_t readDASAutopilotStatus(const Frame &f)
{
	return f.dlc >= 1 ? (f.data[0] & 0x0F) : 0;
}

// DAS_autopilotState from 0x39B byte1 bits[7:4].
// 0=UNAVAIL 1=AVAIL 2=ACTIVE_NOMINAL 3=ACTIVE_MIN_DRIVER ...
// Used by AP-First mode to delay 0x3FD injection until AP is running.
inline uint8_t readDASAutopilotState(const Frame &f)
{
	return f.dlc >= 2 ? ((f.data[1] >> 4) & 0x0F) : 0;
}

inline bool isDASAutopilotActive(uint8_t status)
{
	return status >= 3 && status <= 5;
}

inline int8_t readGtwAutopilotTier(const Frame &f)
{
	if (f.dlc < 6)
		return -1;
	if (readMuxID(f) != 2)
		return -1;
	return (int8_t)((f.data[5] >> 2) & 0x07);
}

inline void setBit(Frame &f, int bit, bool val)
{
	int byteIdx = bit / 8;
	int bitIdx = bit % 8;
	if (byteIdx >= f.dlc)
		return;
	uint8_t mask = 1 << bitIdx;
	if (val)
		f.data[byteIdx] |= mask;
	else
		f.data[byteIdx] &= ~mask;
}
