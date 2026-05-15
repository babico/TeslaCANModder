#pragma once

/**
 * @file firmware/lib/transport/ble/handler/bus/carserver.h
 * @brief CarServer domain (Domain 3) handler - requires car to be AWAKE
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#if BOARD_ENABLE_BLE

#include "vehicle/ble/feature/carserver.h"
#include "vehicle/ble/feature/vcsec.h"
#include "../helpers.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

namespace Tesla
{

/**
 * @brief Dispatch CarServer domain commands (require ECDH session + AES-GCM).
 * @param board Platform adapter.
 * @param sub Sub-command string after "tesla:" prefix.
 * @return True if the command was handled.
 */
template <typename Board> static bool dispatchCarServer(Board &board, const char *sub)
{
	if (strcmp(sub, "wake") == 0)
	{
		runAuthCommand(board, buildWakeAction, "Tesla: wake sent");
		return true;
	}

	if (strcmp(sub, "honk") == 0)
	{
		runAuthCommand(board, buildHonkAction, "Tesla: honk sent");
		return true;
	}

	if (strcmp(sub, "flash") == 0)
	{
		runAuthCommand(board, buildFlashLightsAction, "Tesla: flash sent");
		return true;
	}

	if (strcmp(sub, "window:vent") == 0)
	{
		runAuthCommand(
			board, [](uint8_t *out, size_t cap, size_t &outLen) { return buildWindowAction(true, out, cap, outLen); },
			"Tesla: window vent sent");
		return true;
	}

	if (strcmp(sub, "window:close") == 0)
	{
		runAuthCommand(
			board, [](uint8_t *out, size_t cap, size_t &outLen) { return buildWindowAction(false, out, cap, outLen); },
			"Tesla: window close sent");
		return true;
	}

	if (strcmp(sub, "sunroof:vent") == 0)
	{
		runAuthCommand(
			board, [](uint8_t *out, size_t cap, size_t &outLen) { return buildSunroofAction(3, out, cap, outLen); },
			"Tesla: sunroof vent sent");
		return true;
	}

	if (strcmp(sub, "sunroof:close") == 0)
	{
		runAuthCommand(
			board, [](uint8_t *out, size_t cap, size_t &outLen) { return buildSunroofAction(4, out, cap, outLen); },
			"Tesla: sunroof close sent");
		return true;
	}

	if (strcmp(sub, "sunroof:open") == 0)
	{
		runAuthCommand(
			board, [](uint8_t *out, size_t cap, size_t &outLen) { return buildSunroofAction(5, out, cap, outLen); },
			"Tesla: sunroof open sent");
		return true;
	}

	if (strcmp(sub, "sentry:on") == 0)
	{
		runAuthCommand(
			board,
			[](uint8_t *out, size_t cap, size_t &outLen) { return buildSentryModeAction(true, out, cap, outLen); },
			"Tesla: sentry on sent");
		return true;
	}

	if (strcmp(sub, "sentry:off") == 0)
	{
		runAuthCommand(
			board,
			[](uint8_t *out, size_t cap, size_t &outLen) { return buildSentryModeAction(false, out, cap, outLen); },
			"Tesla: sentry off sent");
		return true;
	}

	if (strcmp(sub, "chargeport:open") == 0)
	{
		runAuthCommand(
			board,
			[](uint8_t *out, size_t cap, size_t &outLen) { return buildChargePortDoorAction(true, out, cap, outLen); },
			"Tesla: chargeport open sent");
		return true;
	}

	if (strcmp(sub, "chargeport:close") == 0)
	{
		runAuthCommand(
			board,
			[](uint8_t *out, size_t cap, size_t &outLen) { return buildChargePortDoorAction(false, out, cap, outLen); },
			"Tesla: chargeport close sent");
		return true;
	}

	if (strcmp(sub, "climatekeeper:off") == 0)
	{
		runAuthCommand(
			board,
			[](uint8_t *out, size_t cap, size_t &outLen) { return buildClimateKeeperAction(0, out, cap, outLen); },
			"Tesla: climate keeper off sent");
		return true;
	}

	if (strcmp(sub, "climatekeeper:on") == 0)
	{
		runAuthCommand(
			board,
			[](uint8_t *out, size_t cap, size_t &outLen) { return buildClimateKeeperAction(1, out, cap, outLen); },
			"Tesla: climate keeper on sent");
		return true;
	}

	if (strcmp(sub, "climatekeeper:dog") == 0)
	{
		runAuthCommand(
			board,
			[](uint8_t *out, size_t cap, size_t &outLen) { return buildClimateKeeperAction(2, out, cap, outLen); },
			"Tesla: dog mode sent");
		return true;
	}

	if (strcmp(sub, "climatekeeper:camp") == 0)
	{
		runAuthCommand(
			board,
			[](uint8_t *out, size_t cap, size_t &outLen) { return buildClimateKeeperAction(3, out, cap, outLen); },
			"Tesla: camp mode sent");
		return true;
	}

	if (strncmp(sub, "valet:on:", 9) == 0)
	{
		const char *pin = sub + 9;
		runAuthCommand(
			board,
			[pin](uint8_t *out, size_t cap, size_t &outLen)
			{ return buildValetModeAction(true, pin, out, cap, outLen); },
			"Tesla: valet mode on sent");
		return true;
	}

	if (strcmp(sub, "valet:off") == 0)
	{
		runAuthCommand(
			board,
			[](uint8_t *out, size_t cap, size_t &outLen) { return buildValetModeAction(false, "", out, cap, outLen); },
			"Tesla: valet mode off sent");
		return true;
	}

	if (strncmp(sub, "steeringheat:", 13) == 0)
	{
		bool on = strcmp(sub + 13, "on") == 0;
		runAuthCommand(
			board,
			[on](uint8_t *out, size_t cap, size_t &outLen)
			{ return buildSteeringWheelHeaterAction(on, out, cap, outLen); },
			on ? "Tesla: steering heat on sent" : "Tesla: steering heat off sent");
		return true;
	}

	if (strncmp(sub, "seat:heat:", 10) == 0)
	{
		const char *rest = sub + 10;
		uint8_t position = 0;
		uint8_t level = 0;
		bool ok = false;

		if (strncmp(rest, "fl:", 3) == 0)
		{
			position = 7;
			level = (uint8_t)(rest[3] - '0');
			ok = true;
		}
		else if (strncmp(rest, "fr:", 3) == 0)
		{
			position = 8;
			level = (uint8_t)(rest[3] - '0');
			ok = true;
		}
		else if (strncmp(rest, "rl:", 3) == 0)
		{
			position = 9;
			level = (uint8_t)(rest[3] - '0');
			ok = true;
		}
		else if (strncmp(rest, "rr:", 3) == 0)
		{
			position = 12;
			level = (uint8_t)(rest[3] - '0');
			ok = true;
		}

		if (ok && level <= 5)
		{
			uint8_t heatLevel = level + 2;
			runAuthCommand(
				board,
				[position, heatLevel](uint8_t *out, size_t cap, size_t &outLen)
				{ return buildSeatHeaterAction(position, heatLevel, out, cap, outLen); },
				"Tesla: seat heat sent");
			return true;
		}
	}

	if (strncmp(sub, "homelink:", 9) == 0)
	{
		float lat = 0.0f, lon = 0.0f;
		sscanf(sub + 9, "%f:%f", &lat, &lon);
		char msg[64];
		snprintf(msg, sizeof(msg), "Tesla: homelink sent (%.4f, %.4f)", lat, lon);
		runAuthCommand(
			board,
			[lat, lon](uint8_t *out, size_t cap, size_t &outLen)
			{ return buildHomelinkAction(lat, lon, out, cap, outLen); },
			msg);
		return true;
	}

	if (strcmp(sub, "charge:start") == 0)
	{
		runAuthCommand(board, buildChargeStartAction, "Tesla: charge start sent");
		return true;
	}

	if (strcmp(sub, "charge:stop") == 0)
	{
		runAuthCommand(board, buildChargeStopAction, "Tesla: charge stop sent");
		return true;
	}

	if (strncmp(sub, "charge:amps:", 12) == 0)
	{
		int amps = atoi(sub + 12);
		if (amps < 1 || amps > 32)
		{
			board.err("Tesla: amps must be 1-32");
			return true;
		}
		char msg[48];
		snprintf(msg, sizeof(msg), "Tesla: charge amps set to %d", amps);
		runAuthCommand(
			board,
			[amps](uint8_t *out, size_t cap, size_t &outLen) { return buildSetAmpsAction(amps, out, cap, outLen); },
			msg);
		return true;
	}

	if (strncmp(sub, "charge:limit:", 13) == 0)
	{
		int pct = atoi(sub + 13);
		if (pct < 50 || pct > 100)
		{
			board.err("Tesla: limit must be 50-100");
			return true;
		}
		char msg[48];
		snprintf(msg, sizeof(msg), "Tesla: charge limit set to %d%%", pct);
		runAuthCommand(
			board,
			[pct](uint8_t *out, size_t cap, size_t &outLen) { return buildSetLimitAction(pct, out, cap, outLen); },
			msg);
		return true;
	}

	if (strcmp(sub, "climate:on") == 0)
	{
		runAuthCommand(
			board, [](uint8_t *out, size_t cap, size_t &outLen) { return buildClimateAction(true, out, cap, outLen); },
			"Tesla: climate on sent");
		return true;
	}

	if (strcmp(sub, "climate:off") == 0)
	{
		runAuthCommand(
			board, [](uint8_t *out, size_t cap, size_t &outLen) { return buildClimateAction(false, out, cap, outLen); },
			"Tesla: climate off sent");
		return true;
	}

	return false;
}

} // namespace Tesla

#endif // BOARD_ENABLE_BLE