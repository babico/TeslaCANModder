/** @file firmware/test/test_native_mirror_autofold/test_mirror_autofold.cpp
 *  @brief Unit tests for mirror auto-fold on lock
 *  @author Tesla CAN Mod Contributors
 *  @license GPL-3.0
 */

#include <unity.h>
#include <cstring>

class __FlashStringHelper;
#define F(s) (reinterpret_cast<const __FlashStringHelper *>(s))

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

#include "core/types.h"
#include "vehicle/can/ids.h"

void saveSettings(const State &) {}
void sendLog(const char *) {}
void sendLog(const __FlashStringHelper *) {}

#include "feature/mirror.h"

void setUp() {}
void tearDown() {}


void test_fold_on_lock()
{
	State s = {};
	s.mirrorAutoFoldEnabled = true;
	s.hasCtrl = true;
	s.vehicleLockedState = false;
	mirrorAutoFoldCheck(s, true);
	TEST_ASSERT_TRUE(s.vehicleLockedState);
	TEST_ASSERT_TRUE(s.burstRemaining > 0);
}

void test_unfold_on_unlock()
{
	State s = {};
	s.mirrorAutoFoldEnabled = true;
	s.hasCtrl = true;
	s.vehicleLockedState = true;
	mirrorAutoFoldCheck(s, false);
	TEST_ASSERT_FALSE(s.vehicleLockedState);
	TEST_ASSERT_TRUE(s.burstRemaining > 0);
}

void test_no_action_same_state()
{
	State s = {};
	s.mirrorAutoFoldEnabled = true;
	s.hasCtrl = true;
	s.vehicleLockedState = true;
	mirrorAutoFoldCheck(s, true);
	TEST_ASSERT_EQUAL(0, s.burstRemaining);
}

void test_disabled_does_nothing()
{
	State s = {};
	s.mirrorAutoFoldEnabled = false;
	s.hasCtrl = true;
	s.vehicleLockedState = false;
	mirrorAutoFoldCheck(s, true);
	TEST_ASSERT_EQUAL(0, s.burstRemaining);
}

void test_no_ctrl_does_nothing()
{
	State s = {};
	s.mirrorAutoFoldEnabled = true;
	s.hasCtrl = false;
	s.vehicleLockedState = false;
	mirrorAutoFoldCheck(s, true);
	TEST_ASSERT_EQUAL(0, s.burstRemaining);
}


void test_cmd_on()
{
	State s = {};
	TEST_ASSERT_TRUE(executeMirrorAutoFoldCmd("mirror:autofold:on", s));
	TEST_ASSERT_TRUE(s.mirrorAutoFoldEnabled);
}

void test_cmd_off()
{
	State s = {};
	s.mirrorAutoFoldEnabled = true;
	TEST_ASSERT_TRUE(executeMirrorAutoFoldCmd("mirror:autofold:off", s));
	TEST_ASSERT_FALSE(s.mirrorAutoFoldEnabled);
}

void test_cmd_unknown()
{
	State s = {};
	TEST_ASSERT_FALSE(executeMirrorAutoFoldCmd("mirror:autofold:toggle", s));
}


int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_fold_on_lock);
	RUN_TEST(test_unfold_on_unlock);
	RUN_TEST(test_no_action_same_state);
	RUN_TEST(test_disabled_does_nothing);
	RUN_TEST(test_no_ctrl_does_nothing);
	RUN_TEST(test_cmd_on);
	RUN_TEST(test_cmd_off);
	RUN_TEST(test_cmd_unknown);
	return UNITY_END();
}

