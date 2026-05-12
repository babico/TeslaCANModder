/** @file firmware/test/test_native_power/test_power.cpp
 *  @brief Unit tests for power management commands
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
#include "feature/power.h"

static State makeState()
{
	State s = {};
	s.variant = HW4;
	s.hasCtrl = true;
	return s;
}
void setUp() {}
void tearDown() {}

void test_power_acc_on()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executePowerCmd("power:acc:on", s));
}
void test_power_acc_off()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executePowerCmd("power:acc:off", s));
}
void test_power_ready()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executePowerCmd("power:ready", s));
}
void test_power_off()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executePowerCmd("power:off", s));
}
void test_power_no_ctrl()
{
	State s = makeState();
	s.hasCtrl = false;
	TEST_ASSERT_FALSE(executePowerCmd("power:ready", s));
}
void test_power_unknown()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executePowerCmd("power:foo", s));
	TEST_ASSERT_FALSE(executePowerCmd("foo", s));
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_power_acc_on);
	RUN_TEST(test_power_acc_off);
	RUN_TEST(test_power_ready);
	RUN_TEST(test_power_off);
	RUN_TEST(test_power_no_ctrl);
	RUN_TEST(test_power_unknown);
	return UNITY_END();
}

