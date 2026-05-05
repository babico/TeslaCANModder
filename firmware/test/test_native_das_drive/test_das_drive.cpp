// ── DAS Drive Tests ──────────────────────────────────────────────────────────
// Tests the openpilot-style DAS frame builders, checksum, clamping, rate
// limit, speed-aware steer cap, standstill brake-hold, and NVS round-trip.
//
// Frame bit-layouts and constants mirror das_drive.h; if those constants
// change, expectations here must be re-derived (intentional — the wire
// format is safety-critical and must not silently drift).

#include <unity.h>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

class __FlashStringHelper;
#define F(s) (reinterpret_cast<const __FlashStringHelper *>(s))

// Native build flags (see [env:native])
#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

// Provide the fake Preferences API before das_drive.h includes <Preferences.h>
#include "../support/fake_preferences.h"
#define Preferences_h  // shadow the real ESP32 header

#include "core/types.h"
#include "vehicle/can/ids.h"

// ── Stubs ────────────────────────────────────────────────────────────────────
struct SendCall
{
	Frame f;
	uint8_t bus;
};
static SendCall stub_sends[64];
static uint8_t  stub_send_count = 0;

void driverSend(const Frame &f, uint8_t bus)
{
	if (stub_send_count < 64)
	{
		stub_sends[stub_send_count].f   = f;
		stub_sends[stub_send_count].bus = bus;
		stub_send_count++;
	}
}

void sendLog(const char *) {}
void sendLog(const __FlashStringHelper *) {}
void saveSettings(const State &) {}
void resetHandlerLogFlags() {}
void applyFilters(State &) {}

// das_drive.h pulls vehicle/can/ids.h which already provides CAN_ID_*; it
// also includes core/forward.h. Path is relative to lib/vehicle/can/
#include "feature/das_drive.h"

// ── helpers ─────────────────────────────────────────────────────────────────
static State makeState()
{
	State s = {};
	s.variant = HW4;
	return s;
}

void setUp()
{
	stub_send_count = 0;
	Preferences::clearAll();
	// Reset module-static state to defaults between tests.
	dasDriveEnabled  = false;
	dasActive        = false;
	dasSteerAngle    = 0.0f;
	dasAccelMin      = 0.0f;
	dasAccelMax      = 0.0f;
	dasSetSpeedKph   = DAS_SPEED_CAP_DEFAULT;
	dasSpeedCapKph   = DAS_SPEED_CAP_DEFAULT;
	dasSpeedLimitKph = DAS_SPEED_LIMIT_DEFAULT;
	dasCounter3      = 0;
	dasCounter4      = 0;
	dasEacCounter    = 0;
	dasCancelCount   = 0;
	dasAppliedAngle  = 0.0f;
}
void tearDown() {}

// ── Tesla CAN checksum ──────────────────────────────────────────────────────
void test_checksum_matches_tesla_formula()
{
	// teslacan.py: sum = (id&0xFF) + (id>>8&0xFF) + sum(data except checksum byte)
	uint8_t d[8] = {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x00};
	uint32_t id  = 0x2B9;
	uint8_t want = (uint8_t)((id & 0xFF) + ((id >> 8) & 0xFF) + 0x10 + 0x20 + 0x30 + 0x40 + 0x50 + 0x60 + 0x70);
	TEST_ASSERT_EQUAL_HEX8(want, dasChecksum(id, d, 8, /*csByte*/ 7));
}

// ── DAS_control (0x2B9) ─────────────────────────────────────────────────────
void test_das_control_active_packs_and_checksums()
{
	uint8_t d[8];
	buildDasControlFrame(d, /*speed*/ 25.0f, /*aMin*/ -1.0f, /*aMax*/ 1.0f,
	                     /*counter*/ 3, /*active*/ true);

	// setSpeedRaw = 250 (0xFA) — bits 0..11
	uint16_t setSpeedRaw = (uint16_t)d[0] | (uint16_t)((d[1] & 0x0F) << 8);
	TEST_ASSERT_EQUAL_UINT16(250, setSpeedRaw);

	// accState = ACC_ON (4) — bits 12..15 (high nibble of d[1])
	TEST_ASSERT_EQUAL_HEX8(DAS_ACC_ON, (d[1] >> 4) & 0x0F);

	// counter — bits 53..55 (high 3 bits of d[6])
	TEST_ASSERT_EQUAL_HEX8(3, (d[6] >> 5) & 0x07);

	// checksum is recomputable
	TEST_ASSERT_EQUAL_HEX8(d[7], dasChecksum(CAN_ID_DAS_CONTROL, d, 8, 7));
}

void test_das_control_clamps_speed_to_cap()
{
	dasSpeedCapKph = 25.0f;
	uint8_t d[8];
	buildDasControlFrame(d, /*speed*/ 999.0f, 0.0f, 0.0f, 0, true);
	uint16_t setSpeedRaw = (uint16_t)d[0] | (uint16_t)((d[1] & 0x0F) << 8);
	// 25 / 0.1 = 250
	TEST_ASSERT_EQUAL_UINT16(250, setSpeedRaw);
}

void test_das_control_clamps_accel_to_safety_envelope()
{
	uint8_t d[8];
	// Try to exceed both envelopes. After clamping, raw values must reflect
	// DAS_ACCEL_MIN_MS2 and DAS_ACCEL_MAX_MS2 (not the requested values).
	buildDasControlFrame(d, 0.0f, /*aMin*/ -99.0f, /*aMax*/ 99.0f, 0, true);

	// accelMinRaw — bits 35..43: bits 35..39 in d[4][7:3], bits 40..43 in d[5][3:0]
	uint16_t accelMinRaw = (uint16_t)((d[4] >> 3) & 0x1F) | (uint16_t)((d[5] & 0x0F) << 5);
	uint16_t expectedMin = (uint16_t)((DAS_ACCEL_MIN_MS2 + 15.0f) / 0.04f + 0.5f);
	TEST_ASSERT_EQUAL_UINT16(expectedMin, accelMinRaw);

	// accelMaxRaw — bits 44..52: bits 44..47 in d[5][7:4], bits 48..52 in d[6][4:0]
	uint16_t accelMaxRaw = (uint16_t)((d[5] >> 4) & 0x0F) | (uint16_t)((d[6] & 0x1F) << 4);
	uint16_t expectedMax = (uint16_t)((DAS_ACCEL_MAX_MS2 + 15.0f) / 0.04f + 0.5f);
	TEST_ASSERT_EQUAL_UINT16(expectedMax, accelMaxRaw);
}

void test_das_control_inactive_uses_cancel_acc_state()
{
	uint8_t d[8];
	buildDasControlFrame(d, 0.0f, 0.0f, 0.0f, 0, /*active*/ false);
	TEST_ASSERT_EQUAL_HEX8(DAS_ACC_CANCEL, (d[1] >> 4) & 0x0F);
}

// ── DAS_steeringControl (0x488) ─────────────────────────────────────────────
void test_das_steering_disabled_sets_type_none()
{
	uint8_t d[4];
	buildDasSteeringFrame(d, /*angle*/ 10.0f, /*enabled*/ false, /*counter*/ 0, /*hw4*/ true);
	TEST_ASSERT_EQUAL_HEX8(DAS_STEER_NONE, (d[2] >> 6) & 0x03);
}

void test_das_steering_hw3_uses_angle_control()
{
	uint8_t d[4];
	buildDasSteeringFrame(d, 10.0f, true, 5, /*hw4*/ false);
	TEST_ASSERT_EQUAL_HEX8(DAS_STEER_ANGLE_CTRL, (d[2] >> 6) & 0x03);
	TEST_ASSERT_EQUAL_HEX8(5, d[2] & 0x0F);
	TEST_ASSERT_EQUAL_HEX8(d[3], dasChecksum(CAN_ID_DAS_STEERING_CTRL, d, 4, 3));
}

void test_das_steering_hw4_uses_lka()
{
	uint8_t d[4];
	buildDasSteeringFrame(d, 0.0f, true, 0, /*hw4*/ true);
	TEST_ASSERT_EQUAL_HEX8(DAS_STEER_LKA, (d[2] >> 6) & 0x03);
}

void test_das_steering_clamps_360_degrees()
{
	uint8_t d[4];
	buildDasSteeringFrame(d, /*angle*/ 9999.0f, true, 0, true);
	// Angle is sign-flipped then encoded as raw = (req+1638.35)/0.1, clamped
	// to [0, 32767]. For req=-360 → raw = (1638.35-360)/0.1 = 12783.5 → 12784
	uint16_t raw = ((uint16_t)(d[0] & 0x7F) << 8) | d[1];
	uint16_t expected = (uint16_t)(((-(-360.0f)) /* sign flip */ * 0 + 0) + 0); // placeholder
	(void)expected;
	// Recompute exactly the same way as the builder for fair comparison.
	float req = -(360.0f);
	uint16_t want = (uint16_t)((req + 1638.35f) / 0.1f + 0.5f);
	TEST_ASSERT_EQUAL_UINT16(want, raw);
}

// ── APS_eacMonitor (0x27D) ──────────────────────────────────────────────────
void test_aps_eac_allow_is_one_and_checksums()
{
	uint8_t d[3];
	buildApsEacFrame(d, /*counter*/ 9);
	TEST_ASSERT_EQUAL_HEX8(0x01, d[0]);
	TEST_ASSERT_EQUAL_HEX8(9, d[1] & 0x0F);
	TEST_ASSERT_EQUAL_HEX8(d[2], dasChecksum(CAN_ID_APS_EAC_MONITOR, d, 3, 2));
}

// ── Speed-aware steer cap ───────────────────────────────────────────────────
void test_max_steer_at_low_speed_is_full_lock()
{
	TEST_ASSERT_EQUAL_FLOAT(DAS_MAX_ANGLE_DEG, dasMaxSteerAtSpeed(0.0f));
	TEST_ASSERT_EQUAL_FLOAT(DAS_MAX_ANGLE_DEG, dasMaxSteerAtSpeed(DAS_LOW_SPEED_KPH - 0.1f));
}

void test_max_steer_at_high_speed_clamped_by_lateral_accel()
{
	// at v = 60 kph (16.67 m/s), lim_rad = 3*2.875 / (16.67^2) ≈ 0.031 rad ≈ 1.78°
	float got = dasMaxSteerAtSpeed(60.0f);
	TEST_ASSERT_TRUE(got > 1.0f);    // floor is 1°
	TEST_ASSERT_TRUE(got < 5.0f);    // bicycle model gives small numbers fast
}

// ── Per-frame angle rate limit ──────────────────────────────────────────────
void test_rate_limit_clamps_positive_jump()
{
	// last=0, target=+50 → step capped at +5°
	float v = dasRateLimitAngle(50.0f, 0.0f);
	TEST_ASSERT_EQUAL_FLOAT(DAS_MAX_ANGLE_RATE_DEG, v);
}

void test_rate_limit_clamps_negative_jump()
{
	float v = dasRateLimitAngle(-50.0f, 0.0f);
	TEST_ASSERT_EQUAL_FLOAT(-DAS_MAX_ANGLE_RATE_DEG, v);
}

void test_rate_limit_passes_small_delta()
{
	float v = dasRateLimitAngle(2.0f, 0.0f);
	TEST_ASSERT_EQUAL_FLOAT(2.0f, v);
}

// ── Standstill brake-hold ───────────────────────────────────────────────────
void test_standstill_hold_applied_when_no_input_near_zero_speed()
{
	float aMin = 0.0f, aMax = 0.0f;
	dasApplyStandstillHold(/*v_kph*/ 0.5f, aMin, aMax);
	TEST_ASSERT_EQUAL_FLOAT(DAS_STANDSTILL_HOLD_MS2, aMin);
}

void test_standstill_hold_skipped_when_user_input_present()
{
	float aMin = 0.0f, aMax = 1.5f; // accel input
	dasApplyStandstillHold(0.5f, aMin, aMax);
	TEST_ASSERT_EQUAL_FLOAT(0.0f, aMin); // unchanged
}

void test_standstill_hold_skipped_above_threshold_speed()
{
	float aMin = 0.0f, aMax = 0.0f;
	dasApplyStandstillHold(/*v_kph*/ 5.0f, aMin, aMax);
	TEST_ASSERT_EQUAL_FLOAT(0.0f, aMin); // unchanged
}

void test_standstill_hold_does_not_relax_stronger_brake()
{
	float aMin = -2.0f, aMax = 0.0f; // already braking harder
	dasApplyStandstillHold(0.0f, aMin, aMax);
	TEST_ASSERT_EQUAL_FLOAT(-2.0f, aMin); // not raised toward -0.4
}

// ── NVS round-trip ──────────────────────────────────────────────────────────
void test_nvs_roundtrip_preserves_enable_cap_and_speed_limit()
{
	dasDriveEnabled  = true;
	dasSpeedCapKph   = 80.0f;
	dasSpeedLimitKph = 45.0f;
	dasSaveNvs();

	dasDriveEnabled  = false;
	dasSpeedCapKph   = 0.0f;
	dasSpeedLimitKph = 0.0f;
	dasLoadNvs();

	TEST_ASSERT_TRUE(dasDriveEnabled);
	TEST_ASSERT_EQUAL_FLOAT(80.0f, dasSpeedCapKph);
	TEST_ASSERT_EQUAL_FLOAT(45.0f, dasSpeedLimitKph);
}

void test_nvs_load_clamps_cap_to_safety_window()
{
	// Write an out-of-range cap directly to backing store.
	Preferences p;
	p.begin("tcm_das", false);
	p.putUShort("cap", 9999);
	p.end();

	dasLoadNvs();
	TEST_ASSERT_EQUAL_FLOAT(DAS_SPEED_CAP_MAX_KPH, dasSpeedCapKph);
}

// ── dasDriveSetEnabled side-effects ─────────────────────────────────────────
void test_disable_queues_cancel_burst_and_zeros_angle()
{
	dasDriveEnabled = true;
	dasActive       = true;
	dasAppliedAngle = 12.5f;
	dasCancelCount  = 0;

	dasDriveSetEnabled(false);
	TEST_ASSERT_FALSE(dasDriveEnabled);
	TEST_ASSERT_FALSE(dasActive);
	TEST_ASSERT_EQUAL_UINT8(DAS_CANCEL_FRAMES, dasCancelCount);
	TEST_ASSERT_EQUAL_FLOAT(0.0f, dasAppliedAngle);
}

// ── main ────────────────────────────────────────────────────────────────────
int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_checksum_matches_tesla_formula);

	RUN_TEST(test_das_control_active_packs_and_checksums);
	RUN_TEST(test_das_control_clamps_speed_to_cap);
	RUN_TEST(test_das_control_clamps_accel_to_safety_envelope);
	RUN_TEST(test_das_control_inactive_uses_cancel_acc_state);

	RUN_TEST(test_das_steering_disabled_sets_type_none);
	RUN_TEST(test_das_steering_hw3_uses_angle_control);
	RUN_TEST(test_das_steering_hw4_uses_lka);
	RUN_TEST(test_das_steering_clamps_360_degrees);

	RUN_TEST(test_aps_eac_allow_is_one_and_checksums);

	RUN_TEST(test_max_steer_at_low_speed_is_full_lock);
	RUN_TEST(test_max_steer_at_high_speed_clamped_by_lateral_accel);

	RUN_TEST(test_rate_limit_clamps_positive_jump);
	RUN_TEST(test_rate_limit_clamps_negative_jump);
	RUN_TEST(test_rate_limit_passes_small_delta);

	RUN_TEST(test_standstill_hold_applied_when_no_input_near_zero_speed);
	RUN_TEST(test_standstill_hold_skipped_when_user_input_present);
	RUN_TEST(test_standstill_hold_skipped_above_threshold_speed);
	RUN_TEST(test_standstill_hold_does_not_relax_stronger_brake);

	RUN_TEST(test_nvs_roundtrip_preserves_enable_cap_and_speed_limit);
	RUN_TEST(test_nvs_load_clamps_cap_to_safety_window);

	RUN_TEST(test_disable_queues_cancel_burst_and_zeros_angle);
	return UNITY_END();
}
