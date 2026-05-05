#pragma once
// ── Pure Gamepad Axis Math ───────────────────────────────────────────────────
// Deadzone + expo + inversion + normalization. No globals, no Arduino/BLE
// dependencies — testable from native unit tests.
//
// Axis layout: 0..3 are sticks (centered at 128, range ±128), 4..5 are
// triggers (start at 0, range 0..255). Returns a normalized float in
// [-1.0, +1.0] for sticks, or [0.0, +1.0] for triggers (callers may then
// flip via `inv`).

#include <stdint.h>

inline float gpAxisMath(uint8_t idx, uint8_t raw, uint8_t dzU, uint8_t expoU, bool inv)
{
	bool trigger = (idx == 4 || idx == 5);
	float center = trigger ? 0.0f : 128.0f;
	float scale = trigger ? 255.0f : 128.0f;
	float v = ((float)raw - center);
	float dz = (float)dzU;
	if (v > -dz && v < dz)
		v = 0.0f;
	else
		v = (v > 0 ? v - dz : v + dz);
	float denom = scale - dz;
	if (denom <= 0.0f)
		denom = 1.0f;
	float n = v / denom;
	if (n > 1.0f)
		n = 1.0f;
	if (n < -1.0f)
		n = -1.0f;
	if (expoU > 0)
	{
		float k = (float)expoU / 100.0f;
		if (k > 1.0f)
			k = 1.0f;
		float sign = (n < 0) ? -1.0f : 1.0f;
		float a = n < 0 ? -n : n;
		n = sign * ((1.0f - k) * a + k * a * a * a);
	}
	if (inv)
		n = -n;
	return n;
}
