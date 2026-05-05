#pragma once
#include "core/can/bus.h"

// ── Tesla CAN IDs ────────────────────────────────────────────────────────────
// Grouped by physical CAN bus lane (X179 connector).
// TX = firmware injects this frame.  RX = firmware reads/intercepts it.
//
//  BUS_CHASSIS (0)  X179 pins 13-14  Chassis CAN — vehicle ECUs (rack/ESP/IBST).
//                                    *All DAS injection targets this bus.*
//  BUS_VEHICLE (1)  X179 pins  9-10  Vehicle Control CAN
//  BUS_BODY    (2)  X179 pins   2-3  Body Control CAN

// ── BUS_CHASSIS (0) — Autopilot / Party CAN ──────────────────────────────────
// 69 - Legacy stalk position (RX/intercept, LEGACY only; FSD enable trigger)
#define CAN_ID_LEGACY_STALK      0x045
// 373 - ID175WheelSpeed (RX, FL/FR/RL/RR packed 13-bit LE, 0.04 km/h)
#define CAN_ID_WHEEL_SPEED       0x175
// 553 - EPAS_internalHarness (RX, harness detection; CRC-8 protected)
#define CAN_ID_EPAS_HARNESS      0x229
// 637 - APS_eacMonitor (TX DAS drive; EPAS steer-allow, must be sent to enable steering)
#define CAN_ID_APS_EAC_MONITOR   0x27D
// 697 - DAS_control (TX DAS drive + intercept; longitudinal ACC: set-speed, accel min/max, jerk, acc state)
#define CAN_ID_DAS_CONTROL       0x2B9
// 905 - DAS_accSpeedLimit (RX; ACC speed cap from sign/map recognition)
#define CAN_ID_DAS_STATUS2       0x389
// 921 - DAS_ISA speed alert (RX, HW4-only; shares 0x399 with BUS_VEHICLE CAN_ID_BLIND_SPOT — HW4 discriminator)
#define CAN_ID_ISA_SPEED         0x399
// 985 - UI_gpsVehicleSpeed / UI_mppSpeedLimit (RX; map-derived speed limit)
#define CAN_ID_UI_GPS_SPEED      0x3D9
// 1006 - DAS_autopilotControl mux (RX/intercept, LEGACY only; presence confirms Legacy variant)
#define CAN_ID_LEGACY_FSD_MUX   0x3EE
// 1016 - DAS_followDistance (RX/intercept, HW3/HW4; follow-distance dial settings 1–4)
#define CAN_ID_FOLLOW_DIST       0x3F8
// 1021 - DAS_FSDControl (RX/intercept, HW3/HW4; FSD activation mux; modified for FSD inject)
#define CAN_ID_FSD_MUX           0x3FD
// 1160 - DAS_steeringControl (TX DAS drive; steering angle request, Motorola mixed-endian fields)
#define CAN_ID_DAS_STEERING_CTRL 0x488

// ── BUS_VEHICLE (1) — Vehicle Control CAN ────────────────────────────────────
// 130 - UI_tripPlanning (TX; preconditioning enable)
#define CAN_ID_PRECONDITION      0x082
// 258 - VCLEFT_doorStatus (RX; front/rear left door open/latch state)
#define CAN_ID_VCLEFT_DOOR_STATUS  0x102
// 259 - VCRIGHT_doorStatus (RX; front/rear right door + trunk latch state)
#define CAN_ID_VCRIGHT_DOOR_STATUS 0x103
// 262 - DI_motorRPM (RX; rear drive unit RPM, all variants)
#define CAN_ID_REAR_MOTOR        0x106
// 277 - DI_frontMotorRPM (RX; front drive unit RPM, dual-motor only)
#define CAN_ID_FRONT_MOTOR       0x115
// 280 - DI_state (RX; gear position, accelerator pedal %, brake applied)
#define CAN_ID_DI_STATE          0x118
// 297 - ID129SteeringAngle (RX; signed 0.1°/LSB steering wheel angle)
#define CAN_ID_STEERING_ANGLE    0x129
// 306 - BMS_hvBusStatus (RX; HV bus voltage, current, power kW)
#define CAN_ID_BMS_HV_BUS        0x132
// 530 - BMS_status (RX; HV contactor state, precondition flags)
#define CAN_ID_BMS_STATUS        0x212
// 537 - VCSEC_TPMSData (RX; tire pressures all four wheels)
#define CAN_ID_TPMS              0x219
// 553 - (see BUS_CHASSIS — EPAS_internalHarness)
// 585 - DI_steerAssist (RX; active drive-mode readback; CRC-8 protected)
#define CAN_ID_DI_STEER          0x249
// 594 - BMS_powerAvailable (RX; regen/discharge limits kW)
#define CAN_ID_BMS_POWER_AV      0x252
// 599 - ID257VehicleSpeed (RX; signed vehicle speed km/h)
#define CAN_ID_VEHICLE_SPEED     0x257
// 627 - UI_vehicleControl (TX; summon, lock/unlock, frunk open)
#define CAN_ID_UI_VEHICLE_CTRL   0x273
// 658 - BMS_socStatus (RX; state of charge %)
#define CAN_ID_BMS_SOC           0x292
// 682 - HVAC_airRecircStatus (TX; air recirculation mode command)
#define CAN_ID_AIR_RECIRC        0x2AA
// 722 - BMS_driveLimits (RX; HV bus voltage/current limits)
#define CAN_ID_BMS_DRIVE_LIM     0x2D2
// 737 - VCFRONT_status (RX; frunk open, any-door flags)
#define CAN_ID_VCFRONT_STATUS    0x2E1
// 755 - HVAC_request (TX; climate commands: temp, fan speed, mode)
#define CAN_ID_CLIMATE           0x2F3
// 786 - BMS_thermalStatus (RX; battery pack, coolant, cell temperatures)
#define CAN_ID_BMS_THERMAL       0x312
// 787 - UI_trackModeSettings (TX/RX; track mode enable, handling profile)
#define CAN_ID_TRACK_MODE        0x313
// 789 - ID315RearInverterTemps (RX; rear inverter/stator/heatsink °C)
#define CAN_ID_REAR_INV_TEMPS    0x315
// 792 - GTW_carState (RX; OTA update state: byte[6] bits[1:0]: 0=none 1=avail 2=installing 3=scheduled)
#define CAN_ID_GTW_CAR_STATE     0x318
// 817 - DAS_autopilotConfig (RX; autopilot tier readback, ~1 Hz — TLSSC target)
#define CAN_ID_DAS_AP_CONFIG     0x331
// 818 - BMS_bmbMinMax (RX; cell voltage min/max)
#define CAN_ID_BMS_MIN_MAX       0x332
// 819 - UI_chargeRequest (TX; charge limit %, port open, schedule)
#define CAN_ID_CHARGE            0x333
// 820 - UI_driveConfig (TX; pedal mode, regen level, creep/roll/hold)
#define CAN_ID_DRIVE_CONFIG      0x334
// 826 - UI_energyGraphData (RX; energy use Wh/km)
#define CAN_ID_BMS_ENERGY        0x33A
// 850 - BMS_energyStatus (RX; pack degradation, rated range)
#define CAN_ID_BMS_ENERGY_ST     0x352
// 880 - EPAS3P_sysStatus (RX; nag killer source + active steering mode)
#define CAN_ID_EPAS_TORQUE       0x370
// 886 - ID376FrontInverterTemps (RX; front inverter/stator/heatsink °C, dual-motor only)
#define CAN_ID_FRONT_INV_TEMPS   0x376
// 920 - GTW_carConfig (RX; HW gen byte[0] bits[7:6]: 0/1=Legacy 2=HW3 3=HW4)
#define CAN_ID_GTW_CAR_CFG       0x398
// 921 - DAS_blindSpotMonitor (RX; BSM rear-left/right levels; shares 0x399 with BUS_CHASSIS CAN_ID_ISA_SPEED)
#define CAN_ID_BLIND_SPOT        0x399
// 923 - DAS_status (RX; hands-on detection state, AP state, lane-change state)
#define CAN_ID_DAS_STATUS        0x39B
// 929 - VCFRONT_vehicleStatus (RX; driver door open flag)
#define CAN_ID_VCFRONT_VEH_STATUS 0x3A1
// 962 - VCLEFT_switchStatus (TX; Palladium yoke turn-signal button; ALC confirm inject)
#define CAN_ID_VCLEFT_SWITCH     0x3C2
// 978 - BMS_kwhCounter (RX; lifetime charge/discharge totals kWh)
#define CAN_ID_BMS_KWH_CNT       0x3D2
// 1010 - BMS_kwhCountersMultiplexed (RX; per-session AC/DC/regen/drive kWh)
#define CAN_ID_BMS_KWH_MUX       0x3F2
// 1011 - ID3F3SeatbeltStatus (RX; front/rear seatbelt buckled flags)
#define CAN_ID_SEATBELT_STATUS   0x3F3
// 1013 - VCFRONT_vehicleLights (RX; exterior light states, turn signal active)
#define CAN_ID_VCFRONT_LIGHTS    0x3F5
// 1025 - BMS_brickVoltages (RX; individual cell voltages, multiplexed by brick index)
#define CAN_ID_BMS_BRICK_V       0x401
// 2047 - GTW_carConfig on mixed/Ethernet-bridge topology (RX; autopilot tier readback)
#define CAN_ID_GTW_CONFIG_ETH    0x7FF

// ── BUS_BODY (2) — Body Control CAN ──────────────────────────────────────────
// 281 - VCLEFT_windowControl (TX; vent open/close/position 0-100)
#define CAN_ID_WINDOW_VENT       0x119
// 644 - UI_sentryMode (TX; sentry mode enable/disable)
#define CAN_ID_SENTRY            0x284
// 947 - UI_trunkControl (TX; trunk open/close, glovebox unlatch)
#define CAN_ID_TRUNK_CTRL        0x3B3

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
