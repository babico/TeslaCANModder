#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/body/sentry.h
 * @brief Sentry mode activation and deactivation via CAN burst on the body bus
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "vehicle/can/fwd.h"

/**
 * @brief Send a CAN burst to enable or disable sentry mode
 * @param enable True to activate sentry mode, false to deactivate
 * @param s Device state reference
 *
 * @note Transmits CAN ID 0x284 as a 30-frame burst with 20 ms spacing on the body bus.
 */
static void controlSentry(bool enable, State &s)
{
	Frame f;
	f.id = CAN_ID_SENTRY;
	f.dlc = 5;
	// Byte 0: 0x20 arms sentry, 0x00 disarms
	f.data[0] = enable ? 0x20 : 0x00;
	f.data[1] = 0x00;
	f.data[2] = 0x00;
	f.data[3] = 0x00;
	f.data[4] = 0x00;

	startBurst(s, f, BUS_BODY, 30, 20);
}

/**
 * @brief Execute a sentry mode command string
 * @param cmd Null-terminated command (e.g. "sentry:on" or "sentry:off")
 * @param s Device state reference
 * @return True if the command was recognized and handled
 *
 * @note Sentry commands are rejected on LEGACY variant vehicles.
 */
static bool executeSentryCmd(const char *cmd, State &s)
{
	if (s.variant == LEGACY)
		return false;

	if (strcmp(cmd, "sentry:on") == 0)
	{
		controlSentry(true, s);
		return true;
	}
	if (strcmp(cmd, "sentry:off") == 0)
	{
		controlSentry(false, s);
		return true;
	}
	return false;
}
