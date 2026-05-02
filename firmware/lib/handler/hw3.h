#pragma once
#include "core/forward.h"
#include "infra/can.h"
#include "feature/profile.h"
#include "feature/offsets.h"
#include "feature/region.h"

// Forward declarations (defined by platform-specific serial)
void sendLog(const char *msg);
void sendLog(const __FlashStringHelper *msg);

static bool hw3LoggedFSD = false;
static bool hw3LoggedNag = false;
static bool hw3LoggedOffset = false;

void resetHW3LogFlags()
{
	hw3LoggedFSD = false;
	hw3LoggedNag = false;
	hw3LoggedOffset = false;
}

// ── HW3 Handler ──────────────────────────────────────────────────────────────
void handleHW3(Frame &f, State &s)
{
	const bool apGateOpen = s.apGateOpen();

	// OTA safety: pass-through unmodified frames during OTA update
	if (s.txPaused)
	{
		driverSend(f, 0);
		return;
	}

	// Follow distance → profile mapping (auto-track from stalk unless pinned)
	if (f.id == CAN_ID_FOLLOW_DIST)
	{
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
			setBit(f, 39, true); // UI_fsdContinueOnGreenWithCIPV: continue on green with lead car (ev-open-can-tools-plugins)
			setBit(f, 46, true);
			setSpeedProfileV12V13(f, s.speedProfile);
			driverSend(f, 0);
			if (!hw3LoggedFSD)
			{
				sendLog(F("HW3: FSD mod active on CAN"));
				hw3LoggedFSD = true;
			}
			return;
		}

		if (mux == 1 && s.nagSuppress && apGateOpen)
		{
			setBit(f, 19, false);
			setBit(f, 47, true); // summon EU unlock bit — matches HW4 and All HW/summon-eu-unlock.json
			// Enhanced Autopilot: set bit 46 on mux=1 to unlock EAP/Summon
			// Source: ev-open-can-tools HW3Handler enhancedAutopilotRuntime + hypery11
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
			if (!hw3LoggedNag)
			{
				sendLog(F("HW3: Nag suppressed on CAN"));
				hw3LoggedNag = true;
			}
			return;
		}

		if (mux == 2 && s.fsdEnabled && apGateOpen)
		{
			writeHW3SpeedOffset(f, s.speedOffset);
			driverSend(f, 0);
			if (!hw3LoggedOffset)
			{
				sendLog(F("HW3: Speed offset applied"));
				hw3LoggedOffset = true;
			}
			return;
		}
	}
}
