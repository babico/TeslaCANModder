/**
 * @file firmware/test/test_native_ban_detect/test_ban_detect.cpp
 * @brief Unit tests for ban detection logic
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <unity.h>
#include <cstring>

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

unsigned long millis();

#include "core/types.h"
#include "vehicle/can/feature/safety/ban_detect.h"

static unsigned long fake_millis = 0;
unsigned long millis()
{
	return fake_millis;
}

void setUp() {}
void tearDown() {}

/* ── apTierName ──────────────────────────────────────────────────────────── */

void test_ap_tier_name_none()
{
	TEST_ASSERT_EQUAL_STRING("NONE", apTierName(0));
}

void test_ap_tier_name_highway()
{
	TEST_ASSERT_EQUAL_STRING("HIGHWAY", apTierName(1));
}

void test_ap_tier_name_enhanced()
{
	TEST_ASSERT_EQUAL_STRING("ENHANCED", apTierName(2));
}

void test_ap_tier_name_self_driving()
{
	TEST_ASSERT_EQUAL_STRING("SELF_DRIVING", apTierName(3));
}

void test_ap_tier_name_basic()
{
	TEST_ASSERT_EQUAL_STRING("BASIC", apTierName(4));
}

void test_ap_tier_name_unknown()
{
	TEST_ASSERT_EQUAL_STRING("UNKNOWN", apTierName(-1));
}

void test_ap_tier_name_unknown_high_value()
{
	TEST_ASSERT_EQUAL_STRING("UNKNOWN", apTierName(10));
}

/* ── checkBanDetection ───────────────────────────────────────────────────── */

void test_ban_detect_self_driving_to_basic()
{
	State s = {};
	TEST_ASSERT_TRUE(checkBanDetection(3, 4, s));
	TEST_ASSERT_EQUAL(1, s.banDetectionCount);
	TEST_ASSERT_EQUAL(5, s.banThreatLevel);
}

void test_ban_detect_self_driving_to_none()
{
	State s = {};
	TEST_ASSERT_TRUE(checkBanDetection(3, 0, s));
	TEST_ASSERT_EQUAL(5, s.banThreatLevel);
}

void test_ban_detect_enhanced_to_highway()
{
	State s = {};
	TEST_ASSERT_TRUE(checkBanDetection(2, 1, s));
	TEST_ASSERT_EQUAL(3, s.banThreatLevel);
}

void test_ban_detect_self_driving_to_enhanced()
{
	State s = {};
	TEST_ASSERT_TRUE(checkBanDetection(3, 2, s));
	TEST_ASSERT_EQUAL(3, s.banThreatLevel);
}

void test_ban_detect_no_change()
{
	State s = {};
	TEST_ASSERT_FALSE(checkBanDetection(3, 3, s));
}

void test_ban_detect_negative_prev()
{
	State s = {};
	TEST_ASSERT_FALSE(checkBanDetection(-1, 0, s));
}

void test_ban_detect_negative_new()
{
	State s = {};
	TEST_ASSERT_FALSE(checkBanDetection(3, -1, s));
}

void test_ban_detect_both_negative()
{
	State s = {};
	TEST_ASSERT_FALSE(checkBanDetection(-1, -1, s));
}

void test_ban_detect_highway_to_none_not_flagged()
{
	State s = {};
	TEST_ASSERT_FALSE(checkBanDetection(1, 0, s));
}

void test_ban_detect_basic_to_self_driving_not_flagged()
{
	State s = {};
	TEST_ASSERT_FALSE(checkBanDetection(4, 3, s));
}

void test_ban_detect_counter_increments()
{
	State s = {};
	checkBanDetection(3, 4, s);
	checkBanDetection(3, 0, s);
	checkBanDetection(2, 1, s);
	TEST_ASSERT_EQUAL(3, s.banDetectionCount);
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_ap_tier_name_none);
	RUN_TEST(test_ap_tier_name_highway);
	RUN_TEST(test_ap_tier_name_enhanced);
	RUN_TEST(test_ap_tier_name_self_driving);
	RUN_TEST(test_ap_tier_name_basic);
	RUN_TEST(test_ap_tier_name_unknown);
	RUN_TEST(test_ap_tier_name_unknown_high_value);

	RUN_TEST(test_ban_detect_self_driving_to_basic);
	RUN_TEST(test_ban_detect_self_driving_to_none);
	RUN_TEST(test_ban_detect_enhanced_to_highway);
	RUN_TEST(test_ban_detect_self_driving_to_enhanced);
	RUN_TEST(test_ban_detect_no_change);
	RUN_TEST(test_ban_detect_negative_prev);
	RUN_TEST(test_ban_detect_negative_new);
	RUN_TEST(test_ban_detect_both_negative);
	RUN_TEST(test_ban_detect_highway_to_none_not_flagged);
	RUN_TEST(test_ban_detect_basic_to_self_driving_not_flagged);
	RUN_TEST(test_ban_detect_counter_increments);

	return UNITY_END();
}
