#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/power.h
 * @brief Power state control via CAN frame 0x273 (UI_vehicleControl)
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "vehicle/can/fwd.h"

/**
 * @brief Set or clear the accessory power bit in a control frame
 * @param f CAN frame to modify (must have dlc >= 1)
 * @param enable true to assert accessory power, false to deassert
 */
inline void setAccessoryPower(Frame &f, bool enable)
{
	if (f.dlc < 1)
		return;
	if (enable)
		f.data[0] |= 0x01;   // bit 0: accessory power request
	else
		f.data[0] &= ~0x01;
}

/**
 * @brief Set or clear the power-off request bit in a control frame
 * @param f CAN frame to modify (must have dlc >= 4)
 * @param off true to request power off, false to clear
 */
inline void setPowerOff(Frame &f, bool off)
{
	if (f.dlc < 4)
		return;
	if (off)
		f.data[3] |= 0x80;   // bit 31: power-off request
	else
		f.data[3] &= ~0x80;
}

/**
 * @brief Set or clear the drive-state readiness request bit
 * @param f CAN frame to modify (must have dlc >= 8)
 * @param enable true to request drive-ready state, false to clear
 */
inline void setDriveStateRequest(Frame &f, bool enable)
{
	if (f.dlc < 8)
		return;
	if (enable)
		f.data[7] |= 0x40;   // bit 62: drive state request
	else
		f.data[7] &= ~0x40;
}

/**
 * @brief Send a power-off burst on the vehicle bus
 * @param s Device state used for burst scheduling and frame construction
 */
static void controlPowerOff(State &s)
{
	Frame f = makeCtrlFrame(s);
	setPowerOff(f, true);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

/**
 * @brief Send an accessory power enable/disable burst on the vehicle bus
 * @param enable true to turn accessory power on, false to turn off
 * @param s Device state used for burst scheduling and frame construction
 */
static void controlAccessoryPower(bool enable, State &s)
{
	Frame f = makeCtrlFrame(s);
	setAccessoryPower(f, enable);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

/**
 * @brief Send a drive-state readiness request burst on the vehicle bus
 * @param s Device state used for burst scheduling and frame construction
 */
static void controlDriveState(State &s)
{
	Frame f = makeCtrlFrame(s);
	setDriveStateRequest(f, true);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

/**
 * @brief Execute a power-related command string
 * @param cmd Null-terminated command (e.g. "power:acc:on", "power:off", "power:ready")
 * @param s Device state; must have a valid control frame captured
 * @return true if the command was recognized and executed
 */
static bool executePowerCmd(const char *cmd, State &s)
{
	if (!s.hasCtrl)
		return false;

	if (strcmp(cmd, "power:acc:on") == 0)
	{
		controlAccessoryPower(true, s);
		return true;
	}
	if (strcmp(cmd, "power:acc:off") == 0)
	{
		controlAccessoryPower(false, s);
		return true;
	}
	if (strcmp(cmd, "power:ready") == 0)
	{
		controlDriveState(s);
		return true;
	}
	if (strcmp(cmd, "power:off") == 0)
	{
		controlPowerOff(s);
		return true;
	}
	return false;
}
