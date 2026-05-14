/**
 * @file firmware/test/test_native_das_drive/test_das_drive.cpp
 * @brief Unit tests for DAS drive frame builders, checksum, clamping, and NVS persistence
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <unity.h>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

class __FlashStringHelper;
#define F(s) (reinterpret_cast<const __FlashStringHelper *>(s))

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

#include "../support/fake_preferences.h"
#define Preferences_h

#include "core/types.h"
#include "vehicle/can/ids.h"

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

#include "feature/das/das_drive.h"

/** @brief Creates a default HW4 State for DAS drive tests */
static State makeState()
{
	State s = {};
	s.variant = HW4;
	return s;
}

/** @brief Resets all DAS globals, stub send log, and NVS fake storage before each test */
void setUp()
{
	stub_send_count = 0;
	Preferences::clearAll();
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

/** @brief Test fixture teardown — no cleanup required */
void tearDown() {}

/** @brief Verifies dasChecksum matches the Tesla additive checksum formula */
void test_checksum_matches_tesla_formula()
{
	uint8_t d[8] = {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x00};
	uint32_t id  = 0x2B9;
	uint8_t want = (uint8_t)((id & 0xFF) + ((id >> 8) & 0xFF) + 0x10 + 0x20 + 0x30 + 0x40 + 0x50 + 0x60 + 0x70);
	TEST_ASSERT_EQUAL_HEX8(want, dasChecksum(id, d, 8, 7));
}

/** @brief Verifies DAS control frame packs set speed, ACC state, counter, and checksum */
void test_das_control_active_packs_and_checksums()
{
	uint8_t d[8];
	buildDasControlFrame(d, 25.0f, -1.0f, 1.0f, 3, true);

	// set speed: 25 kph * 10 = 250 in 12-bit field across bytes 0-1
	uint16_t setSpeedRaw = (uint16_t)d[0] | (uint16_t)((d[1] & 0x0F) << 8);
	TEST_ASSERT_EQUAL_UINT16(250, setSpeedRaw);

	// ACC state nibble in upper half of byte 1
	TEST_ASSERT_EQUAL_HEX8(DAS_ACC_ON, (d[1] >> 4) & 0x0F);

	// counter in bits [7:5] of byte 6
	TEST_ASSERT_EQUAL_HEX8(3, (d[6] >> 5) & 0x07);

	TEST_ASSERT_EQUAL_HEX8(d[7], dasChecksum(CAN_ID_DAS_CONTROL, d, 8, 7));
}

/** @brief Verifies set speed is clamped to the configured speed cap */
void test_das_control_clamps_speed_to_cap()
{
	dasSpeedCapKph = 25.0f;
	uint8_t d[8];
	buildDasControlFrame(d, 999.0f, 0.0f, 0.0f, 0, true);
	uint16_t setSpeedRaw = (uint16_t)d[0] | (uint16_t)((d[1] & 0x0F) << 8);
	TEST_ASSERT_EQUAL_UINT16(250, setSpeedRaw);
}

/** @brief Verifies accel min/max are clamped to the safety envelope constants */
void test_das_control_clamps_accel_to_safety_envelope()
{
	uint8_t d[8];
	buildDasControlFrame(d, 0.0f, -99.0f, 99.0f, 0, true);

	uint16_t accelMinRaw = (uint16_t)((d[4] >> 3) & 0x1F) | (uint16_t)((d[5] & 0x0F) << 5);
	uint16_t expectedMin = (uint16_t)((DAS_ACCEL_MIN_MS2 + 15.0f) / 0.04f + 0.5f);
	TEST_ASSERT_EQUAL_UINT16(expectedMin, accelMinRaw);

	uint16_t accelMaxRaw = (uint16_t)((d[5] >> 4) & 0x0F) | (uint16_t)((d[6] & 0x1F) << 4);
	uint16_t expectedMax = (uint16_t)((DAS_ACCEL_MAX_MS2 + 15.0f) / 0.04f + 0.5f);
	TEST_ASSERT_EQUAL_UINT16(expectedMax, accelMaxRaw);
}

/** @brief Verifies inactive control frame uses DAS_ACC_CANCEL state */
void test_das_control_inactive_uses_cancel_acc_state()
{
	uint8_t d[8];
	buildDasControlFrame(d, 0.0f, 0.0f, 0.0f, 0, false);
	TEST_ASSERT_EQUAL_HEX8(DAS_ACC_CANCEL, (d[1] >> 4) & 0x0F);
}

/** @brief Verifies disabled steering sets steer type to NONE */
void test_das_steering_disabled_sets_type_none()
{
	uint8_t d[4];
	buildDasSteeringFrame(d, 10.0f, false, 0, true);
	TEST_ASSERT_EQUAL_HEX8(DAS_STEER_NONE, (d[2] >> 6) & 0x03);
}

/** @brief Verifies HW3 steering uses angle control type and packs counter */
void test_das_steering_hw3_uses_angle_control()
{
	uint8_t d[4];
	buildDasSteeringFrame(d, 10.0f, true, 5, false);
	TEST_ASSERT_EQUAL_HEX8(DAS_STEER_ANGLE_CTRL, (d[2] >> 6) & 0x03);
	TEST_ASSERT_EQUAL_HEX8(5, d[2] & 0x0F);
	TEST_ASSERT_EQUAL_HEX8(d[3], dasChecksum(CAN_ID_DAS_STEERING_CTRL, d, 4, 3));
}

/** @brief Verifies HW4 steering uses LKA steer type */
void test_das_steering_hw4_uses_lka()
{
	uint8_t d[4];
	buildDasSteeringFrame(d, 0.0f, true, 0, true);
	TEST_ASSERT_EQUAL_HEX8(DAS_STEER_LKA, (d[2] >> 6) & 0x03);
}

/** @brief Verifies steering angle is clamped to ±360 degrees */
void test_das_steering_clamps_360_degrees()
{
	uint8_t d[4];
	buildDasSteeringFrame(d, 9999.0f, true, 0, true);
	uint16_t raw = ((uint16_t)(d[0] & 0x7F) << 8) | d[1];
	float req = -(360.0f);
	uint16_t want = (uint16_t)((req + 1638.35f) / 0.1f + 0.5f);
	TEST_ASSERT_EQUAL_UINT16(want, raw);
}

/** @brief Verifies APS EAC frame sets allow=1, packs counter, and checksums */
void test_aps_eac_allow_is_one_and_checksums()
{
	uint8_t d[3];
	buildApsEacFrame(d, 9);
	TEST_ASSERT_EQUAL_HEX8(0x01, d[0]);
	TEST_ASSERT_EQUAL_HEX8(9, d[1] & 0x0F);
	TEST_ASSERT_EQUAL_HEX8(d[2], dasChecksum(CAN_ID_APS_EAC_MONITOR, d, 3, 2));
}

/** @brief Verifies max steer at low speed returns full lock angle */
void test_max_steer_at_low_speed_is_full_lock()
{
	TEST_ASSERT_EQUAL_FLOAT(DAS_MAX_ANGLE_DEG, dasMaxSteerAtSpeed(0.0f));
	TEST_ASSERT_EQUAL_FLOAT(DAS_MAX_ANGLE_DEG, dasMaxSteerAtSpeed(DAS_LOW_SPEED_KPH - 0.1f));
}

/** @brief Verifies max steer at highway speed is reduced by lateral acceleration limit */
void test_max_steer_at_high_speed_clamped_by_lateral_accel()
{
	float got = dasMaxSteerAtSpeed(60.0f);
	TEST_ASSERT_TRUE(got > 1.0f);
	TEST_ASSERT_TRUE(got < 5.0f);
}

/** @brief Verifies rate limiter clamps a large positive angle jump */
void test_rate_limit_clamps_positive_jump()
{
	float v = dasRateLimitAngle(50.0f, 0.0f);
	TEST_ASSERT_EQUAL_FLOAT(DAS_MAX_ANGLE_RATE_DEG, v);
}

/** @brief Verifies rate limiter clamps a large negative angle jump */
void test_rate_limit_clamps_negative_jump()
{
	float v = dasRateLimitAngle(-50.0f, 0.0f);
	TEST_ASSERT_EQUAL_FLOAT(-DAS_MAX_ANGLE_RATE_DEG, v);
}

/** @brief Verifies rate limiter passes through small deltas unchanged */
void test_rate_limit_passes_small_delta()
{
	float v = dasRateLimitAngle(2.0f, 0.0f);
	TEST_ASSERT_EQUAL_FLOAT(2.0f, v);
}

/** @brief Verifies standstill hold is applied when speed is near zero and no user input */
void test_standstill_hold_applied_when_no_input_near_zero_speed()
{
	float aMin = 0.0f, aMax = 0.0f;
	dasApplyStandstillHold(0.5f, aMin, aMax);
	TEST_ASSERT_EQUAL_FLOAT(DAS_STANDSTILL_HOLD_MS2, aMin);
}

/** @brief Verifies standstill hold is skipped when user accel input is present */
void test_standstill_hold_skipped_when_user_input_present()
{
	float aMin = 0.0f, aMax = 1.5f;
	dasApplyStandstillHold(0.5f, aMin, aMax);
	TEST_ASSERT_EQUAL_FLOAT(0.0f, aMin);
}

/** @brief Verifies standstill hold is skipped above the speed threshold */
void test_standstill_hold_skipped_above_threshold_speed()
{
	float aMin = 0.0f, aMax = 0.0f;
	dasApplyStandstillHold(5.0f, aMin, aMax);
	TEST_ASSERT_EQUAL_FLOAT(0.0f, aMin);
}

/** @brief Verifies standstill hold does not relax a stronger existing brake request */
void test_standstill_hold_does_not_relax_stronger_brake()
{
	float aMin = -2.0f, aMax = 0.0f;
	dasApplyStandstillHold(0.0f, aMin, aMax);
	TEST_ASSERT_EQUAL_FLOAT(-2.0f, aMin);
}

/** @brief Verifies NVS save/load roundtrip preserves enable, cap, and speed limit */
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

/** @brief Verifies NVS load clamps an out-of-range cap to the safety maximum */
void test_nvs_load_clamps_cap_to_safety_window()
{
	Preferences p;
	p.begin("tcm_das", false);
	p.putUShort("cap", 9999);
	p.end();

	dasLoadNvs();
	TEST_ASSERT_EQUAL_FLOAT(DAS_SPEED_CAP_MAX_KPH, dasSpeedCapKph);
}

/** @brief Verifies disable queues cancel burst frames and zeros the applied angle */
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
