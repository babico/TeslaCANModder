#pragma once

/**
 * @file firmware/lib/transport/ble/handler/dispatch.h
 * @brief Tesla BLE command dispatcher that routes sub-commands through domain handlers
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#if BOARD_ENABLE_BLE

#include "vehicle/ble/feature/key.h"
#include "vehicle/ble/msg.h"
#include "helpers.h"
#include "bus/vcsec.h"
#include "bus/carserver.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

namespace Tesla
{

/**
 * @brief Top-level Tesla BLE command dispatcher.
 *
 * Routes the sub-command string (everything after the "tesla:" prefix) to the appropriate
 * domain handler or key management.
 *
 * @param board Platform adapter satisfying the Board concept (transport, keys, logging, etc.).
 * @param sub Sub-command string with the "tesla:" prefix already stripped by the caller.
 */
template <typename Board> static void executeTeslaCommand(Board &board, const char *sub)
{
	// --- Key management (local operations, no vehicle connection needed) ---
	if (strcmp(sub, "key:gen") == 0)
	{
		KeyPair kp;
		if (!keyGenerate(kp))
		{
			board.err("Tesla: key generation failed");
			return;
		}
		if (!board.saveKey(kp))
		{
			board.err("Tesla: key save failed");
			return;
		}
		board.log("Tesla: key generated and saved");
		return;
	}

	if (strcmp(sub, "key:show") == 0)
	{
		KeyPair kp;
		if (!board.loadKey(kp))
		{
			board.err("Tesla: no key stored (run tesla:key:gen)");
			return;
		}
		char hexBuf[131];
		hexEncode(kp.pub_xy, 65, hexBuf);
		char role[32] = {};
		board.loadRole(role, sizeof(role));
		char line[256];
		snprintf(line, sizeof(line), "{\"t\":\"tesla_key\",\"pub\":\"%s\",\"role\":\"%s\"}", hexBuf, role);
		printStr(line);
		printLn();
		return;
	}

	if (strncmp(sub, "key:role:", 9) == 0)
	{
		const char *role = sub + 9;
		if (strcmp(role, "owner") == 0 || strcmp(role, "charging_manager") == 0)
		{
			board.saveRole(role);
			char msg[48];
			snprintf(msg, sizeof(msg), "Tesla: role set to %s", role);
			board.log(msg);
		}
		else
		{
			board.err("Tesla: unknown role (use 'owner' or 'charging_manager')");
		}
		return;
	}

	if (strncmp(sub, "vin:", 4) == 0)
	{
		const char *vin = sub + 4;
		size_t vlen = strlen(vin);
		if (vlen < 5 || vlen > 17)
		{
			board.err("Tesla: VIN must be 5-17 characters");
			return;
		}
		board.saveVin(vin);
		char msg[48];
		snprintf(msg, sizeof(msg), "Tesla: VIN stored: %s", vin);
		board.log(msg);
		return;
	}

	if (strcmp(sub, "key:send") == 0)
	{
		KeyPair kp;
		if (!board.loadKey(kp))
		{
			board.err("Tesla: no key stored (run tesla:key:gen)");
			return;
		}
		char roleStr[32] = {};
		board.loadRole(roleStr, sizeof(roleStr));
		uint8_t role = board.roleValue(roleStr);

		auto cl = board.makeTransport();
		if (!connectToVehicle(board, cl))
			return;

		uint8_t uuid[16];
		board.random(uuid, 16);
		uint8_t reqBuf[200];
		size_t reqLen = 0;
		if (!buildAddKeyRequest(kp.pub_xy, role, KEY_FORM_FACTOR_NFC_CARD, uuid, reqBuf, sizeof(reqBuf), reqLen))
		{
			board.err("Tesla: add-key-request encoding failed");
			cl.disconnect();
			return;
		}
		if (!cl.send(reqBuf, reqLen))
		{
			board.err("Tesla: add-key-request send failed");
			cl.disconnect();
			return;
		}
		board.log("Tesla: add-key-request sent - owner must tap NFC card on vehicle");
		cl.disconnect();
		return;
	}

	// --- VCSEC domain (lock, unlock, closures) ---
	if (dispatchVCSEC(board, sub))
		return;

	// --- CarServer domain (honk, flash, climate, charge, etc.) ---
	if (dispatchCarServer(board, sub))
		return;

	board.err("Tesla: unknown sub-command");
}

} // namespace Tesla

#endif // BOARD_ENABLE_BLE