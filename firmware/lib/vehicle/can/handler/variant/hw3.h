#pragma once

/**
 * @file firmware/lib/vehicle/can/handler/variant/hw3.h
 * @brief HW3 (Autopilot 3.0) variant CAN frame handler for FSD, nag suppression, and speed offset
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "vehicle/can/ids.h"
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
	setBit(f, 19, false); // ECE R79 hands-on nag disable
	setBit(f, 47, true);  // Summon EU unlock
	if (s.enhancedAutopilot)
		setBit(f, 46, true); // EAP/Summon unlock on mux=1
	if (s.laneGraphEnable)
		setBit(f, 45, true); // Lane graph visualization enable
	// Clear EU speed restriction bit for European-market vehicles
	if (s.eceR79Bypass && s.hasRegion && isEuropeanMarket(s.regionCode))
		applyEceR79Bypass(f);
	driverSend(f, 0);
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
				setBit(f, 41, false); // UI_drivingSide: 0 = LHD
				fdModified = true;
			}
			if (s.assistNavEnable)
			{
				setBit(f, 13, true); // UI_driveOnMapsEnable
				setBit(f, 48, true); // UI_hasDriveOnNav
				setBit(f, 49, true); // UI_followNavRouteEnable
				fdModified = true;
			}
			if (s.assistHandsOff)
			{
				setBit(f, 14, true); // UI_handsOnRequirementDisable
				fdModified = true;
			}
			if (s.assistDevMode)
			{
				setBit(f, 5, true); // UI_dasDeveloper
				fdModified = true;
			}
			if (s.assistTelemetryOff)
			{
				setBit(f, 43, false); // UI_enableTripTelemetry disable
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

			setBit(f, 38, true);
			setBit(f, 39, true); // UI_fsdContinueOnGreenWithCIPV
			setBit(f, 46, true);
			setSpeedProfileV12V13(f, s.speedProfile);
			driverSend(f, 0);
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
			driverSend(f, 0);
			ONCE_LOG(hw3LoggedOffset, F("HW3: Speed offset applied"));
			return;
		}
	}
}
