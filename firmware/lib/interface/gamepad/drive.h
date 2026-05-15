#pragma once

/**
 * @file firmware/lib/interface/gamepad/drive.h
 * @brief Gamepad analog axes to DAS drive control translation
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "interface/gamepad/api.h"
#include "interface/gamepad/axis_math.h"

#if BOARD_ENABLE_BLE

/**
 * @brief Apply axis tuning (deadzone, expo, inversion) to a raw axis value using stored settings
 * @param idx Axis index (0-5)
 * @param raw Raw 8-bit value from the HID report
 * @return Normalized axis value after tuning
 */
static float gpApplyAxis(uint8_t idx, uint8_t raw)
{
	return gpAxisMath(idx, raw, gpAxisDz[idx], gpAxisExpo[idx],
		(gpAxisInvMask & (1u << idx)) != 0);
}

/**
 * @brief Tick function that maps gamepad axes to DAS steering, brake, and acceleration
 * @param s Current vehicle state reference
 * @param now Current timestamp in milliseconds
 */
static void gamepadDriveTick(State &s, unsigned long now)
{
	if (!gpEnabled || !dasDriveIsEnabled())
		return;
	if (!gpConnected)
		return;

	float lx = gpApplyAxis(0, gpAxes[0]);    // Left stick X → steering
	float ltN = gpApplyAxis(4, gpAxes[4]);    // Left trigger → brake
	float rtN = gpApplyAxis(5, gpAxes[5]);    // Right trigger → acceleration
	if (ltN < 0)
		ltN = -ltN;
	if (rtN < 0)
		rtN = -rtN;

	float steer = lx * 60.0f;                        // Scale to ±60 degrees steering angle
	float brake = ltN * DAS_ACCEL_MIN_MS2;            // Scale trigger to max braking decel
	float accel = rtN * DAS_ACCEL_MAX_MS2;            // Scale trigger to max acceleration
	if (brake < -0.01f)
		accel = 0.0f;                                 // Brake overrides acceleration
	float setSpeed = (brake < -0.01f) ? 0.0f : dasSpeedLimitKph;

	dasSetControl(steer, brake, accel, setSpeed, now);
}

#endif // BOARD_ENABLE_BLE
