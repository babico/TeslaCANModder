#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/wiper.h
 * @brief Wiper speed control and persistence via CAN ID 0x273 (UI_vehicleControl)
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "vehicle/can/fwd.h"

/**
 * @brief Wiper speed request levels
 */
enum WiperRequest
{
	WIPER_OFF = 0,  // Wipers off
	WIPER_1 = 1,    // Intermittent
	WIPER_2 = 2,    // Normal speed
	WIPER_3 = 3     // Fast speed
};

/**
 * @brief Write the requested wiper speed into byte 7 bits [2:0] of a UI_vehicleControl frame
 * @param f The CAN frame to modify (must have dlc >= 8).
 * @param speed The desired wiper speed level.
 */
inline void setWiperRequest(Frame &f, WiperRequest speed)
{
	if (f.dlc < 8)
		return;
	// Wiper speed occupies bits 56-58 (byte 7, lower 3 bits)
	f.data[7] = (f.data[7] & ~0x07) | (speed & 0x07);
}

/**
 * @brief Send a wiper speed override via burst-send on BUS_VEHICLE
 * @param speed The desired wiper speed level.
 * @param s Device state used for control frame construction and burst scheduling.
 * @note Burst-sends 20 frames at 20 ms intervals to override factory wiper control.
 */
static void controlWiper(WiperRequest speed, State &s)
{
	Frame f = makeCtrlFrame(s);
	setWiperRequest(f, speed);

	startBurst(s, f, BUS_VEHICLE, 20, 20);
}

/**
 * @brief Restore the persisted wiper speed on boot or wake
 * @param s Device state containing persistence flags and saved speed.
 * @note Requires wiperPersistEnabled, a non-zero saved speed, and a valid control frame.
 */
inline void wiperPersistRestore(State &s)
{
	if (!s.wiperPersistEnabled)
		return;
	if (s.savedWiperSpeed == 0)
		return;
	if (!s.hasCtrl)
		return;

	WiperRequest req = (WiperRequest)s.savedWiperSpeed;
	controlWiper(req, s);
}

/**
 * @brief Save the current wiper speed to state for later NVS persistence
 * @param s Device state to store the speed in.
 * @param speed The wiper speed value to save.
 */
inline void wiperPersistSave(State &s, uint8_t speed)
{
	if (!s.wiperPersistEnabled)
		return;
	s.savedWiperSpeed = speed;
}

/**
 * @brief Execute wiper speed commands (wiper:off, wiper:1, wiper:2, wiper:3)
 * @param cmd The command string to match.
 * @param s Device state for control frame availability and burst-send.
 * @return True if the command was recognized and executed, false otherwise.
 */
static bool executeWiperCmd(const char *cmd, State &s)
{
	if (!s.hasCtrl)
		return false;

	if (strcmp(cmd, "wiper:off") == 0)
	{
		controlWiper(WIPER_OFF, s);
		return true;
	}
	if (strcmp(cmd, "wiper:1") == 0)
	{
		controlWiper(WIPER_1, s);
		return true;
	}
	if (strcmp(cmd, "wiper:2") == 0)
	{
		controlWiper(WIPER_2, s);
		return true;
	}
	if (strcmp(cmd, "wiper:3") == 0)
	{
		controlWiper(WIPER_3, s);
		return true;
	}
	return false;
}

/**
 * @brief Toggle wiper speed persistence on or off (persisted via NVS)
 * @param cmd The command string to match ("wiperpersist:on" or "wiperpersist:off").
 * @param s Device state to update the persistence flag in.
 * @return True if the command was recognized and executed, false otherwise.
 */
static bool executeWiperPersistCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "wiperpersist:on") == 0)
	{
		s.wiperPersistEnabled = true;
		return true;
	}
	if (strcmp(cmd, "wiperpersist:off") == 0)
	{
		s.wiperPersistEnabled = false;
		return true;
	}
	return false;
}
