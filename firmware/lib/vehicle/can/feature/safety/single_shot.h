#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/single_shot.h
 * @brief Single-shot TX mode toggle for MCP2515 CAN controllers
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/types.h"
#include "core/util/parse.h"

/**
 * @brief Execute a single-shot TX mode command string
 * @param cmd Null-terminated command (e.g. "singleshot:on" or "singleshot:off")
 * @param s Device state reference
 * @return True if the command was recognized and handled
 *
 * @note When enabled, injected CAN frames use the MCP2515 one-shot flag.
 *       Frames that lose arbitration are discarded instead of retried,
 *       preventing cascading bus errors. Persisted via NVS key "ssTx".
 */
static bool executeSingleShotCmd(const char *cmd, State &s)
{
	// Match "singleshot:" prefix (11 chars) then parse the boolean suffix
	if (strncmp(cmd, "singleshot:", 11) == 0 && parseBoolCmd(cmd + 11, s.singleShotTx, s.singleShotTx))
	{
		return true;
	}
	return false;
}
