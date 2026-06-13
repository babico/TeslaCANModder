#pragma once

/**
 * @file firmware/lib/vehicle/can/handler/variant/hw3.h
 * @brief HW3 (Autopilot 3.0) variant CAN frame handler for FSD, nag suppression, and speed offset
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "vehicle/can/ids.h"
#include "bits.h"
#include "feature/fsd/profile.h"
#include "feature/fsd/offsets.h"
#include "feature/fsd/region.h"

static bool hw3LoggedFSD = false;
static bool hw3LoggedNag = false;
static bool hw3LoggedOffset = false;

/**
 * @brief Reset one-shot log flags for the HW3 handler
 */
void resetHW3LogFlags()
{
	hw3LoggedFSD = false;
	hw3LoggedNag = false;
	hw3LoggedOffset = false;
}

/**
 * @brief Apply nag suppression bit pattern on mux=1 frames for HW3
 *
 * Duplicated locally in hw3.h and hw4.h to avoid cross-include between handler
 * and feature layers. Both handlers are the only callers.
 *
 * @param f CAN frame to modify (mux=1 of FSD mux message)
 * @param s Global vehicle state
 */
inline void applyHW3NagSuppressBits(Frame &f, State &s)
{
	setBit(f, NAG_BIT_HANDS_ON_REQUIREMENT, false); // ECE R79 hands-on nag disable
	setBit(f, FSD_BIT_NAG_ORGANIC, true);			 // Summon EU unlock
	if (s.enhancedAutopilot)
		setBit(f, FSD_BIT_EAP, true); // EAP/Summon unlock on mux=1
	if (s.laneGraphEnable)
		setBit(f, UI_BIT_LANE_GRAPH, true); // Lane graph visualization enable
	// Clear EU speed restriction bit for European-market vehicles
	if (s.eceR79Bypass && s.hasRegion && isEuropeanMarket(s.regionCode))
		applyEceR79Bypass(f);
	driverSend(f, BUS_CHASSIS);
	s.canDiag.eapModCount++;
}

/**
 * @brief Main CAN frame handler for HW3 (Autopilot 3.0) vehicles
 *
 * Processes follow-distance, FSD mux, and nag suppression frames. Applies
 * speed profile mapping, UI bit injection, and speed offset writes depending
 * on the mux index and current feature state.
 *
 * @param f Incoming CAN frame to inspect and potentially modify
 * @param s Global vehicle state containing feature flags and diagnostics
 */
void handleHW3(Frame &f, State &s)
{
	const bool apGateOpen = s.apGateOpen();

	if (f.id == CAN_ID_FOLLOW_DIST)
	{
		// Map follow-distance stalk position to speed profile unless pinned
		if (!s.profileOverride)
		{
			int profile = mapHW3FollowDistToProfile(readFollowDistance(f));
			if (profile >= 0)
				s.speedProfile = profile;
		}
		if (apGateOpen)
		{
			bool fdModified = false;
			if (s.lhdEnabled)
			{
				setBit(f, FSD_BIT_DRIVING_SIDE, false); // UI_drivingSide: 0 = LHD
				fdModified = true;
			}
			if (s.assistNavEnable)
			{
				setBit(f, UI_BIT_DRIVE_ON_MAPS, true); // UI_driveOnMapsEnable
				setBit(f, UI_BIT_HAS_DRIVE_ON_NAV, true); // UI_hasDriveOnNav
				setBit(f, UI_BIT_FOLLOW_NAV_ROUTE, true); // UI_followNavRouteEnable
				fdModified = true;
			}
			if (s.assistHandsOff)
			{
				setBit(f, UI_BIT_HANDS_ON_REQ_DISABLE, true); // UI_handsOnRequirementDisable
				fdModified = true;
			}
			if (s.assistDevMode)
			{
				setBit(f, UI_BIT_DAS_DEVELOPER, true); // UI_dasDeveloper
				fdModified = true;
			}
			if (s.assistTelemetryOff)
			{
				setBit(f, UI_BIT_TRIP_TELEMETRY, false); // UI_enableTripTelemetry disable
				fdModified = true;
			}
			if (fdModified)
				driverSend(f, BUS_CHASSIS);
		}
		return;
	}

	if (f.id == CAN_ID_FSD_MUX)
	{
		// AP-First mode: suppress injection until Autopilot state >= 2
		if (s.apFirstEnabled && s.dasApState < 2)
			return;
		uint8_t mux = readMuxID(f);
		bool fsdUI = isFSDSelectedInUI(f);
		bool fsdAllowed = s.fsdEnabled && (s.fsdForceEnabled || fsdUI);

		if (mux == 0 && fsdAllowed && apGateOpen)
		{
			int steps = readHW3UiOffsetSteps(f);
			if (!s.profileOverride && steps >= 0 && steps <= 2)
				s.speedProfile = steps;
			if (!s.offsetOverride)
				s.speedOffset = calculateHW3SpeedOffset(steps);

			setBit(f, FSD_BIT_AP_ACTIVE, true);
			setBit(f, FSD_BIT_CONTINUE_ON_GREEN, true); // UI_fsdContinueOnGreenWithCIPV
			setBit(f, FSD_BIT_EAP, true);
			setSpeedProfileV12V13(f, s.speedProfile);
			driverSend(f, BUS_CHASSIS);
			ONCE_LOG(hw3LoggedFSD, F("HW3: FSD mod active on CAN"));
			return;
		}

		if (mux == 1 && nagModeUsesBit19(s.nagMode) && apGateOpen)
		{
			applyHW3NagSuppressBits(f, s);
			ONCE_LOG(hw3LoggedNag, F("HW3: Nag suppressed on CAN"));
			return;
		}

		if (mux == 2 && s.fsdEnabled && apGateOpen)
		{
			writeHW3SpeedOffset(f, s.speedOffset);
			driverSend(f, BUS_CHASSIS);
			ONCE_LOG(hw3LoggedOffset, F("HW3: Speed offset applied"));
			return;
		}
	}
}
