#pragma once
#include "vehicle/can/fwd.h"

// ── Charge Control (0x333) ───────────────────────────────────────────────────

enum ChargeAction
{
	CHARGE_STOP = 0,
	CHARGE_START = 1,
	CHARGE_PORT_OPEN = 2
};

static void controlCharge(ChargeAction action, const uint8_t *lastCharge, State &s)
{
	Frame f;
	f.id = CAN_ID_CHARGE;
	f.dlc = 5;
	memcpy(f.data, lastCharge, 5);

	switch (action)
	{
	case CHARGE_START:
		f.data[0] |= 0x04; // Set bit 2
		break;
	case CHARGE_STOP:
		f.data[0] &= ~0x04; // Clear bit 2
		break;
	case CHARGE_PORT_OPEN:
		f.data[0] |= 0x01; // Set bit 0
		break;
	}

	startBurst(s, f, BUS_VEHICLE, 20, 20);
}

// ── Charge Command Execution ────────────────────────────────────────────────

static bool executeChargeCmd(const char *cmd, State &s)
{
	if (s.variant == LEGACY)
		return false;
	if (!s.hasCharge)
		return false; // Need 0x333 frame cached

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
