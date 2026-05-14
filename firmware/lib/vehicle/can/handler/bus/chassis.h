#pragma once

/**
 * @file firmware/lib/vehicle/can/handler/bus/chassis.h
 * @brief Bus 0 (Chassis) frame handler for FSD variant-specific processing
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "core/can/bus.h"
#include "core/log/ring.h"
#include "vehicle/can/ids.h"
#include "vehicle/can/feature/das/das_drive.h"
#include "vehicle/can/feature/drive/drive_context.h"
#include "vehicle/can/feature/telemetry/wheel_speeds.h"
#include "handler/helpers.h"
#include "handler/filters.h"
#include "handler/variant/hw4.h"
#include "handler/variant/hw3.h"
#include "handler/variant/legacy.h"

/**
 * @brief Handle an incoming frame on the chassis bus (X179 pins 13-14).
 *
 * Decodes cruise/ACC/GPS/wheel-speed observables, runs P2-06 fallback
 * variant inference when the primary detection frame (0x398) is absent,
 * then dispatches to the per-variant FSD handler.
 *
 * @param f Reference to the received CAN frame.
 * @param s Reference to the shared vehicle state.
 */
inline void handleChassisBus(Frame &f, State &s)
{
	if (f.id == CAN_ID_DAS_CONTROL && f.dlc >= 2)
	{
		s.cruiseSetSpeedKph = decodeCruiseSetSpeedKph(f);
		s.maxSpeedKph = s.cruiseSetSpeedKph;
		return;
	}
	if (f.id == CAN_ID_DAS_STEERING_CTRL && f.dlc >= 3)
	{
		return;
	}
	if (f.id == CAN_ID_DAS_STATUS2 && f.dlc >= 2)
	{
		s.accSpeedLimitKph = decodeAccSpeedLimitKph(f);
		if (s.accSpeedLimitKph > s.maxSpeedKph)
			s.maxSpeedKph = s.accSpeedLimitKph;
		return;
	}
	if (f.id == CAN_ID_UI_GPS_SPEED && f.dlc >= 7)
	{
		s.mapSpeedLimitKph = decodeMapSpeedLimitKph(f);
		if (s.mapSpeedLimitKph > s.maxSpeedKph)
			s.maxSpeedKph = s.mapSpeedLimitKph;
		return;
	}

	// All four wheel speeds packed in one 8-byte frame (read-only telemetry)
	if (f.id == CAN_ID_WHEEL_SPEED && f.dlc >= 7)
	{
		s.wheelSpeedFL = decodeWheelSpeedFL(f.data);
		s.wheelSpeedFR = decodeWheelSpeedFR(f.data);
		s.wheelSpeedRL = decodeWheelSpeedRL(f.data);
		s.wheelSpeedRR = decodeWheelSpeedRR(f.data);
		s.hasWheelSpeeds = true;
		return;
	}

	// P2-06: Fallback variant inference from distinctive frame presence
	if (s.variantAutoDetect && !s.hwAutoDetected)
	{
		if (f.id == CAN_ID_ISA_SPEED && s.variant != HW4)
		{
			// ISA speed chime (0x399) is HW4-exclusive; infer variant from its presence
			bool fromLegacy = (s.variant == LEGACY);
			s.variant = HW4;
			if (fromLegacy)
				s.speedProfile = 1; // P2-07: clear stale legacy stalk value
			applyFilters(s);
			resetHandlerLogFlags();
			sendLog(F("Fallback: HW4 inferred from ISA_SPEED"));
		}
		else if (f.id == CAN_ID_LEGACY_FSD_MUX && s.variant != LEGACY)
		{
			// Legacy FSD mux (0x3EE) is Legacy-exclusive; infer variant
			s.variant = LEGACY;
			applyFilters(s);
			resetHandlerLogFlags();
			sendLog(F("Fallback: LEGACY inferred from legacy mux frame"));
		}
	}

	// Dispatch to the variant-specific FSD handler
	switch (s.variant)
	{
	case HW4:
		handleHW4(f, s);
		break;
	case HW3:
		handleHW3(f, s);
		break;
	case LEGACY:
		handleLegacy(f, s);
		break;
	}
}
