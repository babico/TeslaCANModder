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
#include "feature/window.h"

static State makeState()
{
	State s = {};
	s.variant = HW4;
	return s;
}
void setUp() {}
void tearDown() {}

void test_window_vent_open()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeWindowCmd("window:vent:open", s));
	TEST_ASSERT_EQUAL_UINT8(100, s.burstFrame.data[1]);
}
void test_window_vent_close()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeWindowCmd("window:vent:close", s));
	TEST_ASSERT_EQUAL_UINT8(0, s.burstFrame.data[1]);
}
void test_window_vent_alias()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeWindowCmd("vent:open", s));
	TEST_ASSERT_TRUE(executeWindowCmd("vent:close", s));
}
void test_window_vent_position()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeWindowCmd("window:vent:50", s));
	TEST_ASSERT_EQUAL_UINT8(50, s.burstFrame.data[1]);
}
void test_window_vent_invalid_position()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeWindowCmd("window:vent:101", s));
	TEST_ASSERT_FALSE(executeWindowCmd("window:vent:abc", s));
	TEST_ASSERT_FALSE(executeWindowCmd("window:vent:", s));
}
void test_window_legacy_blocks()
{
	State s = makeState();
	s.variant = LEGACY;
	TEST_ASSERT_FALSE(executeWindowCmd("window:vent:open", s));
}
void test_window_unknown()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeWindowCmd("foo", s));
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_window_vent_open);
	RUN_TEST(test_window_vent_close);
	RUN_TEST(test_window_vent_alias);
	RUN_TEST(test_window_vent_position);
	RUN_TEST(test_window_vent_invalid_position);
	RUN_TEST(test_window_legacy_blocks);
	RUN_TEST(test_window_unknown);
	return UNITY_END();
}
