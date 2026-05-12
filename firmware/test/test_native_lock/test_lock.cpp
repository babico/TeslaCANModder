/** @file firmware/test/test_native_lock/test_lock.cpp
 *  @brief Unit tests for lock and unlock commands
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

static int saveCount = 0;
void saveSettings(const State &)
{
	saveCount++;
}
void resetHandlerLogFlags() {}
void applyFilters(State &) {}

#include "feature/lock.h"

static State makeState()
{
	State s = {};
	s.variant = HW4;
	s.hasCtrl = true;
	return s;
}

void setUp()
{
	saveCount = 0;
}
void tearDown() {}

void test_lock()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeLockCmd("lock", s));
	TEST_ASSERT_EQUAL_UINT32(CAN_ID_UI_VEHICLE_CTRL, s.burstFrame.id);
	TEST_ASSERT_TRUE(s.burstRemaining > 0);
	TEST_ASSERT_EQUAL_UINT8(0x02, s.burstFrame.data[2] & 0x0E);
}

void test_unlock()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeLockCmd("unlock", s));
	TEST_ASSERT_EQUAL_UINT8(0x04, s.burstFrame.data[2] & 0x0E);
}

void test_horn()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeLockCmd("horn", s));
	TEST_ASSERT_EQUAL_UINT8(0x20, s.burstFrame.data[7] & 0x20);
}

void test_child_lock()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeLockCmd("lock:child", s));
	TEST_ASSERT_EQUAL_UINT8(0x01, s.burstFrame.data[2] & 0x01);
}

void test_lock_requires_ctrl_cache()
{
	State s = makeState();
	s.hasCtrl = false;
	TEST_ASSERT_FALSE(executeLockCmd("lock", s));
}

void test_lock_unknown_returns_false()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeLockCmd("locks", s));
	TEST_ASSERT_FALSE(executeLockCmd("fsd:on", s));
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_lock);
	RUN_TEST(test_unlock);
	RUN_TEST(test_horn);
	RUN_TEST(test_child_lock);
	RUN_TEST(test_lock_requires_ctrl_cache);
	RUN_TEST(test_lock_unknown_returns_false);
	return UNITY_END();
}

