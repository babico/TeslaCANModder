#pragma once
#include "core/forward.h"
#include "vehicle/can/ids.h"
#include "feature/profile.h"
#include "feature/isa_chime.h"
#include "feature/region.h"

static bool hw4LoggedFSD = false;
static bool hw4LoggedNag = false;
static bool hw4LoggedISA = false;

void resetHW4LogFlags()
{
	hw4LoggedFSD = false;
	hw4LoggedNag = false;
	hw4LoggedISA = false;
}

// ── HW4 Handler ──────────────────────────────────────────────────────────────
void handleHW4(Frame &f, State &s)
{
	const bool apGateOpen = s.apGateOpen();

	// OTA safety: pass-through unmodified frames during OTA update
	if (s.txPaused)
	{
		driverSend(f, 0);
		return;
	}

	// ISA speed chime suppression
	if (f.id == CAN_ID_ISA_SPEED && s.isaChimeSuppress && apGateOpen)
	{
		if (f.dlc >= 8)
		{
			f.data[1] |= 0x20;
			f.data[7] = computeHW4IsaChecksum(f);
			driverSend(f, 0);
			if (!hw4LoggedISA)
			{
				sendLog(F("HW4: ISA chime suppressed"));
				hw4LoggedISA = true;
			}
			return;
		}
		return;
	}

	// Follow distance → profile mapping (auto-track from stalk unless pinned)
	if (f.id == CAN_ID_FOLLOW_DIST)
	{
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
				setBit(f, 41, false); // UI_drivingSide: 0=LHD
				fdModified = true;
			}
			if (s.assistNavEnable)
			{
				setBit(f, 13, true);  // UI_driveOnMapsEnable
				setBit(f, 48, true);  // UI_hasDriveOnNav
				setBit(f, 49, true);  // UI_followNavRouteEnable
				fdModified = true;
			}
			if (s.assistHandsOff)
			{
				setBit(f, 14, true);  // UI_handsOnRequirementDisable
				fdModified = true;
			}
			if (s.assistDevMode)
			{
				setBit(f, 5, true);   // UI_dasDeveloper
				fdModified = true;
			}
			if (s.assistTelemetryOff)
			{
				setBit(f, 43, false); // UI_enableTripTelemetry = 0
				fdModified = true;
			}
			if (fdModified)
				driverSend(f, BUS_CHASSIS);
		}
		return;
	}

	// FSD mux handling
	if (f.id == CAN_ID_FSD_MUX)
	{
		// AP-First mode (2026.14.x): suppress injection until AP is already active.
		if (s.apFirstEnabled && s.dasApState < 2)
			return;
		uint8_t mux = readMuxID(f);
		bool fsdAllowed = s.fsdEnabled && (s.fsdForceEnabled || isFSDSelectedInUI(f));

		if (mux == 0 && fsdAllowed && apGateOpen)
		{
			setBit(f, 38, true);
			setBit(f, 39, true); // UI_fsdContinueOnGreenWithCIPV: continue on green with lead car (ev-open-can-tools-plugins)
			setBit(f, 46, true);
			setBit(f, 60, true);
			// Emergency Vehicle Detection: allow AP to respond to emergency vehicles
			// Source: hypery11/flipper-tesla-fsd fsd_handler.c (bit 59 on mux=0)
			if (s.evdEnabled)
				setBit(f, 59, true);
			driverSend(f, 0);
			if (!hw4LoggedFSD)
			{
				sendLog(F("HW4: FSD mod active on CAN"));
				hw4LoggedFSD = true;
			}
			return;
		}

		if (mux == 1 && s.nagSuppress && apGateOpen)
		{
			setBit(f, 19, false);
			setBit(f, 47, true);
			// Enhanced Autopilot: set bit 46 on mux=1 to unlock EAP/Summon
			// Source: ev-open-can-tools hw4OffsetRuntime + hypery11 enhanced_autopilot
			if (s.enhancedAutopilot)
				setBit(f, 46, true);
			// Lane graph visualization: bit 45 on mux=1
			// Source: hypery11/flipper-tesla-fsd fsd_handler.c (assist_show_lane_graph)
			if (s.laneGraphEnable)
				setBit(f, 45, true);
			// ECE R79 bypass: clear EU speed restriction bit for European vehicles
			if (s.eceR79Bypass && s.hasRegion && isEuropeanMarket(s.regionCode))
			{
				applyEceR79Bypass(f);
			}
			driverSend(f, 0);
			s.canDiag.eapModCount++;
			if (!hw4LoggedNag)
			{
				sendLog(F("HW4: Nag suppressed on CAN"));
				hw4LoggedNag = true;
			}
			return;
		}

		if (mux == 2 && s.fsdEnabled && apGateOpen)
		{
			writeHW4SpeedProfile(f, s.speedProfile);
			if (s.speedOffset > 0)
			{
				f.data[1] = (f.data[1] & 0xC0) | (s.speedOffset & 0x3F);
			}
			driverSend(f, 0);
			return;
		}
	}
}
