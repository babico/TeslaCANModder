#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/fsd/nag.h
 * @brief Nag alert suppression strategies for steering wheel hands-on detection
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 *
 * Architecture
 * -----------
 * The nag echo subsystem is built around a three-stage pipeline:
 *
 *   1. `nagEchoShouldEcho(state, mode)` — coarse gate: is this mode eligible to
 *      inject right now (mode active, DAS requesting, AP gate, etc.)?
 *   2. `nagEchoCompute(state, mode, nowMs)` — per-mode strategy: update any
 *      internal state (organic walk, natural interval, feifan walk) and return
 *      a `NagTorque` describing what to write to the frame. Returns
 *      `valid == false` to skip this frame (e.g., organic state machine is in
 *      a hold/pause window).
 *   3. `nagEchoApply(frame, torque)` — common frame mutation: DLC guard, rolling
 *      counter increment, torque encoding, handsOnLevel bit write, checksum.
 *
 * Every prior duplicate helper (`nagApplyZeroTorque`, `nagApplyNaturalTorque`,
 * `nagOrganicApply`, `nagFixedOrNaturalShouldEcho`, `nagOrganicShouldEcho`)
 * was collapsed into one of these three functions. Adding a new scheme is now
 * one case in `nagEchoCompute` and one in `nagEchoShouldEcho` — no new
 * frame-mutation helper, no new copy of the DLC/checksum/handsOnLevel boilerplate.
 *
 * The NagTorque value is mode-agnostic: it describes the *final* frame payload
 * (torque raw value, handsOnLevel, which byte holds the rolling counter, and
 * whether the 12-bit torque slot preserves the original data[2] high nibble).
 * `nagEchoApply` interprets it.
 */

#include <cmath>
#include "core/forward.h"
#include "vehicle/can/ids.h"
#include "vehicle/can/checksum.h"
#include "core/util/parse.h"
#include "vehicle/can/feature/fsd/nag/math.h"

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

// ── Organic state-machine constants ───────────────────────────────────────────
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
#define NAG_ORG_RAW_STRONG_PEAK 210	  // 2.1 Nm magnitude in raw units
#define NAG_ORG_RAW_EXC_BASE 2350	  // ~3.02 Nm base for grip excursion
#define NAG_ORG_LEVEL2_THRESHOLD 200  // |raw - 2048| >= 200 -> level 2 (2.0 Nm)
#define NAG_ORG_LEVEL1_THRESHOLD 100  // |raw - 2048| >= 100 -> level 1 (1.0 Nm)

// ── Feifan-mode constants ─────────────────────────────────────────────────────
// Mirrors the V4.1.00 in-the-wild capture (legacy/hypery11 issue #100):
// handsOnLevel left at 0, signed torque random-walks tightly around 0
// (~ -5..+2 raw = -0.05..+0.02 Nm). Random-walk step is bounded to keep the
// distribution close to the captured one while still being non-deterministic.
#define NAG_FEIFAN_RAW_MIN 2043 // -5 raw from center
#define NAG_FEIFAN_RAW_MAX 2050 // +2 raw from center
#define NAG_FEIFAN_RAW_CENTER 2048

// ── NagTorque: mode-agnostic frame payload descriptor ───────────────────────
/**
 * @brief Describes what `nagEchoApply` should write to a 0x370 frame.
 *
 * `valid` is false when the mode's strategy decided not to echo this frame
 * (e.g., organic state machine in a hold/pause window). The handler must skip
 * the apply step in that case.
 */
struct NagTorque
{
	bool valid;			   // False = skip this frame
	int16_t raw;		   // Signed torque raw value (mode-specific scale)
	uint8_t handsOnLevel;  // 0, 1, or 2
	uint8_t counterByte;   // 1 or 6 — which byte's low nibble holds the rolling counter
	bool preserveData2High;// True: 12-bit torque in data[2] low nibble + data[3],
						   //       preserving data[2] high-nibble status (0x11/0x12).
						   // False: full 16-bit signed torque in data[2:3].
};

/**
 * @brief Coarse eligibility gate for a nag-echo injection.
 * @param s Global state (read-only at this stage).
 * @param mode Active nag mode.
 * @return True if the mode is eligible to inject on the next captured frame.
 *
 * Per-mode timing (natural interval) and fine state-machine decisions
 * (organic pause/grace) are deferred to `nagEchoCompute`.
 */
inline bool nagEchoShouldEcho(const State &s, NagMode mode)
{
	switch (mode)
	{
	case NAG_MODE_LEGACY:
		return true; // always-on, no DAS gating
	case NAG_MODE_SAFE:
	case NAG_MODE_FEIFAN:
		if (!s.dasSeen)
			return false;
		return dasHandsOnRequested(s.dasHandsOnState);
	case NAG_MODE_NATURAL:
		// DAS gate here; per-frame interval check is in nagEchoCompute.
		if (!s.dasSeen)
			return false;
		return dasHandsOnRequested(s.dasHandsOnState);
	case NAG_MODE_ORGANIC:
	case NAG_MODE_FULL:
		// DAS must have been seen at least once.
		if (!s.dasSeen)
			return false;
		// AP gate: only inject when Autopilot is actively driving (states 3-6).
		if (s.dasApState < 3 || s.dasApState > 6)
			return false;
		// HandsOn gate: skip idle (0), suspended (8), SNA (15).
		if (s.dasHandsOnState == 0 || s.dasHandsOnState == 8 || s.dasHandsOnState == 15)
			return false;
		// Driver bypass: skip if real driver hands detected on wheel.
		if (s.nagOrganicDriverBypass && s.nagOrganicRealHandsOn != 0)
			return false;
		return true;
	default:
		return false;
	}
}

// ── Organic state-machine helpers (called by nagEchoCompute) ─────────────────
/**
 * @brief Derive handsOnLevel (0/1/2) from absolute torque magnitude.
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
 */
inline void nagOrganicOnStateChange(State &s, unsigned long nowMs)
{
	uint8_t prev = s.nagOrganicPrevState;
	uint8_t next = s.dasHandsOnState;

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
 */
inline int16_t nagOrgComputeState2Torque(State &s, unsigned long nowMs)
{
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
 */
inline int16_t nagOrgComputeStrongTorque(const State &s, unsigned long nowMs)
{
	unsigned long sinceEnter = nowMs - s.nagOrgStrongEnterMs;
	if (sinceEnter < NAG_ORG_STRONG_PAUSE_MS)
		return NAG_ORG_RAW_CENTER;
	unsigned long activeMs = sinceEnter - NAG_ORG_STRONG_PAUSE_MS;
	unsigned long phase = activeMs % NAG_ORG_STRONG_CYCLE_MS;
	int16_t magnitude;
	if (phase < NAG_ORG_STRONG_RAMP_MS)
		magnitude = (int16_t)((phase * NAG_ORG_RAW_STRONG_PEAK) / NAG_ORG_STRONG_RAMP_MS);
	else
		magnitude = NAG_ORG_RAW_STRONG_PEAK;
	int16_t sign = s.steeringAngle > 0.0f ? -1 : 1;
	return (int16_t)(NAG_ORG_RAW_CENTER + sign * magnitude);
}

/**
 * @brief Apply grip excursion overlay: brief high-magnitude pulse every 125-225 frames.
 */
inline int16_t nagOrgApplyGripExcursion(State &s, int16_t baseRaw)
{
	if (s.nagOrgExcFrames > 0)
	{
		s.nagOrgExcFrames--;
		int16_t magnitude = (int16_t)(NAG_ORG_RAW_EXC_BASE + (int16_t)(_nagXorshift() % 41) - 20 - NAG_ORG_RAW_CENTER);
		int16_t sign = baseRaw >= NAG_ORG_RAW_CENTER ? 1 : -1;
		return (int16_t)(NAG_ORG_RAW_CENTER + sign * magnitude);
	}
	if (s.nagOrgFramesUntilExc > 0)
	{
		s.nagOrgFramesUntilExc--;
		return baseRaw;
	}
	uint32_t r = _nagXorshift();
	s.nagOrgExcFrames = (uint8_t)(3 + (r % 3));
	s.nagOrgFramesUntilExc = (uint16_t)(125 + (r % 101));
	int16_t magnitude = (int16_t)(NAG_ORG_RAW_EXC_BASE + (int16_t)((r >> 8) % 41) - 20 - NAG_ORG_RAW_CENTER);
	int16_t sign = baseRaw >= NAG_ORG_RAW_CENTER ? 1 : -1;
	s.nagOrgExcFrames--;
	return (int16_t)(NAG_ORG_RAW_CENTER + sign * magnitude);
}

/**
 * @brief Per-frame organic state machine dispatch. Returns true if a torque
 *        should be emitted on this frame (state machine is not in a hold/pause).
 */
inline bool nagOrganicTick(State &s, unsigned long nowMs)
{
	uint8_t state = s.dasHandsOnState;

	// State 1: grace hold from prior active state for 500 ms.
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

	// State 2: random walk after initial pause.
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

	// States 3/4/5: strong ramp cycle after initial pause.
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

// ── Natural-mode helpers (called by nagEchoCompute) ───────────────────────────
/**
 * @brief Compute natural-strategy torque mimicking sub-gram hand tremor.
 * @return Torque in Nm, clamped to [0.08, 0.18].
 */
inline float nagNaturalTorque(float steeringAngleDeg, uint8_t dasHandsOnLevel)
{
	float base = 0.13f;
	float sigma = 0.015f + (dasHandsOnLevel * 0.003f);
	float torque = base + _nagGaussian(sigma);
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
 * @brief Check if the natural-mode injection interval has elapsed.
 *        Side effect: updates `naturalNagLastMs` and re-rolls `naturalNagIntervalMs`.
 */
inline bool nagNaturalIntervalReady(State &s, unsigned long nowMs)
{
	if ((nowMs - s.naturalNagLastMs) < s.naturalNagIntervalMs)
		return false;
	s.naturalNagLastMs = nowMs;
	s.naturalNagIntervalMs = 150 + (_nagXorshift() % 201);
	return true;
}

// ── Feifan-mode walk ─────────────────────────────────────────────────────────
/**
 * @brief Advance the feifan random walk by one step and return the new raw value.
 *        Walk is bounded to [NAG_FEIFAN_RAW_MIN, NAG_FEIFAN_RAW_MAX] (= ±5/+2 raw
 *        from center 2048), matching the captured V4.1.00 distribution.
 */
inline int16_t nagFeifanStep(State &s)
{
	if (s.nagFeifanWalkRaw < NAG_FEIFAN_RAW_MIN || s.nagFeifanWalkRaw > NAG_FEIFAN_RAW_MAX)
		s.nagFeifanWalkRaw = NAG_FEIFAN_RAW_CENTER;
	// Step in [-2, +2] raw (range 5) — small, non-deterministic, bounded.
	int16_t step = (int16_t)((int)(_nagXorshift() % 5) - 2);
	s.nagFeifanWalkRaw = (int16_t)(s.nagFeifanWalkRaw + step);
	if (s.nagFeifanWalkRaw < NAG_FEIFAN_RAW_MIN)
		s.nagFeifanWalkRaw = NAG_FEIFAN_RAW_MIN;
	if (s.nagFeifanWalkRaw > NAG_FEIFAN_RAW_MAX)
		s.nagFeifanWalkRaw = NAG_FEIFAN_RAW_MAX;
	return s.nagFeifanWalkRaw;
}

// ── nagEchoCompute: per-mode strategy ────────────────────────────────────────
/**
 * @brief Run the per-mode nag strategy: update any state owned by the mode
 *        (organic state machine, natural interval, feifan walk) and return
 *        the torque descriptor to apply.
 *
 * Returns `valid = false` when the strategy says "skip this frame" (e.g.,
 * organic in a pause window, or natural interval not elapsed).
 */
inline NagTorque nagEchoCompute(State &s, NagMode mode, unsigned long nowMs)
{
	switch (mode)
	{
	case NAG_MODE_LEGACY:
	case NAG_MODE_SAFE:
		// Zero torque, handsOnLevel forced to 1, counter on byte 1.
		// Mirrors the pre-refactor nagApplyZeroTorque output exactly.
		return NagTorque{true, 0, 1, 1, false};

	case NAG_MODE_NATURAL: {
		if (!nagNaturalIntervalReady(s, nowMs))
			return NagTorque{false, 0, 0, 0, false};
		int16_t raw = (int16_t)(nagNaturalTorque(s.steeringAngle, s.dasHandsOnState) * 100.0f);
		return NagTorque{true, raw, 1, 1, false};
	}

	case NAG_MODE_ORGANIC:
	case NAG_MODE_FULL:
		if (s.nagOrganicPrevState != s.dasHandsOnState)
		{
			nagOrganicOnStateChange(s, nowMs);
			s.nagOrganicPrevState = s.dasHandsOnState;
		}
		if (!nagOrganicTick(s, nowMs))
			return NagTorque{false, 0, 0, 0, true};
		return NagTorque{true, s.nagOrgLastRaw, s.nagOrgLastLevel, 6, true};

	case NAG_MODE_FEIFAN: {
		int16_t raw = nagFeifanStep(s);
		// handsOnLevel left at 0 (the preflight-safe invariant from feifan).
		return NagTorque{true, raw, 0, 6, true};
	}

	default:
		return NagTorque{false, 0, 0, 0, false};
	}
}

// ── nagEchoApply: common frame mutation ──────────────────────────────────────
/**
 * @brief Write the NagTorque to a 0x370 frame: DLC guard, rolling counter
 *        increment, torque encoding (12-bit or 16-bit), handsOnLevel bit
 *        write, checksum recalculation.
 */
inline void nagEchoApply(Frame &f, const NagTorque &t)
{
	if (f.dlc < 8)
		return;

	// Rolling counter on the configured byte (1 for legacy/safe/natural, 6 for organic/feifan).
	// Preserve the high nibble of that byte (status / second torque channel).
	uint8_t cnt = (f.data[t.counterByte] & 0x0F) + 1;
	f.data[t.counterByte] = (f.data[t.counterByte] & 0xF0) | (cnt & 0x0F);

	// Torque: 12-bit (preserve data[2] high nibble) or full 16-bit.
	if (t.preserveData2High)
	{
		uint16_t u = (uint16_t)(t.raw < 0 ? 0 : t.raw);
		if (u > 0x0FFF)
			u = 0x0FFF;
		f.data[2] = (f.data[2] & 0xF0) | ((u >> 8) & 0x0F);
		f.data[3] = (uint8_t)(u & 0xFF);
	}
	else
	{
		f.data[2] = (uint8_t)((t.raw >> 8) & 0xFF);
		f.data[3] = (uint8_t)(t.raw & 0xFF);
	}

	// handsOnLevel in byte 4 bits[7:6]. Caller-controlled (0/1/2).
	f.data[4] = (f.data[4] & ~0xC0u) | ((t.handsOnLevel & 0x03u) << 6);

	// Checksum.
	f.data[7] = nagChecksum(f.data);
}

// ── Serial command dispatch ──────────────────────────────────────────────────
/**
 * @brief Execute a nag command (mode selection or bypass toggle).
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
