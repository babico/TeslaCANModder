#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/das/tlssc.h
 * @brief TLSSC (Tesla Licensed Self-Steering Capability) restore feature
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "core/util/parse.h"

/**
 * @brief Execute the TLSSC restore enable/disable command.
 *
 * Parses "tlssc:<on|off>" to toggle spoofing of DAS_autopilotConfig (0x331)
 * so the AP ECU reports SELF_DRIVING tier. The setting is persisted to NVS.
 *
 * @param cmd Null-terminated command string (expected prefix "tlssc:").
 * @param s Global state reference.
 * @return true if the command was recognized and executed successfully.
 */
static bool executeTlsscCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "tlssc:", 6) == 0)
	{
		if (!parseBoolCmd(cmd + 6, s.tlsscRestore, s.tlsscRestore))
			return false;
		saveSettings(s);
		return true;
	}
	return false;
}

/**
 * @brief Handle a DAS_autopilotConfig frame (0x331) for TLSSC spoofing.
 *
 * When TLSSC restore is active, overwrites byte[0] lower 6 bits with 0x1B
 * (SELF_DRIVING tier for both DAS_autopilotBase and DAS_autopilot fields)
 * while preserving the upper 2 bits (counter/mux).
 *
 * @param f CAN frame to inspect and potentially modify.
 * @param s Global state containing the tlsscRestore flag.
 * @return true if the frame was modified and should be retransmitted.
 */
static bool handleTlssc(Frame &f, State &s)
{
	if (!s.tlsscRestore)
		return false;
	if (f.dlc < 1)
		return false;

	uint8_t original = f.data[0];
	uint8_t modified = (original & 0xC0) | 0x1B;	// Preserve upper 2 bits, set SELF_DRIVING
	if (modified == original)
		return false;

	f.data[0] = modified;
	return true;
}
