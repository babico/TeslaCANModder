#pragma once
// ── Shared nag-suppression bit manipulation ──────────────────────────────────
// Applies the mux=1 bit-pattern that is identical between HW3 and HW4.
// Call this inside the `if (mux == 1 && s.nagSuppress && apGateOpen)` block,
// then use ONCE_LOG for the variant-specific one-shot log message.

#include "core/forward.h"
#include "vehicle/can/feature/region.h"

inline void applyNagSuppressBits(Frame &f, State &s)
{
	setBit(f, 19, false);
	setBit(f, 47, true);  // summon EU unlock — matches HW4 and All HW/summon-eu-unlock.json
	// Enhanced Autopilot: set bit 46 on mux=1 to unlock EAP/Summon
	// Source: ev-open-can-tools hw4OffsetRuntime / HW3Handler + hypery11 enhanced_autopilot
	if (s.enhancedAutopilot)
		setBit(f, 46, true);
	// Lane graph visualization: bit 45 on mux=1
	// Source: hypery11/flipper-tesla-fsd fsd_handler.c (assist_show_lane_graph)
	if (s.laneGraphEnable)
		setBit(f, 45, true);
	// ECE R79 bypass: clear EU speed restriction bit for European vehicles
	if (s.eceR79Bypass && s.hasRegion && isEuropeanMarket(s.regionCode))
		applyEceR79Bypass(f);
	driverSend(f, 0);
	s.canDiag.eapModCount++;
}
