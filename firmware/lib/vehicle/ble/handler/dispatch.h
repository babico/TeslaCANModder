#pragma once
// ── Tesla BLE command dispatch (board-agnostic) ───────────────────────────────
//
// executeTeslaCommand<Board>(board, sub) is the platform-independent entry point.
// The Board adapter provides all platform services.
//
// Board concept — must provide:
//   makeTransport()              → Transport object (connect/disconnect/send/exchange)
//   random(uint8_t *out, size_t) → fill with random bytes
//   loadKey(KeyPair &)           → bool   (false = not stored)
//   saveKey(const KeyPair &)     → bool
//   loadRole(char *out, size_t)  → fills buffer, e.g. "owner"
//   saveRole(const char *)
//   loadVin(char *out, size_t)   → fills buffer with stored VIN
//   saveVin(const char *)
//   roleValue(const char *)      → uint8_t  (e.g. ROLE_OWNER)
//   log(const char *)
//   err(const char *)
//   print(const char *)
//   println()

#if BOARD_ENABLE_BLE

#include "vehicle/ble/feature/key.h"
#include "vehicle/ble/feature/session.h"
#include "vehicle/ble/msg.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

namespace Tesla
{

// ── Internal helpers ──────────────────────────────────────────────────────────

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

// BuildFn: any callable with signature bool(uint8_t *out, size_t cap, size_t &outLen)
// This accepts both free functions and lambdas (including capturing ones).
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

// ── executeTeslaCommand ───────────────────────────────────────────────────────
// sub = everything after the "tesla:" prefix (caller strips it).
template <typename Board> static void executeTeslaCommand(Board &board, const char *sub)
{
	// ── tesla:key:gen ─────────────────────────────────────────────────────────
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

	// ── tesla:key:show ────────────────────────────────────────────────────────
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
		board.print(line);
		board.println();
		return;
	}

	// ── tesla:key:role:<role> ─────────────────────────────────────────────────
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

	// ── tesla:vin:<VIN> ───────────────────────────────────────────────────────
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

	// ── tesla:key:send ────────────────────────────────────────────────────────
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

	// ── tesla:wake ────────────────────────────────────────────────────────────
	if (strcmp(sub, "wake") == 0)
	{
		runAuthCommand(board, buildWakeAction, "Tesla: wake sent");
		return;
	}

	// ── tesla:charge:start ────────────────────────────────────────────────────
	if (strcmp(sub, "charge:start") == 0)
	{
		runAuthCommand(board, buildChargeStartAction, "Tesla: charge start sent");
		return;
	}

	// ── tesla:charge:stop ─────────────────────────────────────────────────────
	if (strcmp(sub, "charge:stop") == 0)
	{
		runAuthCommand(board, buildChargeStopAction, "Tesla: charge stop sent");
		return;
	}

	// ── tesla:charge:amps:<n> ─────────────────────────────────────────────────
	if (strncmp(sub, "charge:amps:", 12) == 0)
	{
		int amps = atoi(sub + 12);
		if (amps < 1 || amps > 32)
		{
			board.err("Tesla: amps must be 1-32");
			return;
		}
		char msg[48];
		snprintf(msg, sizeof(msg), "Tesla: charge amps set to %d", amps);
		runAuthCommand(
			board, [amps](uint8_t *out, size_t cap, size_t &outLen)
			{ return buildSetAmpsAction(amps, out, cap, outLen); }, msg);
		return;
	}

	// ── tesla:charge:limit:<n> ────────────────────────────────────────────────
	if (strncmp(sub, "charge:limit:", 13) == 0)
	{
		int pct = atoi(sub + 13);
		if (pct < 50 || pct > 100)
		{
			board.err("Tesla: limit must be 50-100");
			return;
		}
		char msg[48];
		snprintf(msg, sizeof(msg), "Tesla: charge limit set to %d%%", pct);
		runAuthCommand(
			board, [pct](uint8_t *out, size_t cap, size_t &outLen)
			{ return buildSetLimitAction(pct, out, cap, outLen); }, msg);
		return;
	}

	// ── tesla:climate:on ──────────────────────────────────────────────────────
	if (strcmp(sub, "climate:on") == 0)
	{
		runAuthCommand(
			board, [](uint8_t *out, size_t cap, size_t &outLen) { return buildClimateAction(true, out, cap, outLen); },
			"Tesla: climate on sent");
		return;
	}

	// ── tesla:climate:off ─────────────────────────────────────────────────────
	if (strcmp(sub, "climate:off") == 0)
	{
		runAuthCommand(
			board, [](uint8_t *out, size_t cap, size_t &outLen) { return buildClimateAction(false, out, cap, outLen); },
			"Tesla: climate off sent");
		return;
	}

	board.err("Tesla: unknown sub-command");
}

} // namespace Tesla

#endif // BOARD_ENABLE_BLE
