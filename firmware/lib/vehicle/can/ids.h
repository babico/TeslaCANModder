#pragma once

/**
 * @file firmware/lib/vehicle/can/ids.h
 * @brief Tesla CAN arbitration IDs and shared frame decoders
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/can/bus.h"

// ── BUS_CHASSIS (0) — X179 pins 13-14 — Autopilot / Party CAN ───────────────
// All DAS injection targets this bus.

#define CAN_ID_LEGACY_STALK      0x045 // 69 — Legacy stalk position (RX, FSD enable trigger)
#define CAN_ID_WHEEL_SPEED       0x175 // 373 — ID175WheelSpeed (RX, FL/FR/RL/RR 13-bit LE, 0.04 km/h)
#define CAN_ID_EPAS_HARNESS      0x229 // 553 — EPAS_internalHarness (RX, CRC-8 protected)
#define CAN_ID_APS_EAC_MONITOR   0x27D // 637 — APS_eacMonitor (TX DAS, EPAS steer-allow gate)
#define CAN_ID_DAS_CONTROL       0x2B9 // 697 — DAS_control (TX DAS, longitudinal ACC)
#define CAN_ID_DAS_STATUS2       0x389 // 905 — DAS_accSpeedLimit (RX, ACC speed cap)
#define CAN_ID_ISA_SPEED         0x399 // 921 — DAS_ISA speed alert (RX, HW4-only)
#define CAN_ID_UI_GPS_SPEED      0x3D9 // 985 — UI_gpsVehicleSpeed (RX, map speed limit)
#define CAN_ID_LEGACY_FSD_MUX   0x3EE // 1006 — DAS_autopilotControl mux (RX, Legacy variant)
#define CAN_ID_FOLLOW_DIST       0x3F8 // 1016 — DAS_followDistance (RX, follow-distance 1–4)
#define CAN_ID_FSD_MUX           0x3FD // 1021 — DAS_FSDControl (RX, FSD activation mux)
#define CAN_ID_DAS_STEERING_CTRL 0x488 // 1160 — DAS_steeringControl (TX DAS, Motorola mixed-endian)

// ── BUS_VEHICLE (1) — X179 pins 9-10 — Vehicle Control CAN ──────────────────

#define CAN_ID_PRECONDITION      0x082 // 130 — UI_tripPlanning (TX, preconditioning enable)
#define CAN_ID_VCLEFT_DOOR_STATUS  0x102 // 258 — VCLEFT_doorStatus (RX, left door open/latch)
#define CAN_ID_VCRIGHT_DOOR_STATUS 0x103 // 259 — VCRIGHT_doorStatus (RX, right door + trunk latch)
#define CAN_ID_REAR_MOTOR        0x106 // 262 — DI_motorRPM (RX, rear drive unit RPM)
#define CAN_ID_FRONT_MOTOR       0x115 // 277 — DI_frontMotorRPM (RX, dual-motor only)
#define CAN_ID_DI_STATE          0x118 // 280 — DI_state (RX, gear/accel/brake)
#define CAN_ID_STEERING_ANGLE    0x129 // 297 — ID129SteeringAngle (RX, signed 0.1°/LSB)
#define CAN_ID_BMS_HV_BUS        0x132 // 306 — BMS_hvBusStatus (RX, voltage/current/power)
#define CAN_ID_BMS_STATUS        0x212 // 530 — BMS_status (RX, HV contactor state)
#define CAN_ID_TPMS              0x219 // 537 — VCSEC_TPMSData (RX, tire pressures)
#define CAN_ID_DI_STEER          0x249 // 585 — DI_steerAssist (RX, CRC-8 protected)
#define CAN_ID_BMS_POWER_AV      0x252 // 594 — BMS_powerAvailable (RX, regen/discharge kW)
#define CAN_ID_VEHICLE_SPEED     0x257 // 599 — ID257VehicleSpeed (RX, signed km/h)
#define CAN_ID_UI_VEHICLE_CTRL   0x273 // 627 — UI_vehicleControl (TX, summon/lock/frunk)
#define CAN_ID_BMS_SOC           0x292 // 658 — BMS_socStatus (RX, state of charge %)
#define CAN_ID_AIR_RECIRC        0x2AA // 682 — HVAC_airRecircStatus (TX, recirc mode)
#define CAN_ID_BMS_DRIVE_LIM     0x2D2 // 722 — BMS_driveLimits (RX, voltage/current limits)
#define CAN_ID_VCFRONT_STATUS    0x2E1 // 737 — VCFRONT_status (RX, frunk/door flags)
#define CAN_ID_CLIMATE           0x2F3 // 755 — HVAC_request (TX, temp/fan/mode)
#define CAN_ID_BMS_THERMAL       0x312 // 786 — BMS_thermalStatus (RX, pack/coolant/cell temps)
#define CAN_ID_TRACK_MODE        0x313 // 787 — UI_trackModeSettings (TX/RX, track mode)
#define CAN_ID_REAR_INV_TEMPS    0x315 // 789 — ID315RearInverterTemps (RX, inverter °C)
#define CAN_ID_GTW_CAR_STATE     0x318 // 792 — GTW_carState (RX, OTA state byte[6] bits[1:0])
#define CAN_ID_DAS_AP_CONFIG     0x331 // 817 — DAS_autopilotConfig (RX, AP tier ~1 Hz)
#define CAN_ID_BMS_MIN_MAX       0x332 // 818 — BMS_bmbMinMax (RX, cell voltage min/max)
#define CAN_ID_CHARGE            0x333 // 819 — UI_chargeRequest (TX, charge limit/port/schedule)
#define CAN_ID_DRIVE_CONFIG      0x334 // 820 — UI_driveConfig (TX, pedal/regen/creep)
#define CAN_ID_BMS_ENERGY        0x33A // 826 — UI_energyGraphData (RX, Wh/km)
#define CAN_ID_BMS_ENERGY_ST     0x352 // 850 — BMS_energyStatus (RX, degradation/range)
#define CAN_ID_EPAS_TORQUE       0x370 // 880 — EPAS3P_sysStatus (RX, nag killer source)
#define CAN_ID_FRONT_INV_TEMPS   0x376 // 886 — ID376FrontInverterTemps (RX, dual-motor only)
#define CAN_ID_GTW_CAR_CFG       0x398 // 920 — GTW_carConfig (RX, HW gen byte[0] bits[7:6])
#define CAN_ID_BLIND_SPOT        0x399 // 921 — DAS_blindSpotMonitor (RX, BSM left/right)
#define CAN_ID_DAS_STATUS        0x39B // 923 — DAS_status (RX, hands-on/AP/lane-change)
#define CAN_ID_VCFRONT_VEH_STATUS 0x3A1 // 929 — VCFRONT_vehicleStatus (RX, driver door)
#define CAN_ID_VCLEFT_SWITCH     0x3C2 // 962 — VCLEFT_switchStatus (TX, yoke turn-signal/ALC)
#define CAN_ID_BMS_KWH_CNT       0x3D2 // 978 — BMS_kwhCounter (RX, lifetime kWh totals)
#define CAN_ID_BMS_KWH_MUX       0x3F2 // 1010 — BMS_kwhCountersMultiplexed (RX, per-session)
#define CAN_ID_SEATBELT_STATUS   0x3F3 // 1011 — ID3F3SeatbeltStatus (RX, buckled flags)
#define CAN_ID_VCFRONT_LIGHTS    0x3F5 // 1013 — VCFRONT_vehicleLights (RX, exterior lights)
#define CAN_ID_BMS_BRICK_V       0x401 // 1025 — BMS_brickVoltages (RX, cell V multiplexed)
#define CAN_ID_GTW_CONFIG_ETH    0x7FF // 2047 — GTW_carConfig Ethernet bridge (RX, AP tier)

// ── BUS_BODY (2) — X179 pins 2-3 — Body Control CAN ─────────────────────────

#define CAN_ID_WINDOW_VENT       0x119 // 281 — VCLEFT_windowControl (TX, vent position 0-100)
#define CAN_ID_SENTRY            0x284 // 644 — UI_sentryMode (TX, sentry enable/disable)
#define CAN_ID_TRUNK_CTRL        0x3B3 // 947 — UI_trunkControl (TX, trunk/glovebox)

/**
 * @brief Compute simple additive checksum for drive-related frames.
 *
 * Used by pedal, regen, and stop commands. Sums all bytes except the last
 * (which holds the checksum itself) and returns the low 8 bits.
 *
 * @param data Pointer to frame data bytes.
 * @param len Total frame length including the checksum byte.
 * @return Checksum value (sum of bytes [0..len-2] & 0xFF).
 */
inline uint8_t driveChecksum(const uint8_t *data, uint8_t len)
{
	uint8_t sum = 0;
	for (uint8_t i = 0; i < len - 1; i++)
		sum += data[i];
	return sum & 0xFF;
}

/**
 * @brief Read follow-distance dial value from a DAS_followDistance frame.
 *
 * Extracts byte 5 bits[7:5] which encode the follow-distance setting (1–4).
 *
 * @param f CAN frame (expected DLC >= 6).
 * @return Follow-distance value (1–4), or 0 if frame too short.
 */
inline uint8_t readFollowDistance(const Frame &f)
{
	return f.dlc >= 6 ? ((f.data[5] & 0xE0) >> 5) : 0; // bits[7:5] of byte 5
}

/**
 * @brief Check if FSD is selected in the vehicle UI.
 *
 * Reads byte 4 bit 6 which indicates STEERING_MODE = 5 (FSD selected).
 *
 * @param f CAN frame (expected DLC >= 5).
 * @return True if FSD is selected in the UI, false otherwise.
 */
inline bool isFSDSelectedInUI(const Frame &f)
{
	return f.dlc >= 5 ? ((f.data[4] >> 6) & 0x01) : false; // byte 4, bit 6
}
