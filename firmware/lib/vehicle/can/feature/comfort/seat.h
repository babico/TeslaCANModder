#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/comfort/seat.h
 * @brief Seat heating control via CAN UI_vehicleControl frame (0x273)
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "vehicle/can/fwd.h"

/**
 * @brief Seat heating intensity levels
 */
enum SeatHeatLevel
{
	SEAT_OFF = 0,
	SEAT_LOW = 1,
	SEAT_MED = 2,
	SEAT_HIGH = 3
};

/**
 * @brief Set front-left seat heating level in the control frame
 * @param f CAN frame to modify (requires DLC >= 6)
 * @param level Desired heating intensity
 */
inline void setSeatHeatFL(Frame &f, SeatHeatLevel level)
{
	if (f.dlc < 6)
		return;
	f.data[5] = (f.data[5] & ~0x0C) | ((level & 0x03) << 2);	// bits 42-43
}

/**
 * @brief Set front-right seat heating level in the control frame
 * @param f CAN frame to modify (requires DLC >= 6)
 * @param level Desired heating intensity
 */
inline void setSeatHeatFR(Frame &f, SeatHeatLevel level)
{
	if (f.dlc < 6)
		return;
	f.data[5] = (f.data[5] & ~0x30) | ((level & 0x03) << 4);	// bits 44-45
}

/**
 * @brief Set rear-left seat heating level in the control frame
 * @param f CAN frame to modify (requires DLC >= 6)
 * @param level Desired heating intensity
 */
inline void setSeatHeatRL(Frame &f, SeatHeatLevel level)
{
	if (f.dlc < 6)
		return;
	f.data[5] = (f.data[5] & ~0xC0) | ((level & 0x03) << 6);	// bits 46-47
}

/**
 * @brief Set rear-right seat heating level in the control frame
 * @param f CAN frame to modify (requires DLC >= 7)
 * @param level Desired heating intensity
 */
inline void setSeatHeatRR(Frame &f, SeatHeatLevel level)
{
	if (f.dlc < 7)
		return;
	f.data[6] = (f.data[6] & ~0x0C) | ((level & 0x03) << 2);	// bits 50-51
}

/**
 * @brief Set rear-center seat heating level in the control frame
 * @param f CAN frame to modify (requires DLC >= 7)
 * @param level Desired heating intensity
 */
inline void setSeatHeatRC(Frame &f, SeatHeatLevel level)
{
	if (f.dlc < 7)
		return;
	f.data[6] = (f.data[6] & ~0x03) | (level & 0x03);	// bits 48-49
}

/**
 * @brief Send a CAN burst to control heating for a specific seat
 * @param seat Seat index (0=FL, 1=FR, 2=RL, 3=RR, 4=RC)
 * @param level Desired heating intensity
 * @param s Global vehicle state used for frame construction and burst transmission
 */
static void controlSeatHeat(uint8_t seat, SeatHeatLevel level, State &s)
{
	Frame f = makeCtrlFrame(s);

	if (seat == 0)
		setSeatHeatFL(f, level);
	else if (seat == 1)
		setSeatHeatFR(f, level);
	else if (seat == 2)
		setSeatHeatRL(f, level);
	else if (seat == 3)
		setSeatHeatRR(f, level);
	else if (seat == 4)
		setSeatHeatRC(f, level);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

/**
 * @brief Execute a seat heating command
 * @param cmd Command string (e.g. "seat:fl:2", "seat:rr:0")
 * @param s Global vehicle state
 * @return True if the command was recognized and executed
 *
 * @note Format is "seat:<position>:<level>" where position is fl/fr/rl/rr/rc
 *       and level is 0-3 corresponding to SeatHeatLevel values.
 */
static bool executeSeatCmd(const char *cmd, State &s)
{
	if (!s.hasCtrl)
		return false;
	if (strncmp(cmd, "seat:", 5) != 0)
		return false;

	const char *pos = cmd + 5;
	// Level is the last character of the command string
	char lastChar = cmd[strlen(cmd) - 1];
	if (lastChar < '0' || lastChar > '3')
		return false;

	SeatHeatLevel level = (SeatHeatLevel)(lastChar - '0');
	uint8_t seat = 255;

	if (strncmp(pos, "fl:", 3) == 0)
		seat = 0;
	else if (strncmp(pos, "fr:", 3) == 0)
		seat = 1;
	else if (strncmp(pos, "rl:", 3) == 0)
		seat = 2;
	else if (strncmp(pos, "rr:", 3) == 0)
		seat = 3;
	else if (strncmp(pos, "rc:", 3) == 0)
		seat = 4;
	else
		return false;

	controlSeatHeat(seat, level, s);
	return true;
}
