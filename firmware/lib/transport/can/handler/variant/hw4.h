#pragma once

/**
 * @file firmware/lib/transport/can/handler/variant/hw4.h
 * @brief HW4 (Autopilot 4.0) variant CAN frame handler for FSD, nag suppression, and ISA chime
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "vehicle/can/ids.h"
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
	setBit(f, 19, false); // ECE R79 hands-on nag disable
	setBit(f, 47, true);  // Summon EU unlock
	if (s.enhancedAutopilot)
		setBit(f, 46, true); // EAP/Summon unlock on mux=1
	if (s.laneGraphEnable)
		setBit(f, 45, true); // Lane graph visualization enable
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
			f.data[1] |= 0x20;					  // Set chime-suppress flag in byte 1
			f.data[7] = computeHW4IsaChecksum(f); // Recompute trailing checksum
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
		bool fsdAllowed = s.fsdEnabled && (s.fsdForceEnabled || isFSDSelectedInUI(f));

		if (mux == 0 && fsdAllowed && apGateOpen)
		{
			setBit(f, 38, true);
			setBit(f, 39, true); // UI_fsdContinueOnGreenWithCIPV
			setBit(f, 46, true);
			setBit(f, 60, true);
			// Emergency Vehicle Detection: bit 59 allows AP to respond to EVs
			if (s.evdEnabled)
				setBit(f, 59, true);
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
				f.data[1] = (f.data[1] & 0xC0) | (s.speedOffset & 0x3F);
			}
			driverSend(f, BUS_CHASSIS);
			return;
		}
	}
}
