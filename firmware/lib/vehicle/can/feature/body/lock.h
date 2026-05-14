#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/body/lock.h
 * @brief Door lock, child lock, and horn control via CAN frame 0x273
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "vehicle/can/fwd.h"

/**
 * @brief Lock request states for the door lock control field.
 */
enum LockRequest
{
	LOCK_IDLE = 0, // No lock action requested
	LOCK = 1,      // Request door lock
	UNLOCK = 2     // Request door unlock
};

/**
 * @brief Set the door lock request field in a UI_vehicleControl frame.
 * @param f CAN frame (0x273) to modify.
 * @param req Desired lock action.
 */
inline void setLockRequest(Frame &f, LockRequest req)
{
	if (f.dlc < 3)
		return;
	f.data[2] = (f.data[2] & ~0x0E) | ((req & 0x07) << 1); // bits 17-19
}

/**
 * @brief Set the child door lock bit in a UI_vehicleControl frame.
 * @param f CAN frame (0x273) to modify.
 * @param enable True to engage child lock, false to disengage.
 */
inline void setChildDoorLock(Frame &f, bool enable)
{
	if (f.dlc < 3)
		return;
	if (enable)
		f.data[2] |= 0x01; // bit 16
	else
		f.data[2] &= ~0x01;
}

/**
 * @brief Set the horn request bit in a UI_vehicleControl frame.
 *
 * Horn uses 0x273 bit 61. Placed in the lock module as it is
 * security-adjacent functionality.
 *
 * @param f CAN frame (0x273) to modify.
 * @param honk True to activate horn, false to deactivate.
 */
inline void setHornRequest(Frame &f, bool honk)
{
	if (f.dlc < 8)
		return;
	if (honk)
		f.data[7] |= 0x20; // bit 61
	else
		f.data[7] &= ~0x20;
}

/**
 * @brief Burst-send a door lock or unlock command on BUS_VEHICLE.
 * @param req Lock action to perform.
 * @param s Vehicle state providing the base control frame.
 */
static void controlLock(LockRequest req, State &s)
{
	Frame f = makeCtrlFrame(s);
	setLockRequest(f, req);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

/**
 * @brief Burst-send a child door lock activation command on BUS_VEHICLE.
 * @param s Vehicle state providing the base control frame.
 */
static void controlChildLock(State &s)
{
	Frame f = makeCtrlFrame(s);
	setChildDoorLock(f, true);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

/**
 * @brief Burst-send a horn activation command on BUS_VEHICLE.
 * @param s Vehicle state providing the base control frame.
 */
static void controlHorn(State &s)
{
	Frame f = makeCtrlFrame(s);
	setHornRequest(f, true);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

/**
 * @brief Execute a lock-related command string.
 *
 * Dispatches "lock", "unlock", "lock:child", and "horn" commands.
 * Requires the vehicle control frame to be available (hasCtrl).
 *
 * @param cmd Null-terminated command string.
 * @param s Vehicle state to operate on.
 * @return True if the command was recognized and executed.
 */
static bool executeLockCmd(const char *cmd, State &s)
{
	if (!s.hasCtrl)
		return false;

	if (strcmp(cmd, "lock") == 0)
	{
		controlLock(LOCK, s);
		return true;
	}
	if (strcmp(cmd, "unlock") == 0)
	{
		controlLock(UNLOCK, s);
		return true;
	}
	if (strcmp(cmd, "lock:child") == 0)
	{
		controlChildLock(s);
		return true;
	}
	if (strcmp(cmd, "horn") == 0)
	{
		controlHorn(s);
		return true;
	}
	return false;
}
