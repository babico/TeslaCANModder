/**
 * @file firmware/test/test_native_display/test_display.cpp
 * @brief Unit tests for display brightness commands
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

#include "core/types.h"
void saveSettings(const State &) {}
void resetHandlerLogFlags() {}
void applyFilters(State &) {}
#include "feature/comfort/display.h"

/** @brief Creates a ready State with HW4 variant and ctrl frame available */
static State makeState()
{
	State s = {};
	s.variant = HW4;
	s.hasCtrl = true;
	return s;
}

/** @brief Test fixture setup — no per-test state required */
void setUp() {}

/** @brief Test fixture teardown — no cleanup required */
void tearDown() {}

/** @brief Verifies brightness value 0 (minimum) is accepted */
void test_display_zero()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeDisplayCmd("maindisplay:0", s));
}

/** @brief Verifies brightness value 127 (maximum) is accepted */
void test_display_max()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeDisplayCmd("maindisplay:127", s));
}

/** @brief Verifies out-of-range brightness values are rejected */
void test_display_out_of_range()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeDisplayCmd("maindisplay:128", s));
	TEST_ASSERT_FALSE(executeDisplayCmd("maindisplay:-1", s));
}

/** @brief Verifies command is rejected when ctrl frame is unavailable */
void test_display_no_ctrl()
{
	State s = makeState();
	s.hasCtrl = false;
	TEST_ASSERT_FALSE(executeDisplayCmd("maindisplay:50", s));
}

/** @brief Verifies unrecognized command prefix returns false */
void test_display_unknown()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeDisplayCmd("foo", s));
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_display_zero);
	RUN_TEST(test_display_max);
	RUN_TEST(test_display_out_of_range);
	RUN_TEST(test_display_no_ctrl);
	RUN_TEST(test_display_unknown);
	return UNITY_END();
}
