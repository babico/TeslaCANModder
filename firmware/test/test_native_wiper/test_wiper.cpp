/** @file firmware/test/test_native_wiper/test_wiper.cpp
 *  @brief Unit tests for wiper control commands
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
#include "feature/comfort/wiper.h"

static State makeState()
{
	State s = {};
	s.variant = HW4;
	s.hasCtrl = true;
	return s;
}
void setUp() {}
void tearDown() {}

void test_wiper_off()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeWiperCmd("wiper:off", s));
}
void test_wiper_1()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeWiperCmd("wiper:1", s));
}
void test_wiper_2()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeWiperCmd("wiper:2", s));
}
void test_wiper_3()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeWiperCmd("wiper:3", s));
}
void test_wiper_no_ctrl()
{
	State s = makeState();
	s.hasCtrl = false;
	TEST_ASSERT_FALSE(executeWiperCmd("wiper:1", s));
}
void test_wiper_unknown()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeWiperCmd("foo", s));
}
void test_wiperpersist_on()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeWiperPersistCmd("wiperpersist:on", s));
	TEST_ASSERT_TRUE(s.wiperPersistEnabled);
}
void test_wiperpersist_off()
{
	State s = makeState();
	s.wiperPersistEnabled = true;
	TEST_ASSERT_TRUE(executeWiperPersistCmd("wiperpersist:off", s));
	TEST_ASSERT_FALSE(s.wiperPersistEnabled);
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_wiper_off);
	RUN_TEST(test_wiper_1);
	RUN_TEST(test_wiper_2);
	RUN_TEST(test_wiper_3);
	RUN_TEST(test_wiper_no_ctrl);
	RUN_TEST(test_wiper_unknown);
	RUN_TEST(test_wiperpersist_on);
	RUN_TEST(test_wiperpersist_off);
	return UNITY_END();
}

