#pragma once
#include "core/forward.h"
#include "vehicle/can/ids.h"
#include "vehicle/can/crc8.h"
#include "core/util/parse.h"

// ── Auto Lane Change (ALC) Confirmation ──────────────────────────────────────
// Monitors DAS_laneChangeState from 0x39B and automatically confirms lane
// changes by injecting the appropriate stalk/button signal.
//
// Model 3/Y (stalk): inject 0x249 SCCM_leftStalk with turn indicator command
// Palladium/Yoke (buttons): inject 0x3C2 VCLEFT_switchStatus button press
//
// DAS_laneChangeState (5-bit from 0x39B):
//   0  = IDLE           - no lane change activity
//   1  = ALC_REQUESTED  - lane change prompted, waiting for confirmation
//   2  = ALC_STARTED    - lane change in progress
//   3+ = various completion/abort states
//
// We inject confirmation when state transitions to ALC_REQUESTED (1).

// Known DAS lane change states
static constexpr uint8_t ALC_STATE_IDLE = 0;
static constexpr uint8_t ALC_STATE_REQUESTED = 1;
static constexpr uint8_t ALC_STATE_STARTED = 2;

// Throttle: don't re-confirm more than once per 2000ms
static constexpr unsigned long ALC_CONFIRM_COOLDOWN_MS = 2000;

// ── DAS Lane Change State Decoder ────────────────────────────────────────────
// Extract DAS_laneChangeState from 0x39B byte[4] bits[4:0] (5-bit field)
// Source: hypery11 fsd_handler.h das_lane_change field
inline uint8_t readDasLaneChangeState(const Frame &f)
{
	if (f.dlc < 5)
		return 0;
	return f.data[4] & 0x1F;
}

// ── Model 3/Y: Stalk Injection via 0x249 ────────────────────────────────────
// SCCM_leftStalk: 3-byte CRC-8 protected frame
// byte[0]: CRC-8 (computed with MAGIC_0x249)
// byte[1]: [7:4]=stalk inputs, [3:0]=alive counter
// byte[2]: wiper/wash bits
//
// Turn signal stalk positions (byte[1] bits[7:6]):
//   0 = OFF, 1 = UP_1 (right), 2 = UP_2, 3 = DOWN_1 (left)
static uint8_t _alcStalkCounter = 0;

inline void buildStalkFrame(Frame &f, bool turnLeft)
{
	f.id = CAN_ID_DI_STEER;
	f.dlc = 3;
	_alcStalkCounter = (_alcStalkCounter + 1) & 0x0F;

	// byte[1]: turn stalk position in bits[7:6], counter in bits[3:0]
	uint8_t stalkBits = turnLeft ? 0xC0 : 0x40; // DOWN_1=left, UP_1=right
	f.data[1] = stalkBits | (_alcStalkCounter & 0x0F);
	f.data[2] = 0x00; // no wiper/wash

	// CRC-8 for 0x249: computed over byte[1..2] with counter
	f.data[0] = teslaCrc8(&f.data[1], 2, _alcStalkCounter, MAGIC_0x249);
}

// ── Palladium/Yoke: Button Injection via 0x3C2 ──────────────────────────────
// VCLEFT_switchStatus: Mux A (byte[0]=0x29), 8-byte frame
// LEFT:  byte[6] |= 0x81, byte[7] |= 0x01
// RIGHT: byte[6] |= 0x04, byte[7] |= 0x0C
inline void buildPalladiumTurnFrame(Frame &f, bool turnLeft)
{
	f.id = CAN_ID_VCLEFT_SWITCH;
	f.dlc = 8;
	for (uint8_t i = 0; i < 8; i++)
		f.data[i] = 0x00;
	f.data[0] = 0x29; // Mux A identifier

	if (turnLeft)
	{
		f.data[6] |= 0x81;
		f.data[7] |= 0x01;
	}
	else
	{
		f.data[6] |= 0x04;
		f.data[7] |= 0x0C;
	}
}

// ── ALC Confirmation Logic ───────────────────────────────────────────────────
// Returns true if a confirmation frame should be sent.
// Caller must determine direction (from DAS state or turn signal context).
inline bool alcShouldConfirm(const State &s, unsigned long nowMs)
{
	if (!s.alcAutoConfirmEnabled)
		return false;
	if (s.txPaused)
		return false;
	if (s.dasLaneChangeState != ALC_STATE_REQUESTED)
		return false;
	if ((nowMs - s.alcLastConfirmMs) < ALC_CONFIRM_COOLDOWN_MS)
		return false;
	return true;
}

// Determine lane change direction from active turn signal state.
// Returns: -1=left, 1=right, 0=unknown
inline int8_t alcDirectionFromTurnSignal(const State &s)
{
	if (s.turnSignalLeft && !s.turnSignalRight)
		return -1;
	if (s.turnSignalRight && !s.turnSignalLeft)
		return 1;
	return 0;
}

// ── ALC Command Handler ─────────────────────────────────────────────────────
// Commands:
//   alc:on  — enable auto lane change confirmation
//   alc:off — disable auto lane change confirmation
bool executeAlcCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "alc:", 4) != 0)
		return false;
	if (!parseBoolCmd(cmd + 4, s.alcAutoConfirmEnabled, s.alcAutoConfirmEnabled))
		return false;
	resetHandlerLogFlags();
	saveSettings(s);
	return true;
}
