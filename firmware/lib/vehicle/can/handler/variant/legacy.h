#pragma once

/**
 * @file firmware/lib/vehicle/can/handler/variant/legacy.h
 * @brief Legacy (pre-HW3) variant CAN frame handler for FSD and nag suppression
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "vehicle/can/ids.h"
#include "bits.h"
#include "feature/fsd/profile.h"
#include "feature/fsd/offsets.h"

static bool legacyLoggedFSD = false;
static bool legacyLoggedNag = false;

/**
 * @brief Reset one-shot log flags for the legacy handler
 */
void resetLegacyLogFlags()
{
	legacyLoggedFSD = false;
	legacyLoggedNag = false;
}

/**
 * @brief Main CAN frame handler for legacy (pre-HW3) vehicles
 *
 * Processes stalk-based profile mapping and FSD mux frames. The legacy variant
 * uses a simpler nag suppression (bit 19 only) and does not support ISA chime,
 * EVD, or ECE R79 bypass features.
 *
 * @param f Incoming CAN frame to inspect and potentially modify
 * @param s Global vehicle state containing feature flags and diagnostics
 */
void handleLegacy(Frame &f, State &s)
{
	if (f.id == CAN_ID_LEGACY_STALK)
	{
		// Map stalk position (upper 3 bits of byte 1) to speed profile
		if (!s.profileOverride && f.dlc >= 2)
		{
			uint8_t stalk = f.data[1] >> 5; // Extract 3-bit stalk position
			if (stalk <= 1)
				s.speedProfile = 2;
			else if (stalk == 2)
				s.speedProfile = 1;
			else
				s.speedProfile = 0;
		}
		return;
	}

	if (f.id == CAN_ID_LEGACY_FSD_MUX)
	{
		const bool apGateOpen = s.apGateOpen();
		uint8_t mux = readMuxID(f);
		bool fsdUI = isFSDSelectedInUI(f);
		bool fsdAllowed = s.fsdEnabled && (s.fsdForceEnabled || fsdUI);

		if (mux == 0 && fsdAllowed && apGateOpen)
		{
			int steps = readHW3UiOffsetSteps(f);
			if (!s.profileOverride && steps >= 0 && steps <= 2)
				s.speedProfile = steps;
			setBit(f, FSD_BIT_EAP, true);
			setSpeedProfileV12V13(f, s.speedProfile);
			driverSend(f, BUS_CHASSIS);
			ONCE_LOG(legacyLoggedFSD, F("Legacy: FSD mod active on CAN"));
			return;
		}

		if (mux == 1 && nagModeUsesBit19(s.nagMode) && apGateOpen)
		{
			setBit(f, NAG_BIT_HANDS_ON_REQUIREMENT, false); // Clear ECE R79 hands-on nag bit
			driverSend(f, BUS_CHASSIS);
			s.canDiag.eapModCount++;
			ONCE_LOG(legacyLoggedNag, F("Legacy: Nag suppressed on CAN"));
			return;
		}
	}
}
