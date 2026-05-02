#pragma once
#include "core/forward.h"

// ── Ban Detection ─────────────────────────────────────────────────────────────
// Monitors GTW autopilot tier (from 0x7FF mux=2) for unexpected drops.
// When the tier drops from SELF_DRIVING (3) to BASIC (4) or NONE (0),
// it indicates a VIN-level ban has been applied by Tesla's servers.
//
// Source: hypery11/flipper-tesla-fsd issue #18 (April 2026 ban wave)
//
// Tier values:
//   0 = NONE           (no autopilot)
//   1 = HIGHWAY        (highway AP)
//   2 = ENHANCED       (Enhanced AP / EAP)
//   3 = SELF_DRIVING   (Full Self-Driving)
//   4 = BASIC          (basic AP — downgraded)
//  -1 = not yet read

static const char *apTierName(int8_t tier)
{
	switch (tier)
	{
	case 0:
		return "NONE";
	case 1:
		return "HIGHWAY";
	case 2:
		return "ENHANCED";
	case 3:
		return "SELF_DRIVING";
	case 4:
		return "BASIC";
	default:
		return "UNKNOWN";
	}
}

// Call after updating gtwAutopilotTier. Returns true if a ban-like
// tier drop was detected (logs a warning via sendLog).
inline bool checkBanDetection(int8_t prevTier, int8_t newTier, State &s)
{
	// Only alert on actual tier drops where we had a known good state
	if (prevTier < 0 || newTier < 0)
		return false;
	if (newTier == prevTier)
		return false;

	// Detect downgrade from SELF_DRIVING to BASIC or NONE
	bool isBanDrop = (prevTier == 3) && (newTier == 0 || newTier == 4);
	// Also detect any drop from ENHANCED/SELF_DRIVING to lower
	bool isTierDrop = (prevTier >= 2) && (newTier < prevTier && newTier != 3);

	if (isBanDrop || isTierDrop)
	{
		s.banDetectionCount++;
		s.banThreatLevel = isBanDrop ? 5 : 3;
		s.banThreatMs = millis();
		return true;
	}
	return false;
}
