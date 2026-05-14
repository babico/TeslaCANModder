/**
 * @file firmware/test/test_native_bms/test_bms.cpp
 * @brief Unit tests for BMS command handling
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
#include "feature/telemetry/bms.h"

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

/** @brief Verify that the base "bms" command is recognized */
void test_bms_command_recognized()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeBmsCmd("bms", s));
}

/** @brief Verify that unknown subcommands and unrelated strings are rejected */
void test_bms_unknown()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeBmsCmd("bms:foo", s));
	TEST_ASSERT_FALSE(executeBmsCmd("foo", s));
	TEST_ASSERT_FALSE(executeBmsCmd("", s));
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_bms_command_recognized);
	RUN_TEST(test_bms_unknown);
	return UNITY_END();
}
