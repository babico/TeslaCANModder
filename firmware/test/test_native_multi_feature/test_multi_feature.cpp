/** @file firmware/test/test_native_multi_feature/test_multi_feature.cpp
 *  @brief Unit tests for multi-feature command dispatch
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
unsigned long millis()
{
	return 0;
}
void saveSettings(const State &) {}
void resetHandlerLogFlags() {}
void applyFilters(State &) {}

#include "feature/comfort/precondition.h"
#include "feature/body/track_mode.h"
#include "feature/body/sentry.h"
#include "feature/comfort/wiper.h"
#include "feature/body/mirror.h"
#include "feature/misc/can_raw.h"
#include "feature/misc/stream.h"
#include "feature/drive/pedal.h"
#include "feature/drive/drive_mode.h"

void driverSend(const Frame &, uint8_t) {}

void setUp() {}
void tearDown() {}

static State allEnabledState()
{
	State s = {};
	s.variant = HW4;
	s.hasDrive = true;
	s.hasCtrl = true;
	s.hasCharge = true;
	s.hasClimate = true;
	return s;
}

void test_independent_persist_flags_dont_collide()
{
	State s = allEnabledState();
	TEST_ASSERT_TRUE(executeMirrorAutoFoldCmd("mirror:autofold:on", s));
	TEST_ASSERT_TRUE(executeWiperPersistCmd("wiperpersist:on", s));
	TEST_ASSERT_TRUE(s.mirrorAutoFoldEnabled);
	TEST_ASSERT_TRUE(s.wiperPersistEnabled);
	TEST_ASSERT_TRUE(executeMirrorAutoFoldCmd("mirror:autofold:off", s));
	TEST_ASSERT_FALSE(s.mirrorAutoFoldEnabled);
	TEST_ASSERT_TRUE(s.wiperPersistEnabled);
}

void test_multiple_state_features_active_simultaneously()
{
	State s = allEnabledState();
	TEST_ASSERT_TRUE(executePreconditionCmd("precondition:on", s));
	TEST_ASSERT_TRUE(executeTrackModeCmd("trackmode:on", s));
	TEST_ASSERT_TRUE(executeSentryCmd("sentry:on", s));
	TEST_ASSERT_TRUE(executeCanRawCmd("can:raw:on", s));
	TEST_ASSERT_TRUE(executeStreamCmd("stream:on", s));

	TEST_ASSERT_TRUE(s.preconditionEnabled);
	TEST_ASSERT_TRUE(s.trackModeEnabled);
	TEST_ASSERT_TRUE(s.rawCanListen);
	TEST_ASSERT_TRUE(s.streamEnabled);
}

void test_disabling_one_feature_leaves_others_intact()
{
	State s = allEnabledState();
	s.preconditionEnabled = true;
	s.trackModeEnabled = true;
	TEST_ASSERT_TRUE(executePreconditionCmd("precondition:off", s));
	TEST_ASSERT_FALSE(s.preconditionEnabled);
	TEST_ASSERT_TRUE(s.trackModeEnabled);
}

void test_handlers_do_not_steal_each_others_commands()
{
	State s = allEnabledState();
	TEST_ASSERT_FALSE(executePreconditionCmd("trackmode:on", s));
	TEST_ASSERT_FALSE(executeTrackModeCmd("precondition:on", s));
	TEST_ASSERT_FALSE(executeSentryCmd("stream:on", s));
	TEST_ASSERT_FALSE(executeStreamCmd("sentry:on", s));
	TEST_ASSERT_FALSE(s.preconditionEnabled);
	TEST_ASSERT_FALSE(s.trackModeEnabled);
	TEST_ASSERT_FALSE(s.streamEnabled);
}

/**
 * @brief 0x334 pedal mode and drive mode override must use separate byte-0 fields.
 *
 * Pedal mode lives in bits 2:0 (mask 0x07); drive mode override lives in
 * bits 6:5 (mask 0x60). Whichever feature writes last must preserve the other's
 * field. Both builders start from the cached lastDrive payload.
 */
void test_pedal_and_drive_mode_bitfields_do_not_collide()
{
	State s = allEnabledState();

	// Pedal write first, then drive-mode rebuild from that frame.
	TEST_ASSERT_TRUE(executePedalCmd("pedal:chill", s));
	uint8_t withPedal[8];
	memcpy(withPedal, s.burstFrame.data, 8);
	Frame f = buildDriveModeFrame(DRIVE_MODE_PERFORMANCE, withPedal);
	TEST_ASSERT_EQUAL_UINT8(0x01, f.data[0] & 0x07);
	TEST_ASSERT_EQUAL_UINT8(0x40, f.data[0] & 0x60);

	// Reverse order: pedal write must preserve the drive-mode field.
	memcpy(s.lastDrive, f.data, 8);
	TEST_ASSERT_TRUE(executePedalCmd("pedal:sport", s));
	TEST_ASSERT_EQUAL_UINT8(0x02, s.burstFrame.data[0] & 0x07);
	TEST_ASSERT_EQUAL_UINT8(0x40, s.burstFrame.data[0] & 0x60);
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_independent_persist_flags_dont_collide);
	RUN_TEST(test_multiple_state_features_active_simultaneously);
	RUN_TEST(test_disabling_one_feature_leaves_others_intact);
	RUN_TEST(test_handlers_do_not_steal_each_others_commands);
	RUN_TEST(test_pedal_and_drive_mode_bitfields_do_not_collide);
	return UNITY_END();
}

