/**
 * @file firmware/test/test_native_ble_distance/test_ble_distance.cpp
 * @brief Unit tests for BLE key distance estimation
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <unity.h>
#include <cmath>

#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

#include "core/types.h"
#include "vehicle/ble/distance.h"

void setUp() {}
void tearDown() {}

void test_parse_ble_distance_mode()
{
	BleDistanceMode m;
	TEST_ASSERT_TRUE(parseBleDistanceMode("off", m));
	TEST_ASSERT_EQUAL(BLE_DISTANCE_OFF, m);
	TEST_ASSERT_TRUE(parseBleDistanceMode("threshold", m));
	TEST_ASSERT_EQUAL(BLE_DISTANCE_THRESHOLD, m);
	TEST_ASSERT_TRUE(parseBleDistanceMode("formula", m));
	TEST_ASSERT_EQUAL(BLE_DISTANCE_FORMULA, m);
	TEST_ASSERT_TRUE(parseBleDistanceMode("kalman", m));
	TEST_ASSERT_EQUAL(BLE_DISTANCE_KALMAN, m);
	TEST_ASSERT_FALSE(parseBleDistanceMode("unknown", m));
}

void test_mode_names_round_trip()
{
	TEST_ASSERT_EQUAL_STRING("off", bleDistanceModeName(BLE_DISTANCE_OFF));
	TEST_ASSERT_EQUAL_STRING("threshold", bleDistanceModeName(BLE_DISTANCE_THRESHOLD));
	TEST_ASSERT_EQUAL_STRING("formula", bleDistanceModeName(BLE_DISTANCE_FORMULA));
	TEST_ASSERT_EQUAL_STRING("kalman", bleDistanceModeName(BLE_DISTANCE_KALMAN));
}

void test_off_returns_negative_one()
{
	State s = {};
	s.bleDistanceMode = BLE_DISTANCE_OFF;
	BleDistanceEstimator est;
	TEST_ASSERT_EQUAL_FLOAT(-1.0f, est.update(-60, s));
	TEST_ASSERT_EQUAL(BLE_DISTANCE_OFF, s.bleDistanceMode);
}

void test_threshold_buckets()
{
	State s = {};
	s.bleDistanceMode = BLE_DISTANCE_THRESHOLD;
	BleDistanceEstimator est;

	TEST_ASSERT_EQUAL_FLOAT(1.0f, est.update(-50, s));
	TEST_ASSERT_EQUAL_FLOAT(5.0f, est.update(-65, s));
	TEST_ASSERT_EQUAL_FLOAT(15.0f, est.update(-80, s));
}

void test_formula_at_one_meter()
{
	State s = {};
	s.bleDistanceMode = BLE_DISTANCE_FORMULA;
	s.bleDistanceFactor = 2.0f;
	s.bleDistanceCalOffset = 0.0f;
	BleDistanceEstimator est;

	// At default TX power -59 dBm and RSSI -59 dBm, distance should be 1 m
	float d = est.update(-59, s);
	TEST_ASSERT_FLOAT_WITHIN(0.05f, 1.0f, d);
	TEST_ASSERT_FLOAT_WITHIN(0.05f, 1.0f, s.bleDistanceMeters);
}

void test_formula_clamps_minimum()
{
	State s = {};
	s.bleDistanceMode = BLE_DISTANCE_FORMULA;
	s.bleDistanceFactor = 2.0f;
	BleDistanceEstimator est;

	// Very strong signal should clamp to 0.1 m, not infinity/negative
	float d = est.update(-20, s);
	TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.1f, d);
}

void test_formula_clamps_maximum()
{
	State s = {};
	s.bleDistanceMode = BLE_DISTANCE_FORMULA;
	s.bleDistanceFactor = 2.0f;
	BleDistanceEstimator est;

	// Very weak signal should clamp to 100 m
	float d = est.update(-100, s);
	TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, d);
}

void test_formula_with_calibration_offset()
{
	State s = {};
	s.bleDistanceMode = BLE_DISTANCE_FORMULA;
	s.bleDistanceFactor = 2.0f;
	s.bleDistanceCalOffset = 10.0f;
	BleDistanceEstimator est;

	// TX power effectively -49 dBm, RSSI -59 dBm -> ratio = (-49 + 59)/20 = 0.5 -> ~3.16 m
	float d = est.update(-59, s);
	TEST_ASSERT_FLOAT_WITHIN(0.1f, 3.16f, d);
}

void test_formula_rejects_zero_exponent()
{
	State s = {};
	s.bleDistanceMode = BLE_DISTANCE_FORMULA;
	s.bleDistanceFactor = 0.0f;
	BleDistanceEstimator est;

	float d = est.update(-59, s);
	TEST_ASSERT_FLOAT_WITHIN(0.05f, 1.0f, d);
}

void test_kalman_smoothes_spike()
{
	State s = {};
	s.bleDistanceMode = BLE_DISTANCE_KALMAN;
	s.bleDistanceFactor = 2.0f;
	s.bleDistanceCalOffset = 0.0f;
	BleDistanceEstimator est;

	// Seed filter with -59 dBm
	est.update(-59, s);
	// Single outlier at -40 should be pulled back toward -59 (smoothed RSSI
	// stays closer to -59 than the raw outlier, so distance stays bounded).
	float d = est.update(-40, s);
	TEST_ASSERT_TRUE(d > 0.0f);
	TEST_ASSERT_TRUE(d < 10.0f);
	TEST_ASSERT_TRUE(s.bleDistanceMeters < 10.0f);
}

void test_kalman_resets_when_mode_switches_to_off()
{
	State s = {};
	s.bleDistanceMode = BLE_DISTANCE_KALMAN;
	s.bleDistanceFactor = 2.0f;
	BleDistanceEstimator est;

	est.update(-59, s);
	s.bleDistanceMode = BLE_DISTANCE_OFF;
	TEST_ASSERT_EQUAL_FLOAT(-1.0f, est.update(-50, s));
}

void test_rssi_stored_in_state()
{
	State s = {};
	s.bleDistanceMode = BLE_DISTANCE_FORMULA;
	BleDistanceEstimator est;
	est.update(-72, s);
	TEST_ASSERT_EQUAL(-72, s.bleRssi);
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_parse_ble_distance_mode);
	RUN_TEST(test_mode_names_round_trip);
	RUN_TEST(test_off_returns_negative_one);
	RUN_TEST(test_threshold_buckets);
	RUN_TEST(test_formula_at_one_meter);
	RUN_TEST(test_formula_clamps_minimum);
	RUN_TEST(test_formula_clamps_maximum);
	RUN_TEST(test_formula_with_calibration_offset);
	RUN_TEST(test_formula_rejects_zero_exponent);
	RUN_TEST(test_kalman_smoothes_spike);
	RUN_TEST(test_kalman_resets_when_mode_switches_to_off);
	RUN_TEST(test_rssi_stored_in_state);

	return UNITY_END();
}
