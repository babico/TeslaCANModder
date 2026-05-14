/**
 * @file firmware/test/test_native_climate/test_climate.cpp
 * @brief Unit tests for climate control commands
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
#include "feature/comfort/climate.h"

static State makeState()
{
	State s = {};
	s.variant = HW4;
	s.hasClimate = true;
	return s;
}

/** @brief Reset test state before each test */
void setUp() {}

/** @brief Cleanup after each test */
void tearDown() {}

/** @brief "climate:keep" is accepted when climate cache is available */
void test_climate_keep()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeClimateCmd("climate:keep", s));
}

/** @brief "climate:off" is accepted when climate cache is available */
void test_climate_off()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeClimateCmd("climate:off", s));
}

/** @brief Climate commands are blocked on LEGACY variant */
void test_climate_legacy_blocks()
{
	State s = makeState();
	s.variant = LEGACY;
	TEST_ASSERT_FALSE(executeClimateCmd("climate:keep", s));
}

/** @brief Climate commands are blocked when no climate cache is available */
void test_climate_no_cache_blocks()
{
	State s = makeState();
	s.hasClimate = false;
	TEST_ASSERT_FALSE(executeClimateCmd("climate:keep", s));
}

/** @brief Unknown climate subcommands return false */
void test_climate_unknown()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeClimateCmd("climate:on", s));
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_climate_keep);
	RUN_TEST(test_climate_off);
	RUN_TEST(test_climate_legacy_blocks);
	RUN_TEST(test_climate_no_cache_blocks);
	RUN_TEST(test_climate_unknown);
	return UNITY_END();
}
