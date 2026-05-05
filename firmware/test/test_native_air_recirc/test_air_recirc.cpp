#include <unity.h>
#include <cstring>

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

#include "core/types.h"
#include "vehicle/can/burst.h"
void saveSettings(const State &) {}
void resetHandlerLogFlags() {}
void applyFilters(State &) {}
#include "feature/air_recirc.h"

static State makeState()
{
	State s = {};
	s.variant = HW4;
	s.hasClimate = true;
	return s;
}
void setUp() {}
void tearDown() {}

void test_airecirc_on()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeAirRecircCmd("airecirc:on", s));
}
void test_airecirc_off()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeAirRecircCmd("airecirc:off", s));
}
void test_airecirc_unknown()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeAirRecircCmd("airecirc:maybe", s));
	TEST_ASSERT_FALSE(executeAirRecircCmd("foo", s));
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_airecirc_on);
	RUN_TEST(test_airecirc_off);
	RUN_TEST(test_airecirc_unknown);
	return UNITY_END();
}
