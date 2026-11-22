#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/fsd/auto_lane_change.h
 * @brief Automatic lane change (ALC) confirmation via stalk or button injection
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "vehicle/can/ids.h"
#include "vehicle/can/crc8.h"
#include "core/util/parse.h"

/**
 * @brief Known DAS lane change states from 0x39B byte[4] bits[4:0].
 */
static constexpr uint8_t ALC_STATE_IDLE = 0;	  // No lane change activity
static constexpr uint8_t ALC_STATE_REQUESTED = 1; // Lane change prompted, awaiting confirmation
static constexpr uint8_t ALC_STATE_STARTED = 2;	  // Lane change in progress

// Throttle interval: don't re-confirm more than once per 2000ms
static constexpr unsigned long ALC_CONFIRM_COOLDOWN_MS = 2000;

/**
 * @brief Extract DAS_laneChangeState from a 0x39B frame.
 * @param f CAN frame from DAS (ID 0x39B).
 * @return 5-bit lane change state value, or 0 if frame too short.
 */
inline uint8_t readDasLaneChangeState(const Frame &f)
{
	if (f.dlc < 5)
		return 0;
	return f.data[4] & 0x1F; // Bits[4:0] of byte[4]
}

/**
 * @brief Build a Model 3/Y SCCM left-stalk injection frame (0x249).
 *
 * Frame shape re-derived from the uhi22-tesla-crc 0x249 capture corpus
 * (870 accepted 4-byte frames; docs/legacy/uhi22-tesla-crc.md §2/§7/§8):
 *   data[0]      = CRC-8/OPENSAFETY over {data[1] & 0xF0, data[2], data[3], 0x00}
 *                  XOR MAGIC_0x249[data[1] & 0x0F]
 *   data[1][3:0] = alive counter (excluded from the CRC input window)
 *   data[1][7:4] = 0 (idle high-beam / wash-wipe)
 *   data[2][3:0] = turn-stalk enum: 0x08 = full left (DOWN), 0x04 = full right (UP)
 *   data[3]      = 0x00 (0x00 in every captured frame)
 *
 * The old builder emitted DLC 3 with stalk bits in byte[1] bits[7:6] and left
 * the counter nibble INSIDE the CRC window — that shape fits 0/870 corpus frames.
 * Stalk enum values follow the community DBC / in-flight rewriter cross-reference
 * (docs/legacy/canhackers-jupiter.md §7, docs/legacy/joshwardell-model3dbc.md §10);
 * real-vehicle acceptance of the injected frame remains UNVERIFIED.
 *
 * @param f Output frame to populate.
 * @param turnLeft True for left lane change (full-down), false for right (full-up).
 */
static uint8_t _alcStalkCounter = 0;

inline void buildStalkFrame(Frame &f, bool turnLeft)
{
	f.id = CAN_ID_DI_STEER;
	f.dlc = 4;
	_alcStalkCounter = (_alcStalkCounter + 1) & 0x0F;

	f.data[1] = _alcStalkCounter;	// idle high nibble; low nibble = alive counter
	f.data[2] = turnLeft ? 0x08 : 0x04; // turn-stalk enum, low nibble only
	f.data[3] = 0x00;					// tail byte: 0x00 in every captured frame

	// Counter nibble is masked out of the CRC input window (capture-derived rule).
	uint8_t payload[3] = {static_cast<uint8_t>(f.data[1] & 0xF0), f.data[2], f.data[3]};
	f.data[0] = teslaCrc8(payload, 3, _alcStalkCounter, MAGIC_0x249);
}

/**
 * @brief Build a Palladium/Yoke button injection frame (0x3C2 VCLEFT_switchStatus).
 *
 * Mux A frame (byte[0]=0x29), 8 bytes. Encodes left or right turn button press.
 *
 * @param f Output frame to populate.
 * @param turnLeft True for left turn button, false for right.
 */
inline void buildPalladiumTurnFrame(Frame &f, bool turnLeft)
{
	f.id = CAN_ID_VCLEFT_SWITCH;
	f.dlc = 8;
	for (uint8_t i = 0; i < 8; i++)
		f.data[i] = 0x00;
	f.data[0] = 0x29; // Mux A identifier

	if (turnLeft)
	{
		f.data[6] |= 0x81; // Left turn button press pattern
		f.data[7] |= 0x01;
	}
	else
	{
		f.data[6] |= 0x04; // Right turn button press pattern
		f.data[7] |= 0x0C;
	}
}

/**
 * @brief Determine whether an ALC confirmation frame should be sent.
 * @param s Device state (checks enable flag, TX pause, DAS state, cooldown).
 * @param nowMs Current timestamp in milliseconds.
 * @return True if confirmation should be injected now.
 */
inline bool alcShouldConfirm(const State &s, unsigned long nowMs)
{
	if (!s.alcAutoConfirmEnabled)
		return false;
	if (s.dasLaneChangeState != ALC_STATE_REQUESTED)
		return false;
	if ((nowMs - s.alcLastConfirmMs) < ALC_CONFIRM_COOLDOWN_MS)
		return false;
	return true;
}

/**
 * @brief Determine lane change direction from active turn signal state.
 * @param s Device state with turn signal flags.
 * @return -1 for left, 1 for right, 0 if direction is unknown.
 */
inline int8_t alcDirectionFromTurnSignal(const State &s)
{
	if (s.turnSignalLeft && !s.turnSignalRight)
		return -1;
	if (s.turnSignalRight && !s.turnSignalLeft)
		return 1;
	return 0;
}

/**
 * @brief Execute an ALC command string ("alc:on" or "alc:off").
 * @param cmd Command string to parse.
 * @param s Device state to update.
 * @return True if the command was recognized and executed.
 */
static bool executeAlcCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "alc:", 4) != 0)
		return false;
	if (!parseBoolCmd(cmd + 4, s.alcAutoConfirmEnabled, s.alcAutoConfirmEnabled))
		return false;
	resetHandlerLogFlags();
	saveSettings(s);
	return true;
}
