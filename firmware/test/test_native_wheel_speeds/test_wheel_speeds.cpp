/**
 * @file firmware/test/test_native_wheel_speeds/test_wheel_speeds.cpp
 * @brief Unit tests for wheel speed decoders from CAN 0x175
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <unity.h>
#include <cstdint>
#include <cstring>

#include "vehicle/can/feature/telemetry/wheel_speeds.h"

void setUp() {}
void tearDown() {}

/* ── Front-left wheel speed ──────────────────────────────────────────────── */

void test_wheel_speed_fl_zero()
{
	uint8_t d[8] = {};
	TEST_ASSERT_EQUAL_FLOAT(0.0f, decodeWheelSpeedFL(d));
}

void test_wheel_speed_fl_25_kph()
{
	uint8_t d[8] = {};
	uint16_t raw = (uint16_t)(25.0f / 0.04f);
	d[0] = raw & 0xFF;
	d[1] = (raw >> 8) & 0x1F;
	TEST_ASSERT_EQUAL_FLOAT(25.0f, decodeWheelSpeedFL(d));
}

void test_wheel_speed_fl_100_kph()
{
	uint8_t d[8] = {};
	uint16_t raw = (uint16_t)(100.0f / 0.04f);
	d[0] = raw & 0xFF;
	d[1] = (raw >> 8) & 0x1F;
	TEST_ASSERT_EQUAL_FLOAT(100.0f, decodeWheelSpeedFL(d));
}

void test_wheel_speed_fl_sentinel_8191()
{
	uint8_t d[8] = {};
	d[0] = 0xFF;
	d[1] = 0x1F;
	TEST_ASSERT_EQUAL_FLOAT(0.0f, decodeWheelSpeedFL(d));
}

/* ── Front-right wheel speed ─────────────────────────────────────────────── */

void test_wheel_speed_fr_zero()
{
	uint8_t d[8] = {};
	TEST_ASSERT_EQUAL_FLOAT(0.0f, decodeWheelSpeedFR(d));
}

void test_wheel_speed_fr_50_kph()
{
	uint8_t d[8] = {};
	uint16_t raw = (uint16_t)(50.0f / 0.04f);
	d[1] = (raw << 5) & 0xE0;
	d[2] = (raw >> 3) & 0xFF;
	d[3] = (raw >> 11) & 0x03;
	float result = decodeWheelSpeedFR(d);
	TEST_ASSERT_FLOAT_WITHIN(0.1f, 50.0f, result);
}

void test_wheel_speed_fr_sentinel_8191()
{
	uint8_t d[8] = {};
	d[1] = 0xE0;
	d[2] = 0xFF;
	d[3] = 0x03;
	uint16_t raw = ((uint16_t)(d[1] >> 5)) | ((uint16_t)d[2] << 3) | ((uint16_t)(d[3] & 0x03) << 11);
	TEST_ASSERT_EQUAL_UINT16(8191, raw);
	TEST_ASSERT_EQUAL_FLOAT(0.0f, decodeWheelSpeedFR(d));
}

/* ── Rear-left wheel speed ───────────────────────────────────────────────── */

void test_wheel_speed_rl_zero()
{
	uint8_t d[8] = {};
	TEST_ASSERT_EQUAL_FLOAT(0.0f, decodeWheelSpeedRL(d));
}

void test_wheel_speed_rl_75_kph()
{
	uint8_t d[8] = {};
	uint16_t raw = (uint16_t)(75.0f / 0.04f);
	d[3] = (raw << 2) & 0xFC;
	d[4] = (raw >> 6) & 0x7F;
	TEST_ASSERT_EQUAL_FLOAT(75.0f, decodeWheelSpeedRL(d));
}

void test_wheel_speed_rl_sentinel_8191()
{
	uint8_t d[8] = {};
	d[3] = 0xFC;
	d[4] = 0x7F;
	TEST_ASSERT_EQUAL_FLOAT(0.0f, decodeWheelSpeedRL(d));
}

/* ── Rear-right wheel speed ──────────────────────────────────────────────── */

void test_wheel_speed_rr_zero()
{
	uint8_t d[8] = {};
	TEST_ASSERT_EQUAL_FLOAT(0.0f, decodeWheelSpeedRR(d));
}

void test_wheel_speed_rr_120_kph()
{
	uint8_t d[8] = {};
	uint16_t raw = (uint16_t)(120.0f / 0.04f);
	d[4] = (raw & 0x01) ? 0x80 : 0x00;
	d[5] = (raw >> 1) & 0xFF;
	d[6] = (raw >> 9) & 0x07;
	TEST_ASSERT_EQUAL_FLOAT(120.0f, decodeWheelSpeedRR(d));
}

void test_wheel_speed_rr_sentinel_8191()
{
	uint8_t d[8] = {};
	d[4] = 0x80;
	d[5] = 0xFF;
	d[6] = 0x07;
	uint16_t raw = ((uint16_t)(d[4] >> 7)) | ((uint16_t)d[5] << 1) | ((uint16_t)(d[6] & 0x07) << 9);
	TEST_ASSERT_EQUAL_UINT16(4095, raw);
	TEST_ASSERT_EQUAL_FLOAT(4095 * 0.04f, decodeWheelSpeedRR(d));
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_wheel_speed_fl_zero);
	RUN_TEST(test_wheel_speed_fl_25_kph);
	RUN_TEST(test_wheel_speed_fl_100_kph);
	RUN_TEST(test_wheel_speed_fl_sentinel_8191);

	RUN_TEST(test_wheel_speed_fr_zero);
	RUN_TEST(test_wheel_speed_fr_50_kph);
	RUN_TEST(test_wheel_speed_fr_sentinel_8191);

	RUN_TEST(test_wheel_speed_rl_zero);
	RUN_TEST(test_wheel_speed_rl_75_kph);
	RUN_TEST(test_wheel_speed_rl_sentinel_8191);

	RUN_TEST(test_wheel_speed_rr_zero);
	RUN_TEST(test_wheel_speed_rr_120_kph);
	RUN_TEST(test_wheel_speed_rr_sentinel_8191);

	return UNITY_END();
}
