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
#include "feature/climate.h"

static State makeState()
{
	State s = {};
	s.variant = HW4;
	s.hasClimate = true;
	return s;
}
void setUp() {}
void tearDown() {}

void test_climate_keep()
{
	State s = makeState();
	TEST_ASSERT_TRUE(execClimateCmd("climate:keep", s));
}
void test_climate_off()
{
	State s = makeState();
	TEST_ASSERT_TRUE(execClimateCmd("climate:off", s));
}
void test_climate_legacy_blocks()
{
	State s = makeState();
	s.variant = LEGACY;
	TEST_ASSERT_FALSE(execClimateCmd("climate:keep", s));
}
void test_climate_no_cache_blocks()
{
	State s = makeState();
	s.hasClimate = false;
	TEST_ASSERT_FALSE(execClimateCmd("climate:keep", s));
}
void test_climate_unknown()
{
	State s = makeState();
	TEST_ASSERT_FALSE(execClimateCmd("climate:on", s));
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
