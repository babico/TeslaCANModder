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
#include "feature/trunk.h"

static State makeState()
{
	State s = {};
	s.variant = HW4;
	s.hasCtrl = true;
	return s;
}
void setUp() {}
void tearDown() {}

void test_frunk_open()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeTrunkCmd("frunk:open", s));
	TEST_ASSERT_EQUAL_UINT8(0x20, s.burstFrame.data[0] & 0x20);
}
void test_frunk_alias()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeTrunkCmd("frunk", s));
}
void test_trunk_open()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeTrunkCmd("trunk:open", s));
	TEST_ASSERT_EQUAL_UINT8(0x02, s.burstFrame.data[0]);
}
void test_trunk_close()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeTrunkCmd("trunk:close", s));
	TEST_ASSERT_EQUAL_UINT8(0x03, s.burstFrame.data[0]);
}
void test_glovebox()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeTrunkCmd("glovebox", s));
	TEST_ASSERT_EQUAL_UINT8(0x01, s.burstFrame.data[0]);
}
void test_frunk_no_ctrl_blocks()
{
	State s = makeState();
	s.hasCtrl = false;
	TEST_ASSERT_FALSE(executeTrunkCmd("frunk:open", s));
}
void test_trunk_legacy_blocks()
{
	State s = makeState();
	s.variant = LEGACY;
	TEST_ASSERT_FALSE(executeTrunkCmd("trunk:open", s));
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_frunk_open);
	RUN_TEST(test_frunk_alias);
	RUN_TEST(test_trunk_open);
	RUN_TEST(test_trunk_close);
	RUN_TEST(test_glovebox);
	RUN_TEST(test_frunk_no_ctrl_blocks);
	RUN_TEST(test_trunk_legacy_blocks);
	return UNITY_END();
}
