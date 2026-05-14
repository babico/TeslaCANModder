#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/comfort/light.h
 * @brief Lighting control helpers and command execution for CAN frame 0x273
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "vehicle/can/fwd.h"

/**
 * @brief Set the front fog light switch bit in a UI_vehicleControl frame.
 * @param f CAN frame (0x273) to modify.
 * @param enable True to activate, false to deactivate.
 */
inline void setFrontFogSwitch(Frame &f, bool enable)
{
	if (f.dlc < 1)
		return;
	if (enable)
		f.data[0] |= 0x08; // bit 3
	else
		f.data[0] &= ~0x08;
}

/**
 * @brief Set the rear fog light switch bit in a UI_vehicleControl frame.
 * @param f CAN frame (0x273) to modify.
 * @param enable True to activate, false to deactivate.
 */
inline void setRearFogSwitch(Frame &f, bool enable)
{
	if (f.dlc < 3)
		return;
	if (enable)
		f.data[2] |= 0x80; // bit 23
	else
		f.data[2] &= ~0x80;
}

/**
 * @brief Set the automatic high beam switch bit in a UI_vehicleControl frame.
 * @param f CAN frame (0x273) to modify.
 * @param enable True to activate, false to deactivate.
 */
inline void setAutoHighBeam(Frame &f, bool enable)
{
	if (f.dlc < 6)
		return;
	if (enable)
		f.data[5] |= 0x02; // bit 41
	else
		f.data[5] &= ~0x02;
}

/**
 * @brief Set the ambient lighting switch bit in a UI_vehicleControl frame.
 * @param f CAN frame (0x273) to modify.
 * @param enable True to activate, false to deactivate.
 */
inline void setAmbientLighting(Frame &f, bool enable)
{
	if (f.dlc < 6)
		return;
	if (enable)
		f.data[5] |= 0x01; // bit 40
	else
		f.data[5] &= ~0x01;
}

/**
 * @brief Set the "See You Home" lighting switch bit in a UI_vehicleControl frame.
 * @param f CAN frame (0x273) to modify.
 * @param enable True to activate, false to deactivate.
 */
inline void setSeeYouHomeLighting(Frame &f, bool enable)
{
	if (f.dlc < 4)
		return;
	if (enable)
		f.data[3] |= 0x40; // bit 30
	else
		f.data[3] &= ~0x40;
}

/**
 * @brief Dome light switch positions for the interior dome light control.
 */
enum DomeLightSwitch
{
	DOME_OFF = 0,  // Dome light forced off
	DOME_ON = 1,   // Dome light forced on
	DOME_AUTO = 2  // Dome light controlled automatically by door state
};

/**
 * @brief Set the dome light switch field in a UI_vehicleControl frame.
 * @param f CAN frame (0x273) to modify.
 * @param mode Desired dome light mode.
 */
inline void setDomeLightSwitch(Frame &f, DomeLightSwitch mode)
{
	if (f.dlc < 8)
		return;
	f.data[7] = (f.data[7] & ~0x18) | ((mode & 0x03) << 3); // bits 59-60
}

/**
 * @brief Burst-send a front fog light activation command on BUS_VEHICLE.
 * @param s Vehicle state providing the base control frame.
 */
static void controlFrontFog(State &s)
{
	Frame f = makeCtrlFrame(s);
	setFrontFogSwitch(f, true);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

/**
 * @brief Burst-send a rear fog light activation command on BUS_VEHICLE.
 * @param s Vehicle state providing the base control frame.
 */
static void controlRearFog(State &s)
{
	Frame f = makeCtrlFrame(s);
	setRearFogSwitch(f, true);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

/**
 * @brief Burst-send an automatic high beam activation command on BUS_VEHICLE.
 * @param s Vehicle state providing the base control frame.
 */
static void controlAutoHighBeam(State &s)
{
	Frame f = makeCtrlFrame(s);
	setAutoHighBeam(f, true);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

/**
 * @brief Burst-send an ambient lighting activation command on BUS_VEHICLE.
 * @param s Vehicle state providing the base control frame.
 */
static void controlAmbientLight(State &s)
{
	Frame f = makeCtrlFrame(s);
	setAmbientLighting(f, true);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

/**
 * @brief Burst-send a "See You Home" lighting activation command on BUS_VEHICLE.
 * @param s Vehicle state providing the base control frame.
 */
static void controlHomeLight(State &s)
{
	Frame f = makeCtrlFrame(s);
	setSeeYouHomeLighting(f, true);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

/**
 * @brief Burst-send a dome light mode command on BUS_VEHICLE.
 * @param mode Desired dome light switch position.
 * @param s Vehicle state providing the base control frame.
 */
static void controlDomeLight(DomeLightSwitch mode, State &s)
{
	Frame f = makeCtrlFrame(s);
	setDomeLightSwitch(f, mode);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

/**
 * @brief Execute a lighting command string.
 *
 * Dispatches "light:*" commands to the appropriate control function.
 * Requires the vehicle control frame to be available (hasCtrl).
 *
 * @param cmd Null-terminated command string.
 * @param s Vehicle state to operate on.
 * @return True if the command was recognized and executed.
 */
static bool executeLightCmd(const char *cmd, State &s)
{
	if (!s.hasCtrl)
		return false;

	if (strcmp(cmd, "light:fog:front") == 0)
	{
		controlFrontFog(s);
		return true;
	}
	if (strcmp(cmd, "light:fog:rear") == 0)
	{
		controlRearFog(s);
		return true;
	}
	if (strcmp(cmd, "light:highbeam:auto") == 0)
	{
		controlAutoHighBeam(s);
		return true;
	}
	if (strcmp(cmd, "light:ambient") == 0)
	{
		controlAmbientLight(s);
		return true;
	}
	if (strcmp(cmd, "light:home") == 0)
	{
		controlHomeLight(s);
		return true;
	}
	if (strcmp(cmd, "light:dome:off") == 0)
	{
		controlDomeLight(DOME_OFF, s);
		return true;
	}
	if (strcmp(cmd, "light:dome:on") == 0)
	{
		controlDomeLight(DOME_ON, s);
		return true;
	}
	if (strcmp(cmd, "light:dome:auto") == 0)
	{
		controlDomeLight(DOME_AUTO, s);
		return true;
	}
	return false;
}
