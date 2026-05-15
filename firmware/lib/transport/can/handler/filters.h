#pragma once

/**
 * @file firmware/lib/transport/can/handler/filters.h
 * @brief Per-bus MCP2515 hardware filter configuration for the Tesla X179 connector
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "transport/can/bus.h"
#include "transport/can/id_filter.h"
#include "transport/can/esp32.h"
#include "vehicle/can/ids.h"
#include "vehicle/can/feature/telemetry/fw_compat.h"

/**
 * @brief Configure hardware acceptance filters on all three CAN buses
 *
 * Applies a toggle-inject pattern: when a feature is enabled its CAN IDs are
 * added to the acceptance list so the frame can be intercepted and mutated.
 * When raw listen mode is active, all filters are cleared to pass every frame.
 *
 * @param s Reference to the global firmware state used to determine active features
 */
void applyFilters(State &s)
{
	// Bus 0 — Chassis (X179 pins 13-14): dynamic filter set based on enabled features
	if (s.rawCanListen)
	{
		driverSetBusFilters(0, nullptr, 0);
	}
	else
	{
		uint32_t ids[14];
		uint8_t count = 0;
		ids[count++] = CAN_ID_DAS_CONTROL;
		ids[count++] = CAN_ID_DAS_STEERING_CTRL;
		ids[count++] = CAN_ID_DAS_STATUS2;
		ids[count++] = CAN_ID_UI_GPS_SPEED;
		ids[count++] = CAN_ID_WHEEL_SPEED;
		switch (s.variant)
		{
		case HW4:
			if (s.isaChimeSuppress)
				ids[count++] = CAN_ID_ISA_SPEED;
			if (s.fsdEnabled)
				ids[count++] = CAN_ID_FOLLOW_DIST;
			if (s.fsdEnabled || nagModeUsesBit19(s.nagMode))
				ids[count++] = CAN_ID_FSD_MUX;
			break;
		case HW3:
			if (s.fsdEnabled)
				ids[count++] = CAN_ID_FOLLOW_DIST;
			if (s.fsdEnabled || nagModeUsesBit19(s.nagMode))
				ids[count++] = CAN_ID_FSD_MUX;
			break;
		case LEGACY:
			if (s.fsdEnabled)
				ids[count++] = CAN_ID_LEGACY_STALK;
			if (s.fsdEnabled || nagModeUsesBit19(s.nagMode))
				ids[count++] = CAN_ID_LEGACY_FSD_MUX;
			break;
		}
		// Include discriminating frames for variant auto-detection when 0x398 has not been seen
		if (s.variantAutoDetect && !s.hwAutoDetected)
		{
			bool isaAlready = (s.variant == HW4 && s.isaChimeSuppress);
			bool legacyMuxAlready =
				(s.variant == LEGACY && (s.fsdEnabled || nagModeUsesBit19(s.nagMode)));
			if (!isaAlready)
				ids[count++] = CAN_ID_ISA_SPEED; // HW4-only discriminator
			if (!legacyMuxAlready)
				ids[count++] = CAN_ID_LEGACY_FSD_MUX; // Legacy-only discriminator
		}
		if (count > 0)
		{
			driverSetBusFilters(BUS_CHASSIS, ids, count);
		}
		else
		{
			static const uint32_t none[] = {0x000};
			driverSetBusFilters(BUS_CHASSIS, none, 1);
		}
	}

	// Bus 1 — Vehicle (X179 pins 9-10): vehicle control, BMS, and telemetry frames
	if (s.rawCanListen)
	{
		driverSetBusFilters(BUS_VEHICLE, nullptr, 0);
	}
	else
	{
		static const uint32_t vehIds[] = {CAN_ID_PRECONDITION,
										  CAN_ID_BMS_HV_BUS,
										  CAN_ID_UI_VEHICLE_CTRL,
										  CAN_ID_BMS_SOC,
										  CAN_ID_CLIMATE,
										  CAN_ID_BMS_THERMAL,
										  CAN_ID_TRACK_MODE,
										  CAN_ID_GTW_CAR_STATE,
										  CAN_ID_CHARGE,
										  CAN_ID_DRIVE_CONFIG,
										  CAN_ID_BMS_ENERGY,
										  CAN_ID_EPAS_TORQUE,
										  CAN_ID_GTW_CAR_CFG,
										  CAN_ID_DAS_STATUS,
										  CAN_ID_GTW_CONFIG_ETH,
										  CAN_ID_BLIND_SPOT,
										  CAN_ID_VCLEFT_DOOR_STATUS,
										  CAN_ID_VCRIGHT_DOOR_STATUS,
										  CAN_ID_VCFRONT_STATUS,
										  CAN_ID_VCFRONT_VEH_STATUS,
										  CAN_ID_BMS_ENERGY_ST,
										  CAN_ID_BMS_MIN_MAX,
										  CAN_ID_BMS_POWER_AV,
										  CAN_ID_BMS_STATUS,
										  CAN_ID_BMS_DRIVE_LIM,
										  CAN_ID_BMS_KWH_CNT,
										  CAN_ID_BMS_KWH_MUX,
										  CAN_ID_BMS_BRICK_V,
										  CAN_ID_TPMS,
										  CAN_ID_DI_STEER,
										  CAN_ID_VCFRONT_LIGHTS,
										  CAN_ID_VEHICLE_SPEED,
										  CAN_ID_DI_STATE,
										  CAN_ID_STEERING_ANGLE,
										  CAN_ID_REAR_MOTOR,
										  CAN_ID_FRONT_MOTOR,
										  CAN_ID_SEATBELT_STATUS,
										  CAN_ID_GTW_VERSION,
										  CAN_ID_DAS_AP_CONFIG,
										  CAN_ID_REAR_INV_TEMPS,
										  CAN_ID_FRONT_INV_TEMPS};
		driverSetBusFilters(BUS_VEHICLE, vehIds, sizeof(vehIds) / sizeof(vehIds[0]));
	}

	// Bus 2 — Body (X179 pins 2-3): body control frames
	if (s.rawCanListen)
	{
		driverSetBusFilters(BUS_BODY, nullptr, 0);
	}
	else
	{
		static const uint32_t bodyIds[] = {CAN_ID_WINDOW_VENT, CAN_ID_SENTRY, CAN_ID_TRUNK_CTRL};
		driverSetBusFilters(BUS_BODY, bodyIds, 3);
	}
}
