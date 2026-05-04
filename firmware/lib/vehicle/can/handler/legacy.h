#pragma once
#include "core/forward.h"
#include "vehicle/can/ids.h"
#include "feature/profile.h"
#include "feature/offsets.h"

// Forward declarations (defined by platform-specific serial)
void sendLog(const char *msg);
void sendLog(const __FlashStringHelper *msg);

static bool legacyLoggedFSD = false;
static bool legacyLoggedNag = false;

void resetLegacyLogFlags()
{
	legacyLoggedFSD = false;
	legacyLoggedNag = false;
}

// ── Legacy Handler ───────────────────────────────────────────────────────────
void handleLegacy(Frame &f, State &s)
{
	// OTA safety: pass-through unmodified frames during OTA update
	if (s.txPaused)
	{
		driverSend(f, 0);
		return;
	}

	// Legacy stalk position → profile mapping (auto-track unless pinned)
	if (f.id == CAN_ID_LEGACY_STALK)
	{
		if (!s.profileOverride && f.dlc >= 2)
		{
			uint8_t stalk = f.data[1] >> 5;
			if (stalk <= 1)
				s.speedProfile = 2;
			else if (stalk == 2)
				s.speedProfile = 1;
			else
				s.speedProfile = 0;
		}
		return;
	}

	// FSD mux handling
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
			setBit(f, 46, true);
			setSpeedProfileV12V13(f, s.speedProfile);
			driverSend(f, 0);
			if (!legacyLoggedFSD)
			{
				sendLog(F("Legacy: FSD mod active on CAN"));
				legacyLoggedFSD = true;
			}
			return;
		}

		if (mux == 1 && s.nagSuppress && apGateOpen)
		{
			setBit(f, 19, false);
			driverSend(f, 0);
			s.canDiag.eapModCount++;
			if (!legacyLoggedNag)
			{
				sendLog(F("Legacy: Nag suppressed on CAN"));
				legacyLoggedNag = true;
			}
			return;
		}
	}
}
