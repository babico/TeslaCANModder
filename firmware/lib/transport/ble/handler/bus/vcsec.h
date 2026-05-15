#pragma once

/**
 * @file firmware/lib/transport/ble/handler/bus/vcsec.h
 * @brief VCSEC domain (Domain 2) handler - works when car is ASLEEP
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#if BOARD_ENABLE_BLE

#include "vehicle/ble/feature/vcsec.h"
#include "../helpers.h"
#include <stdint.h>
#include <string.h>

namespace Tesla
{

/**
 * @brief Run a VCSEC command using SIGNATURE_TYPE_PRESENT_KEY (no ECDH session required).
 * @param board Platform adapter providing transport and logging.
 * @param buildFn Callable that encodes the VCSEC ToVCSECMessage.
 * @param desc Human-readable description logged on success.
 */
template <typename Board, typename BuildFn>
static bool runVCSECCommand(Board &board, BuildFn &&buildFn, const char *desc)
{
	auto cl = board.makeTransport();
	if (!connectToVehicle(board, cl))
		return false;

	uint8_t buf[128];
	size_t bufLen = 0;
	if (!buildFn(buf, sizeof(buf), bufLen))
	{
		board.err("Tesla: VCSEC message encoding failed");
		cl.disconnect();
		return false;
	}
	if (!cl.send(buf, bufLen))
	{
		board.err("Tesla: VCSEC send failed");
		cl.disconnect();
		return false;
	}

	board.log(desc);
	cl.disconnect();
	return true;
}

/**
 * @brief Dispatch VCSEC domain commands.
 * @param board Platform adapter.
 * @param sub Sub-command string after "tesla:" prefix.
 * @return True if the command was handled.
 */
template <typename Board> static bool dispatchVCSEC(Board &board, const char *sub)
{
	if (strcmp(sub, "lock") == 0)
	{
		runVCSECCommand(
			board,
			[](uint8_t *out, size_t cap, size_t &outLen)
			{ return buildVCSECRKEAction(RKE_ACTION_LOCK, out, cap, outLen); },
			"Tesla: lock sent");
		return true;
	}

	if (strcmp(sub, "unlock") == 0)
	{
		runVCSECCommand(
			board,
			[](uint8_t *out, size_t cap, size_t &outLen)
			{ return buildVCSECRKEAction(RKE_ACTION_UNLOCK, out, cap, outLen); },
			"Tesla: unlock sent");
		return true;
	}

	if (strcmp(sub, "trunk:open") == 0)
	{
		runVCSECCommand(
			board,
			[](uint8_t *out, size_t cap, size_t &outLen)
			{ return buildVCSECClosureAction(CLOSURE_FIELD_REAR_TRUNK, CLOSURE_MOVE_TYPE_OPEN, out, cap, outLen); },
			"Tesla: trunk open sent");
		return true;
	}

	if (strcmp(sub, "trunk:close") == 0)
	{
		runVCSECCommand(
			board,
			[](uint8_t *out, size_t cap, size_t &outLen)
			{ return buildVCSECClosureAction(CLOSURE_FIELD_REAR_TRUNK, CLOSURE_MOVE_TYPE_CLOSE, out, cap, outLen); },
			"Tesla: trunk close sent");
		return true;
	}

	if (strcmp(sub, "frunk:open") == 0)
	{
		runVCSECCommand(
			board,
			[](uint8_t *out, size_t cap, size_t &outLen)
			{ return buildVCSECClosureAction(CLOSURE_FIELD_FRONT_TRUNK, CLOSURE_MOVE_TYPE_OPEN, out, cap, outLen); },
			"Tesla: frunk open sent");
		return true;
	}

	if (strcmp(sub, "tonneau:open") == 0)
	{
		runVCSECCommand(
			board,
			[](uint8_t *out, size_t cap, size_t &outLen)
			{ return buildVCSECClosureAction(CLOSURE_FIELD_TONNEAU, CLOSURE_MOVE_TYPE_OPEN, out, cap, outLen); },
			"Tesla: tonneau open sent");
		return true;
	}

	if (strcmp(sub, "tonneau:close") == 0)
	{
		runVCSECCommand(
			board,
			[](uint8_t *out, size_t cap, size_t &outLen)
			{ return buildVCSECClosureAction(CLOSURE_FIELD_TONNEAU, CLOSURE_MOVE_TYPE_CLOSE, out, cap, outLen); },
			"Tesla: tonneau close sent");
		return true;
	}

	if (strcmp(sub, "tonneau:stop") == 0)
	{
		runVCSECCommand(
			board,
			[](uint8_t *out, size_t cap, size_t &outLen)
			{ return buildVCSECClosureAction(CLOSURE_FIELD_TONNEAU, CLOSURE_MOVE_TYPE_STOP, out, cap, outLen); },
			"Tesla: tonneau stop sent");
		return true;
	}

	return false;
}

} // namespace Tesla

#endif // BOARD_ENABLE_BLE