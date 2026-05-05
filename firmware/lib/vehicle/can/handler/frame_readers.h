#pragma once
// ── Pure Frame Readers ───────────────────────────────────────────────────────
// Stateless decoders that only depend on Frame layout. Split out from
// handler/helpers.h so unit tests can exercise them without dragging in the
// dispatch / variant / DAS-drive subsystem.

#include "core/types.h"

// DAS_status from 0x399 byte0 bits[3:0].
inline uint8_t readDASAutopilotStatus(const Frame &f)
{
	return f.dlc >= 1 ? (f.data[0] & 0x0F) : 0;
}

// DAS_autopilotState from 0x39B byte1 bits[7:4].
// 0=UNAVAIL 1=AVAIL 2=ACTIVE_NOMINAL 3=ACTIVE_MIN_DRIVER ...
// Used by AP-First mode to delay 0x3FD injection until AP is running.
inline uint8_t readDASAutopilotState(const Frame &f)
{
	return f.dlc >= 2 ? ((f.data[1] >> 4) & 0x0F) : 0;
}

inline bool isDASAutopilotActive(uint8_t status)
{
	return status >= 3 && status <= 5;
}

// GTW_carConfig mux=2 byte5 bits[4:2] — autopilot HW tier.
// Returns -1 when the frame is not the mux=2 selector or is too short.
inline int8_t readGtwAutopilotTier(const Frame &f)
{
	if (f.dlc < 6)
		return -1;
	if (readMuxID(f) != 2)
		return -1;
	return (int8_t)((f.data[5] >> 2) & 0x07);
}
