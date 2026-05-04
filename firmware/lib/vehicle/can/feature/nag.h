#pragma once
#include <cmath>
#include "core/forward.h"
#include "vehicle/can/ids.h"
#include "infra/util/parse.h"

// ── Nag Killer — EPAS Torque Spoofing ────────────────────────────────────────
// Echoes CAN 0x370 (EPAS3P_sysStatus) with counter+1 and zeroed torque request.
// This convinces Autopilot that hands are on the wheel, suppressing the
// "Apply pressure to steering wheel" nag alert.
// Sources: ev-open-can-tools, hypery11-flipper, tesla-fsd.netlify.app.
//
// Checksum algorithm: sum of bytes 0-6 + CAN ID low byte (0x70) + high byte (0x03)
//
// Modes:
//   legacy  — always echo with zeroed torque when enabled
//   safe    — echo only when DAS actively requests hands-on
//   natural — dynamic Gaussian jitter on torque (0.08–0.18 Nm) with steering
//             angle feedback and non-linear interval to evade DAS heuristics

// Compute checksum for the 0x370 EPAS frame (sum of first 7 data bytes + CAN ID bytes)
inline uint8_t nagKillerChecksum(const uint8_t *data)
{
	uint16_t sum = 0;
	for (uint8_t i = 0; i < 7; i++)
		sum += data[i];
	sum += (CAN_ID_EPAS_TORQUE & 0xFF);		   // 0x70
	sum += ((CAN_ID_EPAS_TORQUE >> 8) & 0xFF); // 0x03
	return (uint8_t)(sum & 0xFF);
}

// Modify a captured 0x370 frame: increment counter, zero torque, recalculate checksum
inline void nagKillerModify(Frame &f)
{
	if (f.dlc < 8)
		return;
	// Increment rolling counter in byte 1 lower nibble (wraps 0-15)
	uint8_t counter = f.data[1] & 0x0F;
	counter = (counter + 1) & 0x0F;
	f.data[1] = (f.data[1] & 0xF0) | counter;
	// Zero the torque request in bytes 2-3 (tells EPAS no steering input needed)
	f.data[2] = 0x00;
	f.data[3] = 0x00;
	// Set handsOnLevel=1 (DETECTED) in byte 4 bits[7:6].
	// Fix (hypery11 v2.11): OR-ing 0x40 without clearing leaves level=3 unchanged on
	// escalated frames. Mask first so any incoming level is replaced cleanly.
	f.data[4] = (f.data[4] & ~0xC0u) | 0x40u;
	// Recalculate checksum in byte 7 to match modified payload
	f.data[7] = nagKillerChecksum(f.data);
}

// ── Natural Nag — Gaussian Jitter Algorithm ──────────────────────────────────
// Simulates realistic human hand tremor on steering torque.
// Uses Box-Muller transform for Gaussian noise, modulated by steering angle
// to complement rack tension direction. Non-linear injection intervals
// prevent DAS detection heuristics from identifying periodic patterns.

// Simple deterministic PRNG (xorshift32) — no need for crypto quality
static uint32_t _nagPrngState = 2463534242UL;
inline uint32_t _nagXorshift()
{
	_nagPrngState ^= _nagPrngState << 13;
	_nagPrngState ^= _nagPrngState >> 17;
	_nagPrngState ^= _nagPrngState << 5;
	return _nagPrngState;
}

// Return pseudo-random float in [0.0, 1.0)
inline float _nagRandFloat()
{
	return (_nagXorshift() & 0xFFFFFF) / 16777216.0f;
}

// Box-Muller Gaussian: mean=0, sigma given. Returns one sample.
inline float _nagGaussian(float sigma)
{
	float u1 = _nagRandFloat();
	float u2 = _nagRandFloat();
	if (u1 < 1e-7f)
		u1 = 1e-7f; // avoid log(0)
	float z = std::sqrt(-2.0f * std::log(u1)) * std::cos(6.2831853f * u2);
	return z * sigma;
}

// Compute a natural torque value (Nm) based on Gaussian jitter + steering feedback.
// Range clamped to [0.08, 0.18] Nm — realistic hand tremor magnitude.
// Direction adapts to complement the steering angle (counter rack tension).
inline float nagNaturalTorque(float steeringAngleDeg, uint8_t dasHandsOnLevel)
{
	// Base torque: center of 0.08–0.18 range = 0.13 Nm
	float base = 0.13f;
	// Sigma scales with DAS escalation: higher urgency → more convincing tremor
	float sigma = 0.015f + (dasHandsOnLevel * 0.003f);
	float jitter = _nagGaussian(sigma);
	float torque = base + jitter;

	// Steering angle feedback: slight bias opposing rack tension
	// At 10° right turn, shift torque slightly leftward (negative direction)
	if (steeringAngleDeg > 2.0f || steeringAngleDeg < -2.0f)
	{
		float angleFactor = steeringAngleDeg / 200.0f; // small proportional bias
		if (angleFactor > 0.03f)
			angleFactor = 0.03f;
		if (angleFactor < -0.03f)
			angleFactor = -0.03f;
		torque -= angleFactor;
	}

	// Clamp to realistic hand tremor range
	if (torque < 0.08f)
		torque = 0.08f;
	if (torque > 0.18f)
		torque = 0.18f;
	return torque;
}

// Compute next non-linear injection interval (ms).
// Varies between 150–350ms to prevent periodic detection.
inline uint16_t nagNaturalNextInterval()
{
	return 150 + (_nagXorshift() % 201); // [150, 350]
}

// Modify a captured 0x370 frame for natural mode: set dynamic torque, inc counter.
// Torque encoding: bytes 2-3 = signed 16-bit, scale factor 0.01 Nm.
//   slxslx reference: data[2] lower nibble = high, data[3] = low → raw = Nm * 100
inline void nagKillerModifyNatural(Frame &f, float torqueNm)
{
	if (f.dlc < 8)
		return;
	// Increment rolling counter
	uint8_t counter = f.data[1] & 0x0F;
	counter = (counter + 1) & 0x0F;
	f.data[1] = (f.data[1] & 0xF0) | counter;

	// Encode torque: signed 16-bit, scale 0.01 Nm
	int16_t raw = (int16_t)(torqueNm * 100.0f);
	f.data[2] = (uint8_t)((raw >> 8) & 0xFF);
	f.data[3] = (uint8_t)(raw & 0xFF);
	// Set handsOnLevel=1 (DETECTED) in byte 4 bits[7:6] (same fix as legacy mode)
	f.data[4] = (f.data[4] & ~0xC0u) | 0x40u;

	// Recalculate checksum
	f.data[7] = nagKillerChecksum(f.data);
}

// Read DAS hands-on-wheel request state from 0x370 byte 5 bits [5:2]
inline uint8_t readDasHandsOnState(const Frame &f)
{
	if (f.dlc < 6)
		return 0;
	return (f.data[5] >> 2) & 0x0F;
}

// Check if the DAS state indicates an active hands-on-wheel request.
// States 2-7, 9, 10 = hands-on requested. States 0, 8 = no request.
inline bool dasHandsOnRequested(uint8_t state)
{
	return ((state >= 2 && state <= 7) || state == 9 || state == 10);
}

// Determine if the nag killer should echo the spoofed frame.
// Legacy mode: always echo when enabled.
// Safe mode: echo only when DAS actively requests hands on the wheel.
// Natural mode: uses safe-mode gating + non-linear interval timing.
inline bool nagKillerShouldEcho(const State &s)
{
	if (!s.nagKillerEnabled)
		return false;
	if (s.nagKillerMode == NAG_KILLER_LEGACY)
		return true;
	// Both safe and natural modes require DAS feedback
	if (!s.dasSeen)
		return false;
	return dasHandsOnRequested(s.dasHandsOnState);
}

// Check if natural mode interval has elapsed (call from dispatch)
inline bool nagNaturalIntervalReady(State &s, unsigned long nowMs)
{
	if (s.nagKillerMode != NAG_KILLER_NATURAL)
		return false;
	if ((nowMs - s.naturalNagLastMs) < s.naturalNagIntervalMs)
		return false;
	s.naturalNagLastMs = nowMs;
	s.naturalNagIntervalMs = nagNaturalNextInterval();
	return true;
}

// ── Nag Suppress Command ─────────────────────────────────────────────────────
// Toggles bit-19 nag suppression (lighter approach than full torque spoofing).
// Command: nag:on / nag:off
bool executeNagCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "nag:", 4) == 0)
	{
		if (!s.features().nag)
			return false;
		if (!parseBoolCmd(cmd + 4, s.nagSuppress, s.nagSuppress))
			return false;
		resetHandlerLogFlags();
		saveSettings(s);
		applyFilters(s);
		return true;
	}
	return false;
}

// ── Nag Killer Command ───────────────────────────────────────────────────────
// Commands:
//   nag:killer:on       — enable EPAS torque spoofing
//   nag:killer:off      — disable EPAS torque spoofing
//   nag:killer:mode:legacy  — always echo when enabled
//   nag:killer:mode:safe    — echo only when DAS requests hands-on
//   nag:killer:mode:natural — Gaussian jitter with steering angle feedback

bool executeNagKillerCmd(const char *cmd, State &s)
{
	// nag:killer:mode:<legacy|safe> — set echo behavior mode
	if (strncmp(cmd, "nag:killer:mode:", 16) == 0)
	{
		if (!s.features().nag)
			return false;
		NagKillerMode mode;
		if (!parseNagKillerMode(cmd + 16, mode))
			return false;
		s.nagKillerMode = mode;
		resetHandlerLogFlags();
		saveSettings(s);
		return true;
	}

	// nag:killer:on / nag:killer:off — enable/disable torque spoofing
	if (strncmp(cmd, "nag:killer:", 11) == 0)
	{
		if (!s.features().nag)
			return false;
		if (!parseBoolCmd(cmd + 11, s.nagKillerEnabled, s.nagKillerEnabled))
			return false;
		resetHandlerLogFlags();
		saveSettings(s);
		return true;
	}
	return false;
}
