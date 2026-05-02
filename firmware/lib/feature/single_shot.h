#pragma once
#include "core/types.h"
#include "infra/parse.h"

// ── 1.6 Single-Shot TX Mode ─────────────────────────────────────────────────
// When enabled, all injected CAN frames use the MCP2515 one-shot TX mode
// (TXBnCTRL.TXREQ with one-shot flag). Frames that fail to arbitrate are
// discarded rather than retried, preventing cascading bus errors.
//
// Command: singleshot:on / singleshot:off
// NVS key: "ssTx" (persisted bool)

inline bool execSingleShotCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "singleshot:", 11) == 0 && parseBoolCmd(cmd + 11, s.singleShotTx, s.singleShotTx))
	{
		return true;
	}
	return false;
}
