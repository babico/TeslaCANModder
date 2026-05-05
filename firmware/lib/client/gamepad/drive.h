#pragma once
// ── Gamepad Analog Axes → DAS Drive Control ─────────────────────────────────

#include "client/gamepad/api.h"
#include "client/gamepad/axis_math.h"

#if BOARD_ENABLE_BLE

static float gpApplyAxis(uint8_t idx, uint8_t raw)
{
	return gpAxisMath(idx, raw, gpAxisDz[idx], gpAxisExpo[idx],
		(gpAxisInvMask & (1u << idx)) != 0);
}

static void gamepadDriveTick(State &s, unsigned long now)
{
	if (!gpEnabled || !dasDriveIsEnabled())
		return;
	if (!gpConnected)
		return;

	float lx = gpApplyAxis(0, gpAxes[0]);
	float ltN = gpApplyAxis(4, gpAxes[4]);
	float rtN = gpApplyAxis(5, gpAxes[5]);
	if (ltN < 0)
		ltN = -ltN;
	if (rtN < 0)
		rtN = -rtN;

	float steer = lx * 60.0f;
	float brake = ltN * DAS_ACCEL_MIN_MS2;
	float accel = rtN * DAS_ACCEL_MAX_MS2;
	if (brake < -0.01f)
		accel = 0.0f;
	float setSpeed = (brake < -0.01f) ? 0.0f : dasSpeedLimitKph;

	dasSetControl(steer, brake, accel, setSpeed, now);
}

#endif // BOARD_ENABLE_BLE
