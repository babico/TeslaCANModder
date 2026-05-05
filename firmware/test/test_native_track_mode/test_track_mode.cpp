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
#include "feature/track_mode.h"

static State makeState()
{
	State s = {};
	s.variant = HW4;
	return s;
}
void setUp() {}
void tearDown() {}

void test_trackmode_on()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeTrackModeCmd("trackmode:on", s));
	TEST_ASSERT_TRUE(s.trackModeEnabled);
}
void test_trackmode_off()
{
	State s = makeState();
	s.trackModeEnabled = true;
	TEST_ASSERT_TRUE(executeTrackModeCmd("trackmode:off", s));
	TEST_ASSERT_FALSE(s.trackModeEnabled);
}
void test_trackmode_legacy_blocks()
{
	State s = makeState();
	s.variant = LEGACY;
	TEST_ASSERT_FALSE(executeTrackModeCmd("trackmode:on", s));
}
void test_trackmode_unknown()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeTrackModeCmd("foo", s));
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_trackmode_on);
	RUN_TEST(test_trackmode_off);
	RUN_TEST(test_trackmode_legacy_blocks);
	RUN_TEST(test_trackmode_unknown);
	return UNITY_END();
}
