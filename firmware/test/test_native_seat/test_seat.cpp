/** @file firmware/test/test_native_seat/test_seat.cpp
 *  @brief Unit tests for seat adjustment commands
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
#include "feature/comfort/seat.h"

static State makeState()
{
	State s = {};
	s.variant = HW4;
	s.hasCtrl = true;
	return s;
}
void setUp() {}
void tearDown() {}

void test_seat_fl()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeSeatCmd("seat:fl:2", s));
}
void test_seat_fr()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeSeatCmd("seat:fr:0", s));
}
void test_seat_rl()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeSeatCmd("seat:rl:1", s));
}
void test_seat_rr()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeSeatCmd("seat:rr:3", s));
}
void test_seat_rc()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeSeatCmd("seat:rc:1", s));
}
void test_seat_invalid_level()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeSeatCmd("seat:fl:4", s));
	TEST_ASSERT_FALSE(executeSeatCmd("seat:fl:9", s));
}
void test_seat_invalid_position()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeSeatCmd("seat:xx:1", s));
}
void test_seat_no_ctrl()
{
	State s = makeState();
	s.hasCtrl = false;
	TEST_ASSERT_FALSE(executeSeatCmd("seat:fl:1", s));
}
void test_seat_unknown()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeSeatCmd("foo", s));
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_seat_fl);
	RUN_TEST(test_seat_fr);
	RUN_TEST(test_seat_rl);
	RUN_TEST(test_seat_rr);
	RUN_TEST(test_seat_rc);
	RUN_TEST(test_seat_invalid_level);
	RUN_TEST(test_seat_invalid_position);
	RUN_TEST(test_seat_no_ctrl);
	RUN_TEST(test_seat_unknown);
	return UNITY_END();
}

