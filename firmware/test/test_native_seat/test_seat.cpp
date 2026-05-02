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
#include "feature/seat.h"

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
	TEST_ASSERT_TRUE(execSeatCmd("seat:fl:2", s));
}
void test_seat_fr()
{
	State s = makeState();
	TEST_ASSERT_TRUE(execSeatCmd("seat:fr:0", s));
}
void test_seat_rl()
{
	State s = makeState();
	TEST_ASSERT_TRUE(execSeatCmd("seat:rl:1", s));
}
void test_seat_rr()
{
	State s = makeState();
	TEST_ASSERT_TRUE(execSeatCmd("seat:rr:3", s));
}
void test_seat_rc()
{
	State s = makeState();
	TEST_ASSERT_TRUE(execSeatCmd("seat:rc:1", s));
}
void test_seat_invalid_level()
{
	State s = makeState();
	TEST_ASSERT_FALSE(execSeatCmd("seat:fl:4", s));
	TEST_ASSERT_FALSE(execSeatCmd("seat:fl:9", s));
}
void test_seat_invalid_position()
{
	State s = makeState();
	TEST_ASSERT_FALSE(execSeatCmd("seat:xx:1", s));
}
void test_seat_no_ctrl()
{
	State s = makeState();
	s.hasCtrl = false;
	TEST_ASSERT_FALSE(execSeatCmd("seat:fl:1", s));
}
void test_seat_unknown()
{
	State s = makeState();
	TEST_ASSERT_FALSE(execSeatCmd("foo", s));
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
