/**
 * @file firmware/test/test_native_motor_temps/test_motor_temps.cpp
 * @brief Unit tests for motor/inverter temperature decoders
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <unity.h>
#include <cstdint>
#include <cstring>

#include "vehicle/can/feature/telemetry/motor_temps.h"

void setUp() {}
void tearDown() {}

/* ── Rear inverter temps (0x315) ─────────────────────────────────────────── */

void test_rear_inv_temp_zero_raw()
{
	uint8_t d[8] = {};
	d[1] = 0;
	TEST_ASSERT_EQUAL_INT8(-40, decodeRearInvTemp(d));
}

void test_rear_inv_temp_40_raw()
{
	uint8_t d[8] = {};
	d[1] = 40;
	TEST_ASSERT_EQUAL_INT8(0, decodeRearInvTemp(d));
}

void test_rear_inv_temp_215_raw()
{
	uint8_t d[8] = {};
	d[1] = 215;
	TEST_ASSERT_EQUAL_INT8(175, decodeRearInvTemp(d));
}

void test_rear_inv_temp_100_raw()
{
	uint8_t d[8] = {};
	d[1] = 100;
	TEST_ASSERT_EQUAL_INT8(60, decodeRearInvTemp(d));
}

/* ── Rear stator temps (0x315) ───────────────────────────────────────────── */

void test_rear_stator_temp_zero_raw()
{
	uint8_t d[8] = {};
	d[2] = 0;
	TEST_ASSERT_EQUAL_INT8(-40, decodeRearStatorTemp(d));
}

void test_rear_stator_temp_85_raw()
{
	uint8_t d[8] = {};
	d[2] = 85;
	TEST_ASSERT_EQUAL_INT8(45, decodeRearStatorTemp(d));
}

/* ── Rear heatsink temps (0x315) ─────────────────────────────────────────── */

void test_rear_heatsink_temp_zero_raw()
{
	uint8_t d[8] = {};
	d[4] = 0;
	TEST_ASSERT_EQUAL_INT8(-40, decodeRearHeatsinkTemp(d));
}

void test_rear_heatsink_temp_120_raw()
{
	uint8_t d[8] = {};
	d[4] = 120;
	TEST_ASSERT_EQUAL_INT8(80, decodeRearHeatsinkTemp(d));
}

/* ── Front inverter temps (0x376) ────────────────────────────────────────── */

void test_front_inv_temp_zero_raw()
{
	uint8_t d[8] = {};
	d[1] = 0;
	TEST_ASSERT_EQUAL_INT8(-40, decodeFrontInvTemp(d));
}

void test_front_inv_temp_75_raw()
{
	uint8_t d[8] = {};
	d[1] = 75;
	TEST_ASSERT_EQUAL_INT8(35, decodeFrontInvTemp(d));
}

/* ── Front stator temps (0x376) ──────────────────────────────────────────── */

void test_front_stator_temp_zero_raw()
{
	uint8_t d[8] = {};
	d[2] = 0;
	TEST_ASSERT_EQUAL_INT8(-40, decodeFrontStatorTemp(d));
}

void test_front_stator_temp_90_raw()
{
	uint8_t d[8] = {};
	d[2] = 90;
	TEST_ASSERT_EQUAL_INT8(50, decodeFrontStatorTemp(d));
}

/* ── Front heatsink temps (0x376) ────────────────────────────────────────── */

void test_front_heatsink_temp_zero_raw()
{
	uint8_t d[8] = {};
	d[4] = 0;
	TEST_ASSERT_EQUAL_INT8(-40, decodeFrontHeatsinkTemp(d));
}

void test_front_heatsink_temp_200_raw()
{
	uint8_t d[8] = {};
	d[4] = 200;
	TEST_ASSERT_EQUAL_INT8(160, decodeFrontHeatsinkTemp(d));
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_rear_inv_temp_zero_raw);
	RUN_TEST(test_rear_inv_temp_40_raw);
	RUN_TEST(test_rear_inv_temp_215_raw);
	RUN_TEST(test_rear_inv_temp_100_raw);

	RUN_TEST(test_rear_stator_temp_zero_raw);
	RUN_TEST(test_rear_stator_temp_85_raw);

	RUN_TEST(test_rear_heatsink_temp_zero_raw);
	RUN_TEST(test_rear_heatsink_temp_120_raw);

	RUN_TEST(test_front_inv_temp_zero_raw);
	RUN_TEST(test_front_inv_temp_75_raw);

	RUN_TEST(test_front_stator_temp_zero_raw);
	RUN_TEST(test_front_stator_temp_90_raw);

	RUN_TEST(test_front_heatsink_temp_zero_raw);
	RUN_TEST(test_front_heatsink_temp_200_raw);

	return UNITY_END();
}
