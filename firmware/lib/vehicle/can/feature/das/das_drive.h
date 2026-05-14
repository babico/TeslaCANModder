#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/das/das_drive.h
 * @brief DAS Drive — openpilot-style autopilot CAN injection for gamepad-driven longitudinal
 *        and lateral control on BUS_CHASSIS (X179 pins 13-14).
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "core/can/bus.h"
#include "vehicle/can/ids.h"
#include "vehicle/can/checksum.h"
#include <Preferences.h>

// Sends three frames on BUS_CHASSIS (autopilot party CAN):
//   DAS_control         0x2B9 (697)  — longitudinal ACC, 25 Hz
//   DAS_steeringControl 0x488 (1160) — steering angle,   50 Hz
//   APS_eacMonitor      0x27D (637)  — EPAS steer-allow, 10 Hz
//
// HW variant mapping (matches openpilot carcontroller.py):
//   HW3    → DAS_steeringControlType = ANGLE_CONTROL (1)
//   HW4    → DAS_steeringControlType = LANE_KEEP_ASSIST (2)
//   LEGACY → ANGLE_CONTROL (1)
//
// Safety gates (openpilot Tesla CarController port — opendbc/car/tesla):
//   accel_max  ≤  2.0 m/s²       (ACCEL_MAX)
//   accel_min  ≥ -3.48 m/s²      (ACCEL_MIN)
//   speed cap  ≤ runtime dasSpeedCapKph (NVS, default 25, max 200 kph)
//   jerk       ±4.9 m/s³         (fault threshold ±5.0)
//   angle rate ≤ 5°/20 ms frame  (EPS faults at 12)
//   max angle  ≤ 360° absolute, speed-aware lateral-accel clamp above 8 kph
//   standstill brake-hold = -0.4 m/s² when no input near zero speed
//   dead-man   frames stop if dasSetControl() not called within 150 ms
//   cancel     5× DAS_ACC_CANCEL frames before going silent

#define DAS_ACC_ON 4	  // DAS_accState value: ACC engaged
#define DAS_ACC_CANCEL 13 // DAS_accState value: disengage ACC

#define DAS_STEER_NONE 0	   // No steering control
#define DAS_STEER_ANGLE_CTRL 1 // HW3 / LEGACY angle control mode
#define DAS_STEER_LKA 2		   // HW4 FSD14+ lane keep assist mode

#define DAS_CTRL_INTERVAL_MS 40	 // 25 Hz longitudinal frame rate
#define DAS_STEER_INTERVAL_MS 20 // 50 Hz steering frame rate
#define DAS_EAC_INTERVAL_MS 100	 // 10 Hz EPAS allow frame rate
#define DAS_CANCEL_FRAMES 5		 // Number of cancel frames in disengage burst

#define DAS_ACCEL_MAX_MS2 2.0f
#define DAS_ACCEL_MIN_MS2 -3.48f
#define DAS_SPEED_CAP_DEFAULT 25.0f	  // Default safety cap on first boot (NVS-overridden)
#define DAS_SPEED_CAP_MIN_KPH 1.0f	  // Minimum allowed safety cap
#define DAS_SPEED_CAP_MAX_KPH 200.0f  // Absolute compile-time ceiling (DAS_control byte limit)
#define DAS_SPEED_LIMIT_DEFAULT 25.0f // Default user speed limit (NVS-overridden)
#define DAS_JERK_MAX_MS3 4.9f
#define DAS_JERK_MIN_MS3 -4.9f
#define DAS_DEADMAN_MS 150 // Dead-man timeout before auto-cancel

// Steering angle rate limit: 5° per 20 ms frame (openpilot MAX_ANGLE_RATE). EPS faults at 12.
#define DAS_MAX_ANGLE_RATE_DEG 5.0f
// EPAS hard fault above 360° absolute
#define DAS_MAX_ANGLE_DEG 360.0f
// Below this speed the lateral-accel angle clamp is bypassed (full lock for parking)
#define DAS_LOW_SPEED_KPH 8.0f
// Lateral acceleration cap for speed-aware angle limiting (bicycle model approximation)
#define DAS_LAT_ACCEL_MAX_MS2 3.0f
// Tesla Model 3 wheelbase used in bicycle-model angle calculation
#define DAS_WHEELBASE_M 2.875f
// Standstill brake-hold threshold speed
#define DAS_STANDSTILL_KPH 1.5f
// Brake-hold deceleration injected at standstill to prevent creep/roll
#define DAS_STANDSTILL_HOLD_MS2 -0.4f

static bool dasDriveEnabled = false;
static bool dasActive = false;	   // Has live control input from gamepad
static float dasSteerAngle = 0.0f; // Requested steer angle (degrees, signed)
static float dasAccelMin = 0.0f;   // Brake command (m/s², ≤ 0)
static float dasAccelMax = 0.0f;   // Throttle command (m/s², ≥ 0)
static float dasSetSpeedKph = DAS_SPEED_CAP_DEFAULT;
static float dasSpeedCapKph = DAS_SPEED_CAP_DEFAULT;	 // Runtime safety cap (NVS-backed)
static float dasSpeedLimitKph = DAS_SPEED_LIMIT_DEFAULT; // User-configured speed limit
static uint8_t dasCounter3 = 0;							 // DAS_controlCounter [0..7]
static uint8_t dasCounter4 = 0;							 // steeringControlCounter [0..15]
static uint8_t dasEacCounter = 0;						 // APS_eacMonitorCounter [0..15]
static unsigned long dasCtrlLastMs = 0;
static unsigned long dasSteerLastMs = 0;
static unsigned long dasEacLastMs = 0;
static unsigned long dasLastUpdateMs = 0; // Dead-man timer reference
static uint8_t dasCancelCount = 0;		  // Cancel frames remaining to send
static float dasAppliedAngle = 0.0f;	  // Last commanded steer angle (post rate-limit)
static Preferences dasPrefs;

/**
 * @brief Force a 5-frame DAS cancel burst to disengage AP cruise.
 *
 * Safe to call any time; only takes effect if dasDriveEnabled is true
 * so we actually own the chassis bus.
 */
static void dasSendCancelBurst()
{
	dasActive = false;
	dasCancelCount = DAS_CANCEL_FRAMES;
}

/**
 * @brief Build a DAS_control frame (0x2B9, 8 bytes, little-endian).
 *
 * Bit layout:
 *   bits  0-11: DAS_setSpeed (factor 0.1)
 *   bits 12-15: DAS_accState (4=ACC_ON, 13=CANCEL)
 *   bits 16-17: DAS_aebEvent (always 0)
 *   bits 18-26: DAS_jerkMin (factor 0.018, offset -9.1)
 *   bits 27-34: DAS_jerkMax (factor 0.034)
 *   bits 35-43: DAS_accelMin (factor 0.04, offset -15)
 *   bits 44-52: DAS_accelMax (factor 0.04, offset -15)
 *   bits 53-55: DAS_controlCounter
 *   bits 56-63: DAS_controlChecksum
 *
 * @param d Output buffer (8 bytes, zeroed and filled by this function).
 * @param speed_kph Desired set speed in km/h (clamped to safety cap).
 * @param accel_min Minimum acceleration in m/s² (brake, ≤ 0).
 * @param accel_max Maximum acceleration in m/s² (throttle, ≥ 0).
 * @param counter Rolling counter value [0..7].
 * @param active True if ACC is engaged, false to send CANCEL state.
 */
static void buildDasControlFrame(uint8_t *d, float speed_kph, float accel_min, float accel_max, uint8_t counter,
								 bool active)
{
	memset(d, 0, 8);

	// Hard clamp inputs to safe operating range
	if (speed_kph < 0)
		speed_kph = 0;
	if (speed_kph > dasSpeedCapKph)
		speed_kph = dasSpeedCapKph;
	if (accel_min < DAS_ACCEL_MIN_MS2)
		accel_min = DAS_ACCEL_MIN_MS2;
	if (accel_min > 0.0f)
		accel_min = 0.0f;
	if (accel_max < 0.0f)
		accel_max = 0.0f;
	if (accel_max > DAS_ACCEL_MAX_MS2)
		accel_max = DAS_ACCEL_MAX_MS2;

	uint16_t setSpeedRaw = (uint16_t)(speed_kph / 0.1f + 0.5f);
	uint8_t accState = active ? DAS_ACC_ON : DAS_ACC_CANCEL;
	// Fixed jerk limits (safe range, below ±5.0 m/s³ fault threshold)
	uint16_t jerkMinRaw = (uint16_t)((DAS_JERK_MIN_MS3 + 9.1f) / 0.018f + 0.5f);
	uint8_t jerkMaxRaw = (uint8_t)(DAS_JERK_MAX_MS3 / 0.034f + 0.5f);
	uint16_t accelMinRaw = (uint16_t)((accel_min + 15.0f) / 0.04f + 0.5f);
	uint16_t accelMaxRaw = (uint16_t)((accel_max + 15.0f) / 0.04f + 0.5f);

	// Pack little-endian bit fields
	d[0] = (uint8_t)(setSpeedRaw & 0xFF);		  // bits  0-7
	d[1] |= (uint8_t)((setSpeedRaw >> 8) & 0x0F); // bits  8-11
	d[1] |= (uint8_t)((accState & 0x0F) << 4);	  // bits 12-15
	// bits 16-17: aebEvent = 0 (already zero)
	d[2] |= (uint8_t)((jerkMinRaw & 0x3F) << 2);	 // bits 18-23
	d[3] |= (uint8_t)((jerkMinRaw >> 6) & 0x07);	 // bits 24-26
	d[3] |= (uint8_t)((jerkMaxRaw & 0x1F) << 3);	 // bits 27-31
	d[4] |= (uint8_t)((jerkMaxRaw >> 5) & 0x07);	 // bits 32-34
	d[4] |= (uint8_t)((accelMinRaw & 0x1F) << 3);	 // bits 35-39
	d[5] |= (uint8_t)((accelMinRaw >> 5) & 0x0F);	 // bits 40-43
	d[5] |= (uint8_t)((accelMaxRaw & 0x0F) << 4);	 // bits 44-47
	d[6] |= (uint8_t)((accelMaxRaw >> 4) & 0x1F);	 // bits 48-52
	d[6] |= (uint8_t)((counter & 0x07) << 5);		 // bits 53-55
	d[7] = dasChecksum(CAN_ID_DAS_CONTROL, d, 8, 7); // bits 56-63
}

/**
 * @brief Build a DAS_steeringControl frame (0x488, 4 bytes, mixed endianness).
 *
 * Bit layout:
 *   DAS_steeringAngleRequest   : Motorola bit 6, len 15 (factor 0.1, offset -1638.35)
 *   DAS_steeringHapticRequest  : Motorola bit 7, len 1 (byte 0 bit 7)
 *   DAS_steeringControlCounter : Intel bit 16, len 4 (byte 2 bits[3:0])
 *   DAS_steeringControlType    : Motorola bit 23, len 2 (byte 2 bits[7:6])
 *   DAS_steeringControlChecksum: Intel bit 24, len 8 (byte 3)
 *
 * @param d Output buffer (4 bytes, zeroed and filled by this function).
 * @param angle_deg Requested steering angle in degrees (signed, ±360 max).
 * @param enabled True if steering control is active.
 * @param counter Rolling counter value [0..15].
 * @param hw4 True for HW4 (LKA mode), false for HW3/LEGACY (angle control mode).
 */
static void buildDasSteeringFrame(uint8_t *d, float angle_deg, bool enabled, uint8_t counter, bool hw4)
{
	memset(d, 0, 4);

	if (angle_deg > 360.0f)
		angle_deg = 360.0f;
	if (angle_deg < -360.0f)
		angle_deg = -360.0f;

	// openpilot negates the requested angle (sign convention difference)
	float req_angle = -angle_deg;
	float raw_f = (req_angle + 1638.35f) / 0.1f;
	if (raw_f < 0)
		raw_f = 0;
	if (raw_f > 32767)
		raw_f = 32767;
	uint16_t raw = (uint16_t)(raw_f + 0.5f);

	// byte[0]: haptic(b7)=0, angle[14:8] in bits[6:0] (Motorola MSB first)
	d[0] = (uint8_t)((raw >> 8) & 0x7F);
	// byte[1]: angle[7:0] (Motorola LSB portion)
	d[1] = (uint8_t)(raw & 0xFF);
	// byte[2]: steeringControlType[7:6] | counter[3:0]
	uint8_t ctrl_type = 0;
	if (enabled)
		ctrl_type = hw4 ? DAS_STEER_LKA : DAS_STEER_ANGLE_CTRL;
	d[2] = (uint8_t)((ctrl_type << 6) | (counter & 0x0F));
	// byte[3]: checksum
	d[3] = dasChecksum(CAN_ID_DAS_STEERING_CTRL, d, 4, 3);
}

/**
 * @brief Build an APS_eacMonitor frame (0x27D, 3 bytes, little-endian).
 *
 * Bit layout:
 *   bits  0-1:  APS_eacAllow (1 = allow EPAS to accept steer commands)
 *   bits  8-11: APS_eacMonitorCounter
 *   bits 16-23: APS_eacMonitorChecksum
 *
 * @param d Output buffer (3 bytes, zeroed and filled by this function).
 * @param counter Rolling counter value [0..15].
 */
static void buildApsEacFrame(uint8_t *d, uint8_t counter)
{
	memset(d, 0, 3);
	d[0] = 0x01;		   // APS_eacAllow = 1 (permit EPAS steering)
	d[1] = counter & 0x0F; // APS_eacMonitorCounter
	d[2] = dasChecksum(CAN_ID_APS_EAC_MONITOR, d, 3, 2);
}

/**
 * @brief Load DAS configuration from NVS (non-volatile storage).
 *
 * Reads enabled state, speed cap, and speed limit from the "tcm_das" namespace.
 * Values are clamped to compile-time min/max bounds.
 */
static void dasLoadNvs()
{
	dasPrefs.begin("tcm_das", true);
	dasDriveEnabled = dasPrefs.getBool("en", false);
	// Runtime safety cap — changeable from API/serial/dashboard for closed-track use.
	// Hard ceiling is DAS_SPEED_CAP_MAX_KPH.
	uint16_t cap = dasPrefs.getUShort("cap", (uint16_t)DAS_SPEED_CAP_DEFAULT);
	if (cap < (uint16_t)DAS_SPEED_CAP_MIN_KPH)
		cap = (uint16_t)DAS_SPEED_CAP_MIN_KPH;
	if (cap > (uint16_t)DAS_SPEED_CAP_MAX_KPH)
		cap = (uint16_t)DAS_SPEED_CAP_MAX_KPH;
	dasSpeedCapKph = (float)cap;
	uint8_t spd = dasPrefs.getUChar("spd", (uint8_t)DAS_SPEED_LIMIT_DEFAULT);
	if (spd > (uint8_t)dasSpeedCapKph)
		spd = (uint8_t)dasSpeedCapKph;
	dasSpeedLimitKph = (float)spd;
	dasPrefs.end();
}

/**
 * @brief Persist current DAS configuration to NVS.
 *
 * Writes enabled state, speed cap, and speed limit to the "tcm_das" namespace.
 */
static void dasSaveNvs()
{
	dasPrefs.begin("tcm_das", false);
	dasPrefs.putBool("en", dasDriveEnabled);
	dasPrefs.putUShort("cap", (uint16_t)dasSpeedCapKph);
	dasPrefs.putUChar("spd", (uint8_t)dasSpeedLimitKph);
	dasPrefs.end();
}

/**
 * @brief Initialize the DAS drive subsystem by loading persisted configuration.
 */
static void dasInit()
{
	dasLoadNvs();
}

/**
 * @brief Enable or disable DAS drive mode (persisted to NVS).
 *
 * Disabling immediately queues a 5-frame cancel burst and resets the applied angle.
 *
 * @param en True to enable drive mode, false to disable and disengage.
 */
static void dasDriveSetEnabled(bool en)
{
	dasDriveEnabled = en;
	if (!en)
	{
		dasActive = false;
		dasCancelCount = DAS_CANCEL_FRAMES;
		dasAppliedAngle = 0.0f;
	}
	dasSaveNvs();
}

/**
 * @brief Check if DAS drive mode is enabled.
 * @return True if drive mode is enabled.
 */
static bool dasDriveIsEnabled()
{
	return dasDriveEnabled;
}

/**
 * @brief Check if DAS drive has active live control input.
 * @return True if actively receiving gamepad commands.
 */
static bool dasDriveIsActive()
{
	return dasActive;
}

/**
 * @brief Update live control values from the gamepad. Call every ~20 ms.
 *
 * Resets the dead-man timer. If not called within DAS_DEADMAN_MS, auto-cancel triggers.
 *
 * @param steer_deg Requested steering angle in degrees (signed).
 * @param accel_min_ms2 Brake command in m/s² (≤ 0).
 * @param accel_max_ms2 Throttle command in m/s² (≥ 0).
 * @param speed_kph Desired set speed in km/h.
 * @param now Current timestamp in milliseconds (millis()).
 */
static void dasSetControl(float steer_deg, float accel_min_ms2, float accel_max_ms2, float speed_kph, unsigned long now)
{
	if (!dasDriveEnabled)
		return;
	dasSteerAngle = steer_deg;
	dasAccelMin = accel_min_ms2;
	dasAccelMax = accel_max_ms2;
	dasSetSpeedKph = speed_kph;
	dasLastUpdateMs = now;
	dasActive = true;
}

/**
 * @brief Compute the maximum allowed steering angle at a given speed.
 *
 * Below DAS_LOW_SPEED_KPH the full ±360° is permitted for parking maneuvers.
 * Above that threshold, the angle is capped to limit lateral acceleration to
 * DAS_LAT_ACCEL_MAX_MS2 using the bicycle-model approximation:
 * max_angle_rad = a_y_max * L / v².
 *
 * @param v_kph Current vehicle speed in km/h (absolute value).
 * @return Maximum allowed steering angle in degrees.
 */
static float dasMaxSteerAtSpeed(float v_kph)
{
	if (v_kph < DAS_LOW_SPEED_KPH)
		return DAS_MAX_ANGLE_DEG;
	float v_ms = v_kph / 3.6f;
	float lim_rad = (DAS_LAT_ACCEL_MAX_MS2 * DAS_WHEELBASE_M) / (v_ms * v_ms);
	float lim_deg = lim_rad * 57.29578f; // rad to deg conversion
	if (lim_deg > DAS_MAX_ANGLE_DEG)
		lim_deg = DAS_MAX_ANGLE_DEG;
	if (lim_deg < 1.0f)
		lim_deg = 1.0f;
	return lim_deg;
}

/**
 * @brief Apply per-frame angle rate limiting (openpilot MAX_ANGLE_RATE = 5°/20ms).
 * @param target Externally-requested steering angle in degrees.
 * @param last Previously commanded angle (dasAppliedAngle).
 * @return New commanded angle after rate limiting.
 */
static float dasRateLimitAngle(float target, float last)
{
	float delta = target - last;
	if (delta > DAS_MAX_ANGLE_RATE_DEG)
		delta = DAS_MAX_ANGLE_RATE_DEG;
	if (delta < -DAS_MAX_ANGLE_RATE_DEG)
		delta = -DAS_MAX_ANGLE_RATE_DEG;
	return last + delta;
}

/**
 * @brief Apply standstill brake-hold to prevent creep/roll at near-zero speed.
 *
 * If no meaningful longitudinal input is present and the vehicle is nearly stopped,
 * a small negative acceleration is injected. Mirrors openpilot's brake-hold strategy.
 *
 * @param v_kph Current vehicle speed in km/h.
 * @param accel_min Reference to minimum acceleration (modified in-place if hold applies).
 * @param accel_max Reference to maximum acceleration (read to detect "no input").
 */
static void dasApplyStandstillHold(float v_kph, float &accel_min, float &accel_max)
{
	bool noInput = (accel_max < 0.05f) && (accel_min > -0.05f);
	if (noInput && fabsf(v_kph) < DAS_STANDSTILL_KPH)
	{
		if (accel_min > DAS_STANDSTILL_HOLD_MS2)
			accel_min = DAS_STANDSTILL_HOLD_MS2;
	}
}

/**
 * @brief Rate-limited CAN frame sender — call every loop iteration.
 *
 * Manages timing for all three DAS frames (control, steering, EAC) and handles
 * dead-man timeout, cancel burst sequencing, and speed-aware angle clamping.
 *
 * @param now Current timestamp in milliseconds (millis()).
 * @param s Global state reference (provides vehicle speed, variant, txPaused flag).
 */
static void dasTick(unsigned long now, const State &s)
{
	if (s.txPaused)
		return;
	if (!dasDriveEnabled && dasCancelCount == 0)
		return;

	// Dead-man: auto-cancel if no control update received within timeout
	if (dasActive && (now - dasLastUpdateMs) > DAS_DEADMAN_MS)
	{
		dasActive = false;
		dasCancelCount = DAS_CANCEL_FRAMES;
	}

	const bool active = dasActive;
	const bool hw4 = (s.variant == HW4);

	// APS_eacMonitor — 10 Hz (EPAS steer-allow keepalive)
	if (now - dasEacLastMs >= DAS_EAC_INTERVAL_MS)
	{
		dasEacLastMs = now;
		if (dasDriveEnabled && (active || dasCancelCount > 0))
		{
			uint8_t d[3];
			buildApsEacFrame(d, dasEacCounter);
			Frame f;
			f.id = CAN_ID_APS_EAC_MONITOR;
			f.dlc = 3;
			memcpy(f.data, d, 3);
			driverSend(f, BUS_CHASSIS);
			dasEacCounter = (dasEacCounter + 1) & 0x0F;
		}
	}

	// DAS_steeringControl — 50 Hz (lateral angle command)
	if (now - dasSteerLastMs >= DAS_STEER_INTERVAL_MS)
	{
		dasSteerLastMs = now;
		if (dasDriveEnabled)
		{
			// Speed-aware angle clamp + 5°/frame rate limit (openpilot Tesla port)
			float vKph = fabsf(s.vehicleSpeed);
			float maxAng = dasMaxSteerAtSpeed(vKph);
			float target = dasSteerAngle;
			if (target > maxAng)
				target = maxAng;
			if (target < -maxAng)
				target = -maxAng;
			float commanded =
				active ? dasRateLimitAngle(target, dasAppliedAngle) : 0.0f; // Disengaged — relax to centre
			dasAppliedAngle = commanded;

			uint8_t d[4];
			buildDasSteeringFrame(d, commanded, active, dasCounter4, hw4);
			Frame f;
			f.id = CAN_ID_DAS_STEERING_CTRL;
			f.dlc = 4;
			memcpy(f.data, d, 4);
			driverSend(f, BUS_CHASSIS);
			dasCounter4 = (dasCounter4 + 1) & 0x0F;
		}
	}

	// DAS_control — 25 Hz (longitudinal ACC command)
	if (now - dasCtrlLastMs >= DAS_CTRL_INTERVAL_MS)
	{
		dasCtrlLastMs = now;
		if (dasDriveEnabled && (active || dasCancelCount > 0))
		{
			// Standstill brake-hold prevents creep when user releases both triggers
			float aMin = dasAccelMin, aMax = dasAccelMax;
			if (active)
				dasApplyStandstillHold(s.vehicleSpeed, aMin, aMax);

			uint8_t d[8];
			buildDasControlFrame(d, dasSetSpeedKph, aMin, aMax, dasCounter3, active);
			Frame f;
			f.id = CAN_ID_DAS_CONTROL;
			f.dlc = 8;
			memcpy(f.data, d, 8);
			driverSend(f, BUS_CHASSIS);
			dasCounter3 = (dasCounter3 + 1) & 0x07;
			if (!active && dasCancelCount > 0)
				dasCancelCount--;
		}
	}
}

/**
 * @brief Execute a DAS drive command string received from the client.
 *
 * Supported commands: "drive:on", "drive:off", "drive:speed:<kph>", "drive:cap:<kph>".
 *
 * @param cmd Null-terminated command string.
 * @param s Global state reference (unused but required by dispatch signature).
 * @return True if the command was recognized and executed, false otherwise.
 */
static bool executeDasCmd(const char *cmd, State &)
{
	if (strncmp(cmd, "drive:", 6) != 0)
		return false;
	const char *sub = cmd + 6;

	if (strcmp(sub, "on") == 0)
	{
		dasDriveSetEnabled(true);
		return true;
	}
	if (strcmp(sub, "off") == 0)
	{
		dasDriveSetEnabled(false);
		return true;
	}
	if (strncmp(sub, "speed:", 6) == 0)
	{
		int v = atoi(sub + 6);
		if (v < 1)
			v = 1;
		if (v > (int)dasSpeedCapKph)
			v = (int)dasSpeedCapKph;
		dasSpeedLimitKph = (float)v;
		dasSaveNvs();
		return true;
	}
	if (strncmp(sub, "cap:", 4) == 0)
	{
		// Runtime safety cap — bounded so users can't lock themselves out (min)
		// or overflow the DAS_control speed byte (max).
		int v = atoi(sub + 4);
		if (v < (int)DAS_SPEED_CAP_MIN_KPH)
			v = (int)DAS_SPEED_CAP_MIN_KPH;
		if (v > (int)DAS_SPEED_CAP_MAX_KPH)
			v = (int)DAS_SPEED_CAP_MAX_KPH;
		dasSpeedCapKph = (float)v;
		// Pull user-facing limit down if it now exceeds the new cap
		if (dasSpeedLimitKph > dasSpeedCapKph)
			dasSpeedLimitKph = dasSpeedCapKph;
		dasSaveNvs();
		return true;
	}
	return false;
}
