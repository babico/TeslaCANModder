/** @file firmware/test/test_native_stream/test_stream.cpp
 *  @brief Unit tests for CAN stream output formatting
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
void saveSettings(const State &) {}
void resetHandlerLogFlags() {}
void applyFilters(State &) {}
#include "feature/stream.h"

static State makeState()
{
	State s = {};
	s.variant = HW4;
	return s;
}
void setUp() {}
void tearDown() {}

void test_stream_on_resets_count()
{
	State s = makeState();
	s.streamCount = 99;
	TEST_ASSERT_TRUE(executeStreamCmd("stream:on", s));
	TEST_ASSERT_TRUE(s.streamEnabled);
	TEST_ASSERT_EQUAL_UINT32(0, s.streamCount);
}
void test_stream_off()
{
	State s = makeState();
	s.streamEnabled = true;
	TEST_ASSERT_TRUE(executeStreamCmd("stream:off", s));
	TEST_ASSERT_FALSE(s.streamEnabled);
}
void test_stream_unknown()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeStreamCmd("stream:bogus", s));
	TEST_ASSERT_FALSE(executeStreamCmd("foo", s));
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_stream_on_resets_count);
	RUN_TEST(test_stream_off);
	RUN_TEST(test_stream_unknown);
	return UNITY_END();
}

