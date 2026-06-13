#pragma once

/**
 * @file firmware/lib/transport/can/handler/variant/hw4.h
 * @brief HW4 (Autopilot 4.0) variant CAN frame handler for FSD, nag suppression, and ISA chime
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "vehicle/can/ids.h"
#include "bits.h"
#include "feature/fsd/profile.h"
#include "feature/fsd/isa_chime.h"
#include "feature/fsd/region.h"

static bool hw4LoggedFSD = false;
static bool hw4LoggedNag = false;
static bool hw4LoggedISA = false;

/**
 * @brief Reset one-shot log flags for the HW4 handler
 */
void resetHW4LogFlags()
{
	hw4LoggedFSD = false;
	hw4LoggedNag = false;
	hw4LoggedISA = false;
}

/**
 * @brief Apply nag suppression bit pattern on mux=1 frames for HW4
 *
 * Duplicated locally in hw3.h and hw4.h to avoid cross-include between handler
 * and feature layers. Both handlers are the only callers.
 *
 * @param f CAN frame to modify (mux=1 of FSD mux message)
 * @param s Global vehicle state
 */
inline void applyHW4NagSuppressBits(Frame &f, State &s)
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
 * @brief Main CAN frame handler for HW4 (Autopilot 4.0) vehicles
 *
 * Processes ISA speed chime suppression, follow-distance mapping, FSD mux
 * injection, and nag suppression. HW4 adds ISA checksum recomputation and
 * emergency vehicle detection (EVD) compared to HW3.
 *
 * @param f Incoming CAN frame to inspect and potentially modify
 * @param s Global vehicle state containing feature flags and diagnostics
 */
void handleHW4(Frame &f, State &s)
{
	const bool apGateOpen = s.apGateOpen();

	// ISA speed chime suppression (HW4-specific)
	if (f.id == CAN_ID_ISA_SPEED && s.isaChimeSuppress && apGateOpen)
	{
		if (f.dlc >= 8)
		{
			f.data[ISA_CHIME_SUPPRESS_BYTE] |= ISA_CHIME_SUPPRESS_MASK; // Set chime-suppress flag
			f.data[7] = computeHW4IsaChecksum(f);						 // Recompute trailing checksum
			driverSend(f, BUS_CHASSIS);
			ONCE_LOG(hw4LoggedISA, F("HW4: ISA chime suppressed"));
			return;
		}
		return;
	}

	if (f.id == CAN_ID_FOLLOW_DIST)
	{
		// Map follow-distance stalk position to speed profile unless pinned
		if (!s.profileOverride)
		{
			int profile = mapHW4FollowDistToProfile(readFollowDistance(f));
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
		bool fsdAllowed = s.fsdEnabled && (s.fsdForceEnabled || isFSDSelectedInUI(f));

		if (mux == 0 && fsdAllowed && apGateOpen)
		{
			setBit(f, FSD_BIT_AP_ACTIVE, true);
			setBit(f, FSD_BIT_CONTINUE_ON_GREEN, true); // UI_fsdContinueOnGreenWithCIPV
			setBit(f, FSD_BIT_EAP, true);
			setBit(f, FSD_BIT_DAS_DEV, true);
			// Emergency Vehicle Detection: bit 59 allows AP to respond to EVs
			if (s.evdEnabled)
				setBit(f, FSD_BIT_EVD, true);
			driverSend(f, BUS_CHASSIS);
			ONCE_LOG(hw4LoggedFSD, F("HW4: FSD mod active on CAN"));
			return;
		}

		if (mux == 1 && nagModeUsesBit19(s.nagMode) && apGateOpen)
		{
			applyHW4NagSuppressBits(f, s);
			ONCE_LOG(hw4LoggedNag, F("HW4: Nag suppressed on CAN"));
			return;
		}

		if (mux == 2 && s.fsdEnabled && apGateOpen)
		{
			writeHW4SpeedProfile(f, s.speedProfile);
			if (s.speedOffset > 0)
			{
				// Encode speed offset into bits [5:0] of byte 1, preserving upper 2 bits
				f.data[1] = (f.data[1] & HW4_OFFSET_PRESERVE_MASK) | (s.speedOffset & HW4_OFFSET_FIELD_MASK);
			}
			driverSend(f, BUS_CHASSIS);
			return;
		}
	}
}
