/** @file firmware/test/test_native_pedal/test_pedal.cpp
 *  @brief Unit tests for pedal mapping commands
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
#include "feature/pedal.h"

static State makeState()
{
	State s = {};
	s.variant = HW4;
	s.hasDrive = true;
	return s;
}
void setUp() {}
void tearDown() {}

void test_pedal_standard()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executePedalCmd("pedal:standard", s));
	TEST_ASSERT_EQUAL_UINT8(0x00, s.burstFrame.data[0] & 0x60);
}
void test_pedal_std_alias()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executePedalCmd("pedal:std", s));
}
void test_pedal_chill()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executePedalCmd("pedal:chill", s));
	TEST_ASSERT_EQUAL_UINT8(0x20, s.burstFrame.data[0] & 0x60);
}
void test_pedal_sport()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executePedalCmd("pedal:sport", s));
	TEST_ASSERT_EQUAL_UINT8(0x40, s.burstFrame.data[0] & 0x60);
}
void test_pedal_legacy_blocks()
{
	State s = makeState();
	s.variant = LEGACY;
	TEST_ASSERT_FALSE(executePedalCmd("pedal:sport", s));
}
void test_pedal_no_drive_blocks()
{
	State s = makeState();
	s.hasDrive = false;
	TEST_ASSERT_FALSE(executePedalCmd("pedal:sport", s));
}
void test_pedal_unknown()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executePedalCmd("pedal:eco", s));
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_pedal_standard);
	RUN_TEST(test_pedal_std_alias);
	RUN_TEST(test_pedal_chill);
	RUN_TEST(test_pedal_sport);
	RUN_TEST(test_pedal_legacy_blocks);
	RUN_TEST(test_pedal_no_drive_blocks);
	RUN_TEST(test_pedal_unknown);
	return UNITY_END();
}

