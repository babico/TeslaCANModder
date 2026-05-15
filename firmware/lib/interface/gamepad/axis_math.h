#pragma once

/**
 * @file firmware/lib/interface/gamepad/axis_math.h
 * @brief Pure gamepad axis math: deadzone, expo curve, inversion, and normalization
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <stdint.h>

/**
 * @brief Apply deadzone, expo curve, and inversion to a raw axis value
 *
 * Axes 0-3 are sticks (centered at 128, range ±128), axes 4-5 are triggers
 * (start at 0, range 0-255). Returns a normalized float in [-1.0, +1.0] for
 * sticks or [0.0, +1.0] for triggers.
 *
 * @param idx Axis index (0-3 = sticks, 4-5 = triggers)
 * @param raw Raw 8-bit axis value from the HID report
 * @param dzU Deadzone size in raw units (values within ±dz of center are zeroed)
 * @param expoU Expo curve strength (0 = linear, 100 = full cubic response)
 * @param inv true to invert the output sign
 * @return Normalized axis value after deadzone, expo, and inversion
 */
inline float gpAxisMath(uint8_t idx, uint8_t raw, uint8_t dzU, uint8_t expoU, bool inv)
{
	bool trigger = (idx == 4 || idx == 5);  // Triggers have zero-based range
	float center = trigger ? 0.0f : 128.0f;
	float scale = trigger ? 255.0f : 128.0f;
	float v = ((float)raw - center);

	// Apply deadzone: zero out values within the dead band
	float dz = (float)dzU;
	if (v > -dz && v < dz)
		v = 0.0f;
	else
		v = (v > 0 ? v - dz : v + dz);  // Shift value toward zero by deadzone amount

	// Normalize to [-1, +1] using remaining range after deadzone subtraction
	float denom = scale - dz;
	if (denom <= 0.0f)
		denom = 1.0f;  // Prevent division by zero when dz >= scale
	float n = v / denom;
	if (n > 1.0f)
		n = 1.0f;
	if (n < -1.0f)
		n = -1.0f;

	// Apply expo curve: blend between linear and cubic response
	if (expoU > 0)
	{
		float k = (float)expoU / 100.0f;  // Expo blend factor (0.0 = linear, 1.0 = cubic)
		if (k > 1.0f)
			k = 1.0f;
		float sign = (n < 0) ? -1.0f : 1.0f;
		float a = n < 0 ? -n : n;
		n = sign * ((1.0f - k) * a + k * a * a * a);  // Weighted blend: linear*(1-k) + cubic*k
	}

	if (inv)
		n = -n;
	return n;
}
