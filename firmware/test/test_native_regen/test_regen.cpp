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
#include "feature/regen.h"

static State makeState()
{
	State s = {};
	s.variant = HW4;
	s.hasDrive = true;
	return s;
}
void setUp() {}
void tearDown() {}

void test_regen_off()
{
	State s = makeState();
	TEST_ASSERT_TRUE(execRegenCmd("regen:off", s));
	TEST_ASSERT_EQUAL_UINT8(0, s.burstFrame.data[2]);
}
void test_regen_low()
{
	State s = makeState();
	TEST_ASSERT_TRUE(execRegenCmd("regen:low", s));
	TEST_ASSERT_EQUAL_UINT8(50, s.burstFrame.data[2]);
}
void test_regen_standard()
{
	State s = makeState();
	TEST_ASSERT_TRUE(execRegenCmd("regen:standard", s));
	TEST_ASSERT_EQUAL_UINT8(100, s.burstFrame.data[2]);
}
void test_regen_std_alias()
{
	State s = makeState();
	TEST_ASSERT_TRUE(execRegenCmd("regen:std", s));
}
void test_regen_max()
{
	State s = makeState();
	TEST_ASSERT_TRUE(execRegenCmd("regen:max", s));
	TEST_ASSERT_EQUAL_UINT8(200, s.burstFrame.data[2]);
}
void test_regen_legacy_blocks()
{
	State s = makeState();
	s.variant = LEGACY;
	TEST_ASSERT_FALSE(execRegenCmd("regen:max", s));
}
void test_regen_no_drive_blocks()
{
	State s = makeState();
	s.hasDrive = false;
	TEST_ASSERT_FALSE(execRegenCmd("regen:max", s));
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_regen_off);
	RUN_TEST(test_regen_low);
	RUN_TEST(test_regen_standard);
	RUN_TEST(test_regen_std_alias);
	RUN_TEST(test_regen_max);
	RUN_TEST(test_regen_legacy_blocks);
	RUN_TEST(test_regen_no_drive_blocks);
	return UNITY_END();
}
