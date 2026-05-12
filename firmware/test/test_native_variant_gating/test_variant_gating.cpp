/** @file firmware/test/test_native_variant_gating/test_variant_gating.cpp
 *  @brief Unit tests for feature gating by variant
 *  @author Tesla CAN Mod Contributors
 *  @license GPL-3.0
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
unsigned long millis()
{
	return 0;
}
void saveSettings(const State &) {}
void resetHandlerLogFlags() {}
void applyFilters(State &) {}

#include "feature/regen.h"
#include "feature/precondition.h"
#include "feature/track_mode.h"
#include "feature/window.h"
#include "feature/stop.h"
#include "feature/charge.h"
#include "feature/sentry.h"
#include "feature/trunk.h"
#include "feature/climate.h"

void setUp() {}
void tearDown() {}

static State hw4State()
{
	State s = {};
	s.variant = HW4;
	s.hasDrive = true;
	s.hasCtrl = true;
	s.hasCharge = true;
	s.hasClimate = true;
	return s;
}
static State legacyState()
{
	State s = hw4State();
	s.variant = LEGACY;
	return s;
}

void test_legacy_blocks_regen()
{
	State s = legacyState();
	TEST_ASSERT_FALSE(executeRegenCmd("regen:max", s));
}
void test_legacy_blocks_precondition()
{
	State s = legacyState();
	TEST_ASSERT_FALSE(executePreconditionCmd("precondition:on", s));
}
void test_legacy_blocks_trackmode()
{
	State s = legacyState();
	TEST_ASSERT_FALSE(executeTrackModeCmd("trackmode:on", s));
}
void test_legacy_blocks_window_vent()
{
	State s = legacyState();
	TEST_ASSERT_FALSE(executeWindowCmd("window:vent:open", s));
}
void test_legacy_blocks_stop_modes()
{
	State s = legacyState();
	TEST_ASSERT_FALSE(executeStopCmd("stop:hold", s));
}
void test_legacy_blocks_charge()
{
	State s = legacyState();
	TEST_ASSERT_FALSE(executeChargeCmd("charge:start", s));
}
void test_legacy_blocks_sentry()
{
	State s = legacyState();
	TEST_ASSERT_FALSE(executeSentryCmd("sentry:on", s));
}
void test_legacy_blocks_trunk()
{
	State s = legacyState();
	TEST_ASSERT_FALSE(executeTrunkCmd("trunk:open", s));
}
void test_legacy_blocks_climate_keep()
{
	State s = legacyState();
	TEST_ASSERT_FALSE(executeClimateCmd("climate:keep", s));
}

void test_hw4_allows_all_modern_features()
{
	State s = hw4State();
	TEST_ASSERT_TRUE(executeRegenCmd("regen:max", s));
	TEST_ASSERT_TRUE(executePreconditionCmd("precondition:on", s));
	TEST_ASSERT_TRUE(executeTrackModeCmd("trackmode:on", s));
	TEST_ASSERT_TRUE(executeWindowCmd("window:vent:open", s));
	TEST_ASSERT_TRUE(executeStopCmd("stop:hold", s));
	TEST_ASSERT_TRUE(executeChargeCmd("charge:start", s));
	TEST_ASSERT_TRUE(executeSentryCmd("sentry:on", s));
}
void test_hw3_also_allows_modern_features()
{
	State s = hw4State();
	s.variant = HW3;
	TEST_ASSERT_TRUE(executeRegenCmd("regen:standard", s));
	TEST_ASSERT_TRUE(executePreconditionCmd("precondition:off", s));
	TEST_ASSERT_TRUE(executeTrackModeCmd("trackmode:off", s));
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_legacy_blocks_regen);
	RUN_TEST(test_legacy_blocks_precondition);
	RUN_TEST(test_legacy_blocks_trackmode);
	RUN_TEST(test_legacy_blocks_window_vent);
	RUN_TEST(test_legacy_blocks_stop_modes);
	RUN_TEST(test_legacy_blocks_charge);
	RUN_TEST(test_legacy_blocks_sentry);
	RUN_TEST(test_legacy_blocks_trunk);
	RUN_TEST(test_legacy_blocks_climate_keep);
	RUN_TEST(test_hw4_allows_all_modern_features);
	RUN_TEST(test_hw3_also_allows_modern_features);
	return UNITY_END();
}

