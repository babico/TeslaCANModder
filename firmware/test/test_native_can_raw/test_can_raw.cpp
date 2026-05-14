/**
 * @file firmware/test/test_native_can_raw/test_can_raw.cpp
 * @brief Unit tests for raw CAN listen mode commands
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
#include "feature/misc/can_raw.h"

static State makeState()
{
	State s = {};
	s.variant = HW4;
	return s;
}

/** @brief Reset test state before each test */
void setUp() {}

/** @brief Cleanup after each test */
void tearDown() {}

/** @brief "can:raw:on" enables raw CAN listen mode */
void test_canraw_on()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeCanRawCmd("can:raw:on", s));
	TEST_ASSERT_TRUE(s.rawCanListen);
}

/** @brief "can:raw:off" disables raw CAN listen mode */
void test_canraw_off()
{
	State s = makeState();
	s.rawCanListen = true;
	TEST_ASSERT_TRUE(executeCanRawCmd("can:raw:off", s));
	TEST_ASSERT_FALSE(s.rawCanListen);
}

/** @brief Unknown subcommands and unrelated strings are rejected */
void test_canraw_unknown()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeCanRawCmd("can:raw:bogus", s));
	TEST_ASSERT_FALSE(executeCanRawCmd("foo", s));
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_canraw_on);
	RUN_TEST(test_canraw_off);
	RUN_TEST(test_canraw_unknown);
	return UNITY_END();
}
