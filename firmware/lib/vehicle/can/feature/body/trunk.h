#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/body/trunk.h
 * @brief Unified trunk, frunk, and glovebox control via CAN injection
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "vehicle/can/fwd.h"

/**
 * @brief Identifies which trunk compartment to operate on.
 */
enum TrunkTarget
{
	TRUNK_FRUNK = 0,   // Front trunk (CAN 0x273)
	TRUNK_REAR = 1,    // Rear trunk (CAN 0x3B3)
	TRUNK_GLOVEBOX = 2 // Glovebox (CAN 0x3B3, open only)
};

/**
 * @brief Specifies the desired trunk action.
 */
enum TrunkAction
{
	TRUNK_OPEN = 0,
	TRUNK_CLOSE = 1
};

/**
 * @brief Send a frunk open/close command on CAN 0x273.
 *
 * Copies the last cached UI_vehicleControl frame and sets or clears
 * bit 5 of byte[0] to request frunk open or close.
 *
 * @param lastCtrl Pointer to the last captured 0x273 frame payload (8 bytes).
 * @param open True to open, false to close.
 * @param s Device state for burst transmission.
 */
static void controlFrunk(const uint8_t *lastCtrl, bool open, State &s)
{
	Frame f;
	f.id = CAN_ID_UI_VEHICLE_CTRL;
	f.dlc = 8;
	memcpy(f.data, lastCtrl, 8);

	if (open)
	{
		f.data[0] |= 0x20; // Set bit 5 (frunk open request)
	}
	else
	{
		f.data[0] &= ~0x20; // Clear bit 5
	}

	startBurst(s, f, BUS_VEHICLE, 20, 20);
}

/**
 * @brief Send a rear trunk open/close command on CAN 0x3B3.
 * @param open True to open (0x02), false to close (0x03).
 * @param s Device state for burst transmission.
 */
static void controlTrunk(bool open, State &s)
{
	Frame f;
	f.id = CAN_ID_TRUNK_CTRL;
	f.dlc = 4;
	f.data[0] = open ? 0x02 : 0x03; // 0x02 = open, 0x03 = close
	f.data[1] = 0x00;
	f.data[2] = 0x00;
	f.data[3] = 0x00;

	startBurst(s, f, BUS_BODY, 20, 100);
}

/**
 * @brief Send a glovebox open command on CAN 0x3B3.
 *
 * The glovebox does not support a close command over CAN.
 *
 * @param s Device state for burst transmission.
 */
static void controlGlovebox(State &s)
{
	Frame f;
	f.id = CAN_ID_TRUNK_CTRL;
	f.dlc = 4;
	f.data[0] = 0x01; // Glovebox open command
	f.data[1] = 0x00;
	f.data[2] = 0x00;
	f.data[3] = 0x00;

	startBurst(s, f, BUS_BODY, 20, 100);
}

/**
 * @brief Unified dispatch for trunk/frunk/glovebox operations.
 * @param target Which compartment to operate on.
 * @param action Open or close.
 * @param s Device state for cached frame access and burst.
 * @return True if the operation was dispatched successfully.
 */
static bool executeTrunkControl(TrunkTarget target, TrunkAction action, State &s)
{
	switch (target)
	{
	case TRUNK_FRUNK:
		if (!s.hasCtrl)
			return false; // Need cached 0x273 frame
		controlFrunk(s.lastCtrl, action == TRUNK_OPEN, s);
		return true;

	case TRUNK_REAR:
		controlTrunk(action == TRUNK_OPEN, s);
		return true;

	case TRUNK_GLOVEBOX:
		if (action == TRUNK_CLOSE)
			return false; // Glovebox has no close command
		controlGlovebox(s);
		return true;

	default:
		return false;
	}
}

/**
 * @brief Execute a trunk/frunk/glovebox command string.
 *
 * Supported commands: "frunk:open", "frunk", "frunk:close",
 * "trunk:open", "trunk", "trunk:close", "glovebox".
 * Rejected on LEGACY variant.
 *
 * @param cmd Null-terminated command string.
 * @param s Device state for variant check and burst.
 * @return True if the command was recognized and executed.
 */
static bool executeTrunkCmd(const char *cmd, State &s)
{
	if (s.variant == LEGACY)
		return false;
	if (strcmp(cmd, "frunk:open") == 0 || strcmp(cmd, "frunk") == 0)
	{
		return executeTrunkControl(TRUNK_FRUNK, TRUNK_OPEN, s);
	}
	if (strcmp(cmd, "frunk:close") == 0)
	{
		return executeTrunkControl(TRUNK_FRUNK, TRUNK_CLOSE, s);
	}
	if (strcmp(cmd, "trunk:open") == 0 || strcmp(cmd, "trunk") == 0)
	{
		return executeTrunkControl(TRUNK_REAR, TRUNK_OPEN, s);
	}
	if (strcmp(cmd, "trunk:close") == 0)
	{
		return executeTrunkControl(TRUNK_REAR, TRUNK_CLOSE, s);
	}
	if (strcmp(cmd, "glovebox") == 0)
	{
		return executeTrunkControl(TRUNK_GLOVEBOX, TRUNK_OPEN, s);
	}
	return false;
}
