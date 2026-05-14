#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/body/mirror.h
 * @brief Mirror fold, heat, auto-fold, and dip control via CAN frame 0x273
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "vehicle/can/fwd.h"

/**
 * @brief Mirror fold request states for the fold/unfold control field.
 */
enum MirrorFoldRequest
{
	MIRROR_IDLE = 0,  // No fold action requested
	MIRROR_FOLD = 1,  // Request mirror fold
	MIRROR_UNFOLD = 2 // Request mirror unfold
};

/**
 * @brief Set the mirror fold/unfold request field in a UI_vehicleControl frame.
 * @param f CAN frame (0x273) to modify.
 * @param req Desired fold action.
 */
inline void setMirrorFold(Frame &f, MirrorFoldRequest req)
{
	if (f.dlc < 4)
		return;
	f.data[3] = (f.data[3] & ~0x03) | (req & 0x03); // bits 24-25
}

/**
 * @brief Set the heated mirrors enable bit in a UI_vehicleControl frame.
 * @param f CAN frame (0x273) to modify.
 * @param enable True to activate mirror heating, false to deactivate.
 */
inline void setMirrorHeat(Frame &f, bool enable)
{
	if (f.dlc < 4)
		return;
	if (enable)
		f.data[3] |= 0x04; // bit 26
	else
		f.data[3] &= ~0x04;
}

/**
 * @brief Set the automatic fold-on-lock bit in a UI_vehicleControl frame.
 * @param f CAN frame (0x273) to modify.
 * @param enable True to enable auto-fold on lock, false to disable.
 */
inline void setAutoFoldMirrors(Frame &f, bool enable)
{
	if (f.dlc < 7)
		return;
	if (enable)
		f.data[6] |= 0x10; // bit 52
	else
		f.data[6] &= ~0x10;
}

/**
 * @brief Set the mirror dip-on-reverse bit in a UI_vehicleControl frame.
 * @param f CAN frame (0x273) to modify.
 * @param enable True to dip mirror when shifting to reverse, false to disable.
 */
inline void setMirrorDipOnReverse(Frame &f, bool enable)
{
	if (f.dlc < 7)
		return;
	if (enable)
		f.data[6] |= 0x20; // bit 53
	else
		f.data[6] &= ~0x20;
}

/**
 * @brief Burst-send a mirror fold or unfold command on BUS_VEHICLE.
 *
 * Uses 50 burst frames at 20 ms intervals to ensure the ECU registers
 * the fold command despite periodic factory frames.
 *
 * @param req Fold action to perform.
 * @param s Vehicle state providing the base control frame.
 */
static void controlMirrorFold(MirrorFoldRequest req, State &s)
{
	Frame f = makeCtrlFrame(s);
	setMirrorFold(f, req);

	startBurst(s, f, BUS_VEHICLE, 50, 20);
}

/**
 * @brief Burst-send a heated mirrors activation command on BUS_VEHICLE.
 * @param s Vehicle state providing the base control frame.
 */
static void controlMirrorHeat(State &s)
{
	Frame f = makeCtrlFrame(s);
	setMirrorHeat(f, true);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

/**
 * @brief Burst-send a factory auto-fold-on-lock toggle command on BUS_VEHICLE.
 * @param s Vehicle state providing the base control frame.
 */
static void controlAutoFoldMirrors(State &s)
{
	Frame f = makeCtrlFrame(s);
	setAutoFoldMirrors(f, true);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

/**
 * @brief Burst-send a mirror dip-on-reverse activation command on BUS_VEHICLE.
 * @param s Vehicle state providing the base control frame.
 */
static void controlMirrorDip(State &s)
{
	Frame f = makeCtrlFrame(s);
	setMirrorDipOnReverse(f, true);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

/**
 * @brief Monitor lock state transitions and auto-fold/unfold mirrors.
 *
 * When the vehicle transitions from unlocked to locked, mirrors are folded.
 * When transitioning from locked to unlocked, mirrors are unfolded.
 * Requires mirrorAutoFoldEnabled and hasCtrl to be active.
 *
 * @param s Vehicle state tracking lock transitions.
 * @param vehicleLocked Current lock state reported by the vehicle.
 */
inline void mirrorAutoFoldCheck(State &s, bool vehicleLocked)
{
	if (!s.mirrorAutoFoldEnabled)
		return;
	if (!s.hasCtrl)
		return;

	if (vehicleLocked && !s.vehicleLockedState)
	{
		// Transition to locked: fold mirrors
		controlMirrorFold(MIRROR_FOLD, s);
	}
	else if (!vehicleLocked && s.vehicleLockedState)
	{
		// Transition to unlocked: unfold mirrors
		controlMirrorFold(MIRROR_UNFOLD, s);
	}
	s.vehicleLockedState = vehicleLocked;
}

/**
 * @brief Execute a mirror control command string.
 *
 * Dispatches "mirror:fold", "mirror:unfold", "mirror:heat",
 * "mirror:autofold", and "mirror:dip" commands.
 * Requires the vehicle control frame to be available (hasCtrl).
 *
 * @param cmd Null-terminated command string.
 * @param s Vehicle state to operate on.
 * @return True if the command was recognized and executed.
 */
static bool executeMirrorCmd(const char *cmd, State &s)
{
	if (!s.hasCtrl)
		return false;

	if (strcmp(cmd, "mirror:fold") == 0)
	{
		controlMirrorFold(MIRROR_FOLD, s);
		return true;
	}
	if (strcmp(cmd, "mirror:unfold") == 0)
	{
		controlMirrorFold(MIRROR_UNFOLD, s);
		return true;
	}
	if (strcmp(cmd, "mirror:heat") == 0)
	{
		controlMirrorHeat(s);
		return true;
	}
	if (strcmp(cmd, "mirror:autofold") == 0)
	{
		controlAutoFoldMirrors(s);
		return true;
	}
	if (strcmp(cmd, "mirror:dip") == 0)
	{
		controlMirrorDip(s);
		return true;
	}
	return false;
}

/**
 * @brief Execute the mirror auto-fold on/off toggle command.
 *
 * Enables or disables the automatic fold-on-lock behavior that is
 * persisted via NVS.
 *
 * @param cmd Null-terminated command string.
 * @param s Vehicle state to modify.
 * @return True if the command was recognized and executed.
 */
static bool executeMirrorAutoFoldCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "mirror:autofold:on") == 0)
	{
		s.mirrorAutoFoldEnabled = true;
		return true;
	}
	if (strcmp(cmd, "mirror:autofold:off") == 0)
	{
		s.mirrorAutoFoldEnabled = false;
		return true;
	}
	return false;
}
