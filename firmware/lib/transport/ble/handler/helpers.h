#pragma once

/**
 * @file firmware/lib/transport/ble/handler/helpers.h
 * @brief Shared BLE transport and session helpers used by domain handlers
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#if BOARD_ENABLE_BLE

#include "vehicle/ble/feature/session.h"
#include "vehicle/ble/msg.h"
#include <stdint.h>
#include <string.h>

namespace Tesla
{

/**
 * @brief Connect to a Tesla vehicle over BLE using the last 8 characters of the stored VIN as a suffix filter.
 * @param board Platform adapter providing VIN storage and logging.
 * @param client BLE transport used for scanning and connecting.
 * @return True if the connection was established successfully.
 */
template <typename Board, typename Transport> static bool connectToVehicle(Board &board, Transport &client)
{
	char vin[18] = {};
	board.loadVin(vin, sizeof(vin));

	const char *suffix = nullptr;
	char sfxBuf[9] = {};
	size_t vinLen = strlen(vin);
	if (vinLen >= 8)
	{
		memcpy(sfxBuf, vin + vinLen - 8, 8);
		sfxBuf[8] = '\0';
		suffix = sfxBuf;
	}

	board.log("Tesla: scanning for vehicle...");
	if (!client.connect(suffix, 10))
	{
		board.err("Tesla: vehicle not found via BLE");
		return false;
	}
	board.log("Tesla: connected");
	return true;
}

/**
 * @brief Establish an authenticated BLE session and send a CarServer command.
 * @param board Platform adapter providing transport, key storage, and logging.
 * @param buildFn Callable with signature bool(uint8_t *out, size_t cap, size_t &outLen) that encodes the action.
 * @param desc Human-readable description logged on success.
 * @return True if the command was sent successfully.
 */
template <typename Board, typename BuildFn>
static bool runAuthCommand(Board &board, BuildFn &&buildFn, const char *desc)
{
	auto cl = board.makeTransport();
	if (!connectToVehicle(board, cl))
		return false;

	TeslaSession<Board> sess(board);
	board.log("Tesla: establishing ECDH session...");
	if (!sess.establish(cl, DOMAIN_INFOTAINMENT))
	{
		board.err("Tesla: session establishment failed");
		cl.disconnect();
		return false;
	}

	uint8_t actionBuf[128];
	size_t actionLen = 0;
	if (!buildFn(actionBuf, sizeof(actionBuf), actionLen))
	{
		board.err("Tesla: message encoding failed");
		cl.disconnect();
		return false;
	}

	if (!sess.sendCommand(cl, actionBuf, actionLen))
	{
		board.err("Tesla: command send failed");
		cl.disconnect();
		return false;
	}

	board.log(desc);
	cl.disconnect();
	return true;
}

} // namespace Tesla

#endif // BOARD_ENABLE_BLE