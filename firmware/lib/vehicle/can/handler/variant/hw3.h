#pragma once
#include "core/forward.h"
#include "vehicle/can/ids.h"
#include "feature/profile.h"
#include "feature/offsets.h"
#include "handler/variant/nag.h" // applyNagSuppressBits + ONCE_LOG + region

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
		// AP-First mode (2026.14.x): suppress injection until AP is already active.
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
			setBit(f, 39, true); // UI_fsdContinueOnGreenWithCIPV: continue on green with lead car (ev-open-can-tools-plugins)
			setBit(f, 46, true);
			setSpeedProfileV12V13(f, s.speedProfile);
			driverSend(f, 0);
			ONCE_LOG(hw3LoggedFSD, F("HW3: FSD mod active on CAN"));
			return;
		}

		if (mux == 1 && s.nagSuppress && apGateOpen)
		{
			applyNagSuppressBits(f, s);
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
