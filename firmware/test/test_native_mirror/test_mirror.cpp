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
#include "feature/mirror.h"

static State makeState()
{
	State s = {};
	s.variant = HW4;
	s.hasCtrl = true;
	return s;
}
void setUp() {}
void tearDown() {}

void test_mirror_fold()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeMirrorCmd("mirror:fold", s));
}
void test_mirror_unfold()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeMirrorCmd("mirror:unfold", s));
}
void test_mirror_heat()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeMirrorCmd("mirror:heat", s));
}
void test_mirror_autofold_now()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeMirrorCmd("mirror:autofold", s));
}
void test_mirror_dip()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeMirrorCmd("mirror:dip", s));
}
void test_mirror_no_ctrl()
{
	State s = makeState();
	s.hasCtrl = false;
	TEST_ASSERT_FALSE(executeMirrorCmd("mirror:fold", s));
}
void test_mirror_unknown()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeMirrorCmd("mirror:foo", s));
}
void test_mirror_autofold_persist_on()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeMirrorAutoFoldCmd("mirror:autofold:on", s));
	TEST_ASSERT_TRUE(s.mirrorAutoFoldEnabled);
}
void test_mirror_autofold_persist_off()
{
	State s = makeState();
	s.mirrorAutoFoldEnabled = true;
	TEST_ASSERT_TRUE(executeMirrorAutoFoldCmd("mirror:autofold:off", s));
	TEST_ASSERT_FALSE(s.mirrorAutoFoldEnabled);
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_mirror_fold);
	RUN_TEST(test_mirror_unfold);
	RUN_TEST(test_mirror_heat);
	RUN_TEST(test_mirror_autofold_now);
	RUN_TEST(test_mirror_dip);
	RUN_TEST(test_mirror_no_ctrl);
	RUN_TEST(test_mirror_unknown);
	RUN_TEST(test_mirror_autofold_persist_on);
	RUN_TEST(test_mirror_autofold_persist_off);
	return UNITY_END();
}
