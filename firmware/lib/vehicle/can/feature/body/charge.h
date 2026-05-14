#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/charge.h
 * @brief Charge control feature — start/stop charging and open charge port via CAN 0x333.
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "vehicle/can/fwd.h"

/**
 * @brief Actions available for the charge control CAN frame.
 */
enum ChargeAction
{
	CHARGE_STOP = 0,      // Stop active charging session
	CHARGE_START = 1,     // Start charging session
	CHARGE_PORT_OPEN = 2  // Open the charge port door
};

/**
 * @brief Send a charge control command by mutating the cached 0x333 frame and bursting it.
 * @param action The charge action to perform (start, stop, or open port).
 * @param lastCharge Pointer to the last observed 0x333 frame payload (5 bytes).
 * @param s Global state reference used for burst transmission.
 */
static void controlCharge(ChargeAction action, const uint8_t *lastCharge, State &s)
{
	Frame f;
	f.id = CAN_ID_CHARGE;
	f.dlc = 5;
	memcpy(f.data, lastCharge, 5);

	switch (action)
	{
	case CHARGE_START:
		f.data[0] |= 0x04;  // Set bit 2 — enable charging
		break;
	case CHARGE_STOP:
		f.data[0] &= ~0x04; // Clear bit 2 — disable charging
		break;
	case CHARGE_PORT_OPEN:
		f.data[0] |= 0x01;  // Set bit 0 — unlatch charge port
		break;
	}

	startBurst(s, f, BUS_VEHICLE, 20, 20);
}

/**
 * @brief Execute a charge command string received from the client.
 * @param cmd Null-terminated command string (e.g. "charge:start", "charge:stop", "charge:port").
 * @param s Global state reference containing variant info and cached frames.
 * @return True if the command was recognized and executed, false otherwise.
 */
static bool executeChargeCmd(const char *cmd, State &s)
{
	if (s.variant == LEGACY)
		return false;
	if (!s.hasCharge)
		return false; // Need 0x333 frame cached before we can mutate it

	if (strcmp(cmd, "charge:start") == 0)
	{
		controlCharge(CHARGE_START, s.lastCharge, s);
		return true;
	}
	if (strcmp(cmd, "charge:stop") == 0)
	{
		controlCharge(CHARGE_STOP, s.lastCharge, s);
		return true;
	}
	if (strcmp(cmd, "charge:port") == 0 || strcmp(cmd, "chargeport") == 0)
	{
		controlCharge(CHARGE_PORT_OPEN, s.lastCharge, s);
		return true;
	}
	return false;
}
