#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/body/turn_signal.h
 * @brief Turn signal 3-blink lane change and blind spot decode helpers
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "vehicle/can/fwd.h"

/**
 * @brief Turn signal request values for CAN 0x3F5 byte[0] bits 1:0.
 */
enum TurnSignalRequest
{
	TURN_OFF = 0,     // No signal
	TURN_LEFT_3 = 1,  // Left 3-blink lane change
	TURN_RIGHT_3 = 2, // Right 3-blink lane change
	TURN_HAZARD = 3   // Hazard lights
};

/**
 * @brief Write a turn signal request into byte[0] bits 1:0 of a frame.
 * @param f CAN frame to modify in place.
 * @param req Desired turn signal request value.
 */
inline void setTurnSignalRequest(Frame &f, TurnSignalRequest req)
{
	f.data[0] = (f.data[0] & 0xFC) | ((uint8_t)req & 0x03);
}

/**
 * @brief Inject a turn signal command via CAN 0x3F5 (VCFRONT_vehicleLights).
 *
 * Sends a short burst of 3 frames at 100 ms intervals to trigger
 * one 3-blink lane change cycle.
 *
 * @param s Device state; requires hasCtrl to be set.
 * @param req Turn signal request to inject.
 */
inline void controlTurnSignal(State &s, TurnSignalRequest req)
{
	if (!s.hasCtrl)
		return;
	Frame f;
	f.id = CAN_ID_VCFRONT_LIGHTS;
	f.dlc = 8;
	memset(f.data, 0, 8);
	setTurnSignalRequest(f, req);
	startBurst(s, f, BUS_VEHICLE, 3, 100);
}

/**
 * @brief Execute a turn signal command string.
 *
 * Supported commands: "turn:left3", "turn:right3", "turn:hazard", "turn:off".
 *
 * @param cmd Null-terminated command string.
 * @param s Device state for burst transmission.
 * @return True if the command was recognized and executed.
 */
static bool executeTurnSignalCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "turn:left3") == 0)
	{
		controlTurnSignal(s, TURN_LEFT_3);
		return true;
	}
	if (strcmp(cmd, "turn:right3") == 0)
	{
		controlTurnSignal(s, TURN_RIGHT_3);
		return true;
	}
	if (strcmp(cmd, "turn:hazard") == 0)
	{
		controlTurnSignal(s, TURN_HAZARD);
		return true;
	}
	if (strcmp(cmd, "turn:off") == 0)
	{
		controlTurnSignal(s, TURN_OFF);
		return true;
	}
	return false;
}

/**
 * @brief Decode whether the left turn signal is currently active.
 * @param f Incoming CAN frame (expects DLC >= 7).
 * @return True if left indicator status indicates active blinking.
 */
inline bool decodeTurnSignalLeftActive(const Frame &f)
{
	if (f.dlc < 7)
		return false;
	uint8_t status = (f.data[6] >> 2) & 0x03; // Byte 6 bits 3:2
	return status == 1 || status == 2;
}

/**
 * @brief Decode whether the right turn signal is currently active.
 * @param f Incoming CAN frame (expects DLC >= 7).
 * @return True if right indicator status indicates active blinking.
 */
inline bool decodeTurnSignalRightActive(const Frame &f)
{
	if (f.dlc < 7)
		return false;
	uint8_t status = (f.data[6] >> 4) & 0x03; // Byte 6 bits 5:4
	return status == 1 || status == 2;
}

/**
 * @brief Decode the left blind spot warning level from a CAN frame.
 * @param f Incoming CAN frame (expects DLC >= 1).
 * @return Warning level (0 = none, 1 = caution, 2 = alert), or 0 if invalid.
 */
inline uint8_t decodeBlindSpotLeftLevel(const Frame &f)
{
	if (f.dlc < 1)
		return 0;
	uint8_t level = (f.data[0] >> 4) & 0x03; // Byte 0 bits 5:4
	return level <= 2 ? level : 0;
}

/**
 * @brief Decode the right blind spot warning level from a CAN frame.
 * @param f Incoming CAN frame (expects DLC >= 1).
 * @return Warning level (0 = none, 1 = caution, 2 = alert), or 0 if invalid.
 */
inline uint8_t decodeBlindSpotRightLevel(const Frame &f)
{
	if (f.dlc < 1)
		return 0;
	uint8_t level = (f.data[0] >> 6) & 0x03; // Byte 0 bits 7:6
	return level <= 2 ? level : 0;
}
