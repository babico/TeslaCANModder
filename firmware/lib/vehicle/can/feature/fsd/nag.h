#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/nag.h
 * @brief Nag alert suppression strategies for steering wheel hands-on detection
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <cmath>
#include "core/forward.h"
#include "vehicle/can/ids.h"
#include "core/util/parse.h"

/**
 * @brief Compute the 0x370 EPAS frame checksum.
 * @param data Pointer to the 8-byte frame payload (bytes 0-6 are summed).
 * @return Checksum byte: low byte of (sum of bytes 0-6 + CAN ID bytes).
 */
inline uint8_t nagChecksum(const uint8_t *data)
{
	uint16_t sum = 0;
	for (uint8_t i = 0; i < 7; i++)
		sum += data[i];
	sum += (CAN_ID_EPAS_TORQUE & 0xFF);       // Low byte of 0x370 = 0x70
	sum += ((CAN_ID_EPAS_TORQUE >> 8) & 0xFF); // High byte of 0x370 = 0x03
	return (uint8_t)(sum & 0xFF);
}

/**
 * @brief Deterministic xorshift32 PRNG state shared across all nag strategies.
 */
static uint32_t _nagPrngState = 2463534242UL;

/**
 * @brief Advance the xorshift32 PRNG and return the next pseudo-random value.
 * @return 32-bit pseudo-random number.
 */
inline uint32_t _nagXorshift()
{
	_nagPrngState ^= _nagPrngState << 13;
	_nagPrngState ^= _nagPrngState >> 17;
	_nagPrngState ^= _nagPrngState << 5;
	return _nagPrngState;
}

/**
 * @brief Generate a uniform random float in [0, 1) from the xorshift PRNG.
 * @return Float in the range [0.0, 1.0).
 */
inline float _nagRandFloat()
{
	// Use 24 bits for mantissa precision
	return (_nagXorshift() & 0xFFFFFF) / 16777216.0f;
}

/**
 * @brief Generate a Gaussian-distributed random value using Box-Muller transform.
 * @param sigma Standard deviation of the distribution.
 * @return Random sample from N(0, sigma).
 */
inline float _nagGaussian(float sigma)
{
	float u1 = _nagRandFloat();
	float u2 = _nagRandFloat();
	if (u1 < 1e-7f)
		u1 = 1e-7f; // Avoid log(0)
	float z = std::sqrt(-2.0f * std::log(u1)) * std::cos(6.2831853f * u2);
	return z * sigma;
}

/**
 * @brief Read DAS_autopilotHandsOnState from 0x370 byte 5 bits[5:2].
 * @param f Reference to the CAN frame.
 * @return 4-bit hands-on state value (0-15).
 */
inline uint8_t readDasHandsOnState(const Frame &f)
{
	return f.dlc >= 6 ? ((f.data[5] >> 2) & 0x0F) : 0;
}

/**
 * @brief Check if DAS is actively requesting hands-on confirmation.
 * @param state The DAS hands-on state value (0-15).
 * @return True if state indicates active hands-on request (states 2-7, 9, 10).
 */
inline bool dasHandsOnRequested(uint8_t state)
{
	return (state >= 2 && state <= 7) || state == 9 || state == 10;
}

/**
 * @brief Check if the DAS state is in the strong escalation group.
 * @param state The DAS hands-on state value.
 * @return True for states 3, 4, or 5 (high-urgency nag levels).
 */
inline bool nagIsStrongState(uint8_t state)
{
	return state == 3 || state == 4 || state == 5;
}

/**
 * @brief Apply zero-torque echo to a 0x370 frame (legacy/safe strategies).
 * @param f Mutable reference to the CAN frame to modify in place.
 */
inline void nagApplyZeroTorque(Frame &f)
{
	if (f.dlc < 8)
		return;
	// Increment counter on byte 1 lower nibble (legacy frame shape)
	uint8_t counter = (f.data[1] & 0x0F);
	counter = (counter + 1) & 0x0F;
	f.data[1] = (f.data[1] & 0xF0) | counter;
	// Zero torque signals "no steering input needed" to EPAS
	f.data[2] = 0x00;
	f.data[3] = 0x00;
	// Set handsOnLevel = 1 (DETECTED) in byte 4 bits[7:6]
	f.data[4] = (f.data[4] & ~0xC0u) | 0x40u;
	f.data[7] = nagChecksum(f.data);
}

/**
 * @brief Compute natural-strategy torque mimicking sub-gram hand tremor.
 * @param steeringAngleDeg Current steering angle in degrees.
 * @param dasHandsOnLevel Current DAS escalation level.
 * @return Torque value in Nm, clamped to [0.08, 0.18].
 */
inline float nagNaturalTorque(float steeringAngleDeg, uint8_t dasHandsOnLevel)
{
	float base = 0.13f;
	float sigma = 0.015f + (dasHandsOnLevel * 0.003f);
	float torque = base + _nagGaussian(sigma);
	// Bias against rack tension direction for realism
	if (steeringAngleDeg > 2.0f || steeringAngleDeg < -2.0f)
	{
		float angleFactor = steeringAngleDeg / 200.0f;
		if (angleFactor > 0.03f)
			angleFactor = 0.03f;
		if (angleFactor < -0.03f)
			angleFactor = -0.03f;
		torque -= angleFactor;
	}
	if (torque < 0.08f)
		torque = 0.08f;
	if (torque > 0.18f)
		torque = 0.18f;
	return torque;
}

/**
 * @brief Compute the next non-linear interval between natural-mode injections.
 * @return Interval in milliseconds, uniformly distributed in [150, 350].
 */
inline uint16_t nagNaturalNextInterval()
{
	return 150 + (_nagXorshift() % 201);
}

/**
 * @brief Apply natural-strategy torque to a 0x370 frame.
 * @param f Mutable reference to the CAN frame.
 * @param torqueNm Torque value in Nm to encode.
 */
inline void nagApplyNaturalTorque(Frame &f, float torqueNm)
{
	if (f.dlc < 8)
		return;
	uint8_t counter = (f.data[1] & 0x0F);
	counter = (counter + 1) & 0x0F;
	f.data[1] = (f.data[1] & 0xF0) | counter;
	// Encode torque as signed 16-bit, scale 0.01 Nm per bit
	int16_t raw = (int16_t)(torqueNm * 100.0f);
	f.data[2] = (uint8_t)((raw >> 8) & 0xFF);
	f.data[3] = (uint8_t)(raw & 0xFF);
	// Set handsOnLevel = 1 (DETECTED)
	f.data[4] = (f.data[4] & ~0xC0u) | 0x40u;
	f.data[7] = nagChecksum(f.data);
}

/**
 * @brief Check if the natural-mode injection interval has elapsed.
 * @param s Mutable reference to the global state (updates timing fields).
 * @param nowMs Current timestamp in milliseconds.
 * @return True if the interval elapsed and mode is NAG_MODE_NATURAL.
 */
inline bool nagNaturalIntervalReady(State &s, unsigned long nowMs)
{
	if (s.nagMode != NAG_MODE_NATURAL)
		return false;
	if ((nowMs - s.naturalNagLastMs) < s.naturalNagIntervalMs)
		return false;
	s.naturalNagLastMs = nowMs;
	s.naturalNagIntervalMs = nagNaturalNextInterval();
	return true;
}

/**
 * @brief Determine if legacy/safe/natural strategies should echo this frame.
 * @param s Const reference to the global state.
 * @return True if the current mode and DAS state permit injection.
 */
inline bool nagFixedOrNaturalShouldEcho(const State &s)
{
	if (s.nagMode == NAG_MODE_LEGACY)
		return true; // Legacy is always-on, no DAS gating
	if (s.nagMode == NAG_MODE_SAFE || s.nagMode == NAG_MODE_NATURAL)
	{
		if (!s.dasSeen)
			return false;
		return dasHandsOnRequested(s.dasHandsOnState);
	}
	return false;
}

#define NAG_ORG_STATE2_PAUSE_MS 2000UL
#define NAG_ORG_STATE1_GRACE_MS 500UL
#define NAG_ORG_STRONG_PAUSE_MS 1000UL
#define NAG_ORG_STRONG_CYCLE_MS 1500UL
#define NAG_ORG_STRONG_RAMP_MS 500UL
#define NAG_ORG_STATE2_HOLD_MS 1000UL
#define NAG_ORG_RAW_CENTER 2048
#define NAG_ORG_RAW_MILD_MIN_POS 2098 // +0.5 Nm
#define NAG_ORG_RAW_MILD_MAX_POS 2248 // +2.0 Nm
#define NAG_ORG_RAW_MILD_MIN_NEG 1848 // -2.0 Nm
#define NAG_ORG_RAW_MILD_MAX_NEG 1998 // -0.5 Nm
#define NAG_ORG_RAW_STRONG_PEAK 210   // 2.1 Nm magnitude in raw units
#define NAG_ORG_RAW_EXC_BASE 2350     // ~3.02 Nm base for grip excursion
#define NAG_ORG_LEVEL2_THRESHOLD 200  // |raw - 2048| >= 200 -> level 2 (2.0 Nm)
#define NAG_ORG_LEVEL1_THRESHOLD 100  // |raw - 2048| >= 100 -> level 1 (1.0 Nm)

/**
 * @brief Check if organic injection is allowed for the current frame.
 * @param s Const reference to the global state.
 * @return True if organic/full mode is active and DAS/AP gates permit injection.
 */
inline bool nagOrganicShouldEcho(const State &s)
{
	if (s.nagMode != NAG_MODE_ORGANIC && s.nagMode != NAG_MODE_FULL)
		return false;
	if (!s.dasSeen)
		return false;
	// AP gate: only inject when Autopilot is actively driving (states 3-6)
	if (s.dasApState < 3 || s.dasApState > 6)
		return false;
	// HandsOn gate: skip idle (0), suspended (8), SNA (15)
	if (s.dasHandsOnState == 0 || s.dasHandsOnState == 8 || s.dasHandsOnState == 15)
		return false;
	// Driver bypass: skip if real driver hands detected on wheel
	if (s.nagOrganicDriverBypass && s.nagOrganicRealHandsOn != 0)
		return false;
	return true;
}

/**
 * @brief Derive handsOnLevel (0/1/2) from absolute torque magnitude.
 * @param raw Raw torque value centered at 2048.
 * @return Level 0, 1, or 2 based on distance from center.
 */
inline uint8_t nagOrgLevelFromRaw(int16_t raw)
{
	int32_t absRaw = raw > NAG_ORG_RAW_CENTER ? raw - NAG_ORG_RAW_CENTER : NAG_ORG_RAW_CENTER - raw;
	if (absRaw >= NAG_ORG_LEVEL2_THRESHOLD)
		return 2;
	if (absRaw >= NAG_ORG_LEVEL1_THRESHOLD)
		return 1;
	return 0;
}

/**
 * @brief Reset organic state machine timers on DAS hands-on state transitions.
 * @param s Mutable reference to the global state.
 * @param nowMs Current timestamp in milliseconds.
 */
inline void nagOrganicOnStateChange(State &s, unsigned long nowMs)
{
	uint8_t prev = s.nagOrganicPrevState;
	uint8_t next = s.dasHandsOnState;

	// Entering state 1: capture last output for grace hold period
	if (prev != 1 && next == 1)
	{
		s.nagOrg1EnterMs = nowMs;
		s.nagOrg1HoldRaw = s.nagOrgLastRaw;
		s.nagOrg1HoldLevel = s.nagOrgLastLevel;
	}
	if (next != 1)
	{
		s.nagOrg1EnterMs = 0;
		s.nagOrg1HoldRaw = NAG_ORG_RAW_CENTER;
		s.nagOrg1HoldLevel = 0;
	}

	// Entering state 2: start the initial pause before random walk
	if (prev != 2 && next == 2)
	{
		s.nagOrg2EnterMs = nowMs;
	}
	if (next != 2)
	{
		s.nagOrg2EnterMs = 0;
		s.nagOrg2HoldUntilMs = 0;
		s.nagOrg2HoldRaw = NAG_ORG_RAW_CENTER;
		s.nagOrg2HoldLevel = 0;
		s.nagOrg2Level2Active = false;
	}

	// Entering strong group (3/4/5): start the initial pause before ramp
	if (!nagIsStrongState(prev) && nagIsStrongState(next))
	{
		s.nagOrgStrongEnterMs = nowMs;
	}
	if (!nagIsStrongState(next))
	{
		s.nagOrgStrongEnterMs = 0;
	}
}

/**
 * @brief Compute state 2 torque using a random walk opposite to steering direction.
 * @param s Mutable reference to the global state (updates walk position).
 * @param nowMs Current timestamp in milliseconds.
 * @return Raw torque value centered at 2048.
 */
inline int16_t nagOrgComputeState2Torque(State &s, unsigned long nowMs)
{
	// Walk in the half-range opposite to current steering direction
	int16_t minRaw = s.steeringAngle > 0.0f ? NAG_ORG_RAW_MILD_MIN_NEG : NAG_ORG_RAW_MILD_MIN_POS;
	int16_t maxRaw = s.steeringAngle > 0.0f ? NAG_ORG_RAW_MILD_MAX_NEG : NAG_ORG_RAW_MILD_MAX_POS;

	if (s.nagOrg2WalkRaw < minRaw || s.nagOrg2WalkRaw > maxRaw)
		s.nagOrg2WalkRaw = (int16_t)((minRaw + maxRaw) / 2);

	int16_t step = (int16_t)(_nagXorshift() % 25) - 12;
	s.nagOrg2WalkRaw = (int16_t)(s.nagOrg2WalkRaw + step);
	if (s.nagOrg2WalkRaw < minRaw)
		s.nagOrg2WalkRaw = minRaw;
	if (s.nagOrg2WalkRaw > maxRaw)
		s.nagOrg2WalkRaw = maxRaw;

	// Hold at current value if within a level-2 hold period
	if (nowMs < s.nagOrg2HoldUntilMs)
		return s.nagOrg2HoldRaw;

	bool level2Now = nagOrgLevelFromRaw(s.nagOrg2WalkRaw) >= 2;
	if (level2Now && !s.nagOrg2Level2Active)
	{
		s.nagOrg2HoldUntilMs = nowMs + NAG_ORG_STATE2_HOLD_MS;
		s.nagOrg2HoldRaw = s.nagOrg2WalkRaw;
		s.nagOrg2HoldLevel = 2;
	}
	s.nagOrg2Level2Active = level2Now;
	return s.nagOrg2WalkRaw;
}

/**
 * @brief Compute strong group (3/4/5) torque: ramp to peak then hold, cycling.
 * @param s Const reference to the global state.
 * @param nowMs Current timestamp in milliseconds.
 * @return Raw torque value centered at 2048.
 */
inline int16_t nagOrgComputeStrongTorque(const State &s, unsigned long nowMs)
{
	unsigned long sinceEnter = nowMs - s.nagOrgStrongEnterMs;
	if (sinceEnter < NAG_ORG_STRONG_PAUSE_MS)
		return NAG_ORG_RAW_CENTER;
	unsigned long activeMs = sinceEnter - NAG_ORG_STRONG_PAUSE_MS;
	// Cycle: ramp over 500 ms then hold for remainder of 1500 ms period
	unsigned long phase = activeMs % NAG_ORG_STRONG_CYCLE_MS;
	int16_t magnitude;
	if (phase < NAG_ORG_STRONG_RAMP_MS)
		magnitude = (int16_t)((phase * NAG_ORG_RAW_STRONG_PEAK) / NAG_ORG_STRONG_RAMP_MS);
	else
		magnitude = NAG_ORG_RAW_STRONG_PEAK;
	// Apply opposite to steering direction
	int16_t sign = s.steeringAngle > 0.0f ? -1 : 1;
	return (int16_t)(NAG_ORG_RAW_CENTER + sign * magnitude);
}

/**
 * @brief Apply grip excursion overlay: brief high-magnitude pulse every 125-225 frames.
 * @param s Mutable reference to the global state (tracks excursion countdown).
 * @param baseRaw Base torque value to potentially override.
 * @return Final raw torque value (excursion pulse or unmodified base).
 */
inline int16_t nagOrgApplyGripExcursion(State &s, int16_t baseRaw)
{
	if (s.nagOrgExcFrames > 0)
	{
		s.nagOrgExcFrames--;
		// Excursion magnitude: ~3.0-3.4 Nm pulse
		int16_t magnitude = (int16_t)(NAG_ORG_RAW_EXC_BASE + (int16_t)(_nagXorshift() % 41) - 20 -
									  NAG_ORG_RAW_CENTER);
		int16_t sign = baseRaw >= NAG_ORG_RAW_CENTER ? 1 : -1;
		return (int16_t)(NAG_ORG_RAW_CENTER + sign * magnitude);
	}
	if (s.nagOrgFramesUntilExc > 0)
	{
		s.nagOrgFramesUntilExc--;
		return baseRaw;
	}
	// Schedule next excursion: 3-5 frame pulse after 125-225 frame gap
	uint32_t r = _nagXorshift();
	s.nagOrgExcFrames = (uint8_t)(3 + (r % 3));
	s.nagOrgFramesUntilExc = (uint16_t)(125 + (r % 101));
	int16_t magnitude = (int16_t)(NAG_ORG_RAW_EXC_BASE + (int16_t)((r >> 8) % 41) - 20 -
								  NAG_ORG_RAW_CENTER);
	int16_t sign = baseRaw >= NAG_ORG_RAW_CENTER ? 1 : -1;
	s.nagOrgExcFrames--;
	return (int16_t)(NAG_ORG_RAW_CENTER + sign * magnitude);
}

/**
 * @brief Per-frame organic state machine dispatch.
 * @param s Mutable reference to the global state.
 * @param nowMs Current timestamp in milliseconds.
 * @return True if the frame should be echoed with generated torque values.
 */
inline bool nagOrganicTick(State &s, unsigned long nowMs)
{
	if (!nagOrganicShouldEcho(s))
		return false;

	uint8_t state = s.dasHandsOnState;

	// State 1: grace hold from prior active state for 500 ms
	if (state == 1)
	{
		if (s.nagOrg1EnterMs != 0 && (nowMs - s.nagOrg1EnterMs) < NAG_ORG_STATE1_GRACE_MS)
		{
			s.nagOrgLastRaw = s.nagOrg1HoldRaw;
			s.nagOrgLastLevel = s.nagOrg1HoldLevel;
			return true;
		}
		return false;
	}

	// State 2: random walk after initial pause
	if (state == 2)
	{
		if (s.nagOrg2EnterMs != 0 && (nowMs - s.nagOrg2EnterMs) < NAG_ORG_STATE2_PAUSE_MS)
			return false;
		int16_t raw = nagOrgComputeState2Torque(s, nowMs);
		raw = nagOrgApplyGripExcursion(s, raw);
		s.nagOrgLastRaw = raw;
		s.nagOrgLastLevel = nagOrgLevelFromRaw(raw);
		return true;
	}

	// States 3/4/5: strong ramp cycle after initial pause
	if (nagIsStrongState(state))
	{
		if (s.nagOrgStrongEnterMs != 0 && (nowMs - s.nagOrgStrongEnterMs) < NAG_ORG_STRONG_PAUSE_MS)
			return false;
		int16_t raw = nagOrgComputeStrongTorque(s, nowMs);
		raw = nagOrgApplyGripExcursion(s, raw);
		s.nagOrgLastRaw = raw;
		s.nagOrgLastLevel = nagOrgLevelFromRaw(raw);
		return true;
	}

	return false;
}

/**
 * @brief Apply organic torque and handsOnLevel to a captured 0x370 frame.
 * @param f Mutable reference to the CAN frame.
 * @param s Const reference to the global state containing computed torque values.
 */
inline void nagOrganicApply(Frame &f, const State &s)
{
	if (f.dlc < 8)
		return;

	// Counter on byte 6 lower nibble (organic uses byte 6, not byte 1)
	uint8_t cnt = (f.data[6] & 0x0F);
	cnt = (cnt + 1) & 0x0F;
	f.data[6] = (f.data[6] & 0xF0) | cnt;

	// Torque: Motorola 19|12@0+ encoding in byte 2 low nibble + byte 3
	uint16_t tRaw = (uint16_t)(s.nagOrgLastRaw < 0 ? 0 : s.nagOrgLastRaw);
	if (tRaw > 0x0FFF)
		tRaw = 0x0FFF;
	f.data[2] = (f.data[2] & 0xF0) | ((tRaw >> 8) & 0x0F);
	f.data[3] = (uint8_t)(tRaw & 0xFF);

	// handsOnLevel in byte 4 bits[7:6]
	f.data[4] = (f.data[4] & ~0xC0u) | ((s.nagOrgLastLevel & 0x03u) << 6);

	f.data[7] = nagChecksum(f.data);
}

/**
 * @brief Execute a nag command (mode selection or bypass toggle).
 * @param cmd Raw command string starting with "nag:".
 * @param s Mutable reference to the global state.
 * @return True if the command was recognized and applied successfully.
 */
static bool executeNagCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "nag:", 4) != 0)
		return false;
	if (!s.features().nag)
		return false;

	const char *tail = cmd + 4;

	if (strncmp(tail, "mode:", 5) == 0)
	{
		NagMode mode;
		if (!parseNagMode(tail + 5, mode))
			return false;
		s.nagMode = mode;
		resetHandlerLogFlags();
		saveSettings(s);
		applyFilters(s);
		return true;
	}

	if (strncmp(tail, "bypass:", 7) == 0)
	{
		if (!parseBoolCmd(tail + 7, s.nagOrganicDriverBypass, s.nagOrganicDriverBypass))
			return false;
		resetHandlerLogFlags();
		saveSettings(s);
		return true;
	}

	return false;
}
