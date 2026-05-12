#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/ban_detect.h
 * @brief VIN-level ban detection via GTW autopilot tier monitoring
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"

/**
 * @brief Return a human-readable name for an autopilot tier value.
 *
 * Tier values from GTW frame 0x7FF mux=2:
 *   0=NONE, 1=HIGHWAY, 2=ENHANCED, 3=SELF_DRIVING, 4=BASIC (downgraded).
 *
 * @param tier Autopilot tier value (-1 if not yet read).
 * @return Static string with the tier name.
 */
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

/**
 * @brief Check for a ban-like autopilot tier drop and update threat state.
 *
 * Detects downgrades from SELF_DRIVING (3) to BASIC (4) or NONE (0), which
 * indicate a VIN-level ban applied by Tesla's servers. Also flags any drop
 * from ENHANCED or higher to a lower tier.
 *
 * @param prevTier Previous autopilot tier value.
 * @param newTier Newly observed autopilot tier value.
 * @param s Device state to update (banDetectionCount, banThreatLevel, banThreatMs).
 * @return True if a ban-like tier drop was detected.
 */
inline bool checkBanDetection(int8_t prevTier, int8_t newTier, State &s)
{
	// Only alert on actual tier drops where we had a known good state
	if (prevTier < 0 || newTier < 0)
		return false;
	if (newTier == prevTier)
		return false;

	// Direct ban indicator: SELF_DRIVING -> BASIC or NONE
	bool isBanDrop = (prevTier == 3) && (newTier == 0 || newTier == 4);
	// General downgrade: ENHANCED+ dropping to a lower tier (excluding lateral moves to SELF_DRIVING)
	bool isTierDrop = (prevTier >= 2) && (newTier < prevTier && newTier != 3);

	if (isBanDrop || isTierDrop)
	{
		s.banDetectionCount++;
		s.banThreatLevel = isBanDrop ? 5 : 3; // Max threat for direct ban, moderate for general drop
		s.banThreatMs = millis();
		return true;
	}
	return false;
}
