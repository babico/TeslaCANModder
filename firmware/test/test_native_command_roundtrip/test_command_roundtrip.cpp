/**
 * @file firmware/test/test_native_command_roundtrip/test_command_roundtrip.cpp
 * @brief Unit tests for full command-to-CAN-burst pipeline integration
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
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

#include "feature/drive/regen.h"
#include "feature/body/window.h"
#include "feature/body/sentry.h"
#include "feature/body/lock.h"

/** @brief Test fixture setup — no per-test state required */
void setUp() {}

/** @brief Test fixture teardown — no cleanup required */
void tearDown() {}

/** @brief Creates a State with all readiness flags set for command execution */
static State makeReady()
{
	State s = {};
	s.variant = HW4;
	s.hasDrive = true;
	s.hasCtrl = true;
	s.hasCharge = true;
	return s;
}

/** @brief Verifies regen:max produces correct CAN ID, DLC, payload byte, and burst metadata */
void test_roundtrip_regen_max_emits_correct_can_id_and_payload()
{
	State s = makeReady();
	TEST_ASSERT_TRUE(executeRegenCmd("regen:max", s));
	TEST_ASSERT_EQUAL_UINT32(CAN_ID_DRIVE_CONFIG, s.burstFrame.id);
	TEST_ASSERT_EQUAL_UINT8(8, s.burstFrame.dlc);
	TEST_ASSERT_EQUAL_UINT8(200, s.burstFrame.data[2]);
	TEST_ASSERT_TRUE(s.burstRemaining > 0);
	TEST_ASSERT_EQUAL_UINT8(BUS_VEHICLE, s.burstBus);
}

/** @brief Verifies window:vent emits the window CAN ID with all-window mask and position */
void test_roundtrip_window_vent_emits_window_id()
{
	State s = makeReady();
	TEST_ASSERT_TRUE(executeWindowCmd("window:vent:75", s));
	TEST_ASSERT_EQUAL_UINT32(CAN_ID_WINDOW_VENT, s.burstFrame.id);
	TEST_ASSERT_EQUAL_UINT8(0x1F, s.burstFrame.data[0]); // all-window bitmask
	TEST_ASSERT_EQUAL_UINT8(75, s.burstFrame.data[1]);
	TEST_ASSERT_EQUAL_UINT8(BUS_BODY, s.burstBus);
}

/** @brief Verifies sentry:on emits a burst with the sentry CAN ID */
void test_roundtrip_sentry_on_emits_burst()
{
	State s = makeReady();
	TEST_ASSERT_TRUE(executeSentryCmd("sentry:on", s));
	TEST_ASSERT_EQUAL_UINT32(CAN_ID_SENTRY, s.burstFrame.id);
	TEST_ASSERT_TRUE(s.burstRemaining > 0);
}

/** @brief Verifies lock command emits a non-zero burst */
void test_roundtrip_lock_emits_burst()
{
	State s = makeReady();
	TEST_ASSERT_TRUE(executeLockCmd("lock", s));
	TEST_ASSERT_TRUE(s.burstRemaining > 0);
}

/** @brief Verifies unrecognized commands return false and leave burst at zero */
void test_roundtrip_unknown_command_does_not_emit_burst()
{
	State s = makeReady();
	TEST_ASSERT_FALSE(executeRegenCmd("unrecognized:foo", s));
	TEST_ASSERT_FALSE(executeWindowCmd("unrecognized:foo", s));
	TEST_ASSERT_FALSE(executeSentryCmd("unrecognized:foo", s));
	TEST_ASSERT_FALSE(executeLockCmd("unrecognized:foo", s));
	TEST_ASSERT_EQUAL_UINT8(0, s.burstRemaining);
}

/** @brief Verifies a second command overwrites the burst frame payload */
void test_roundtrip_subsequent_commands_overwrite_burst_frame()
{
	State s = makeReady();
	TEST_ASSERT_TRUE(executeRegenCmd("regen:low", s));
	TEST_ASSERT_EQUAL_UINT8(50, s.burstFrame.data[2]);
	TEST_ASSERT_TRUE(executeRegenCmd("regen:max", s));
	TEST_ASSERT_EQUAL_UINT8(200, s.burstFrame.data[2]);
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_roundtrip_regen_max_emits_correct_can_id_and_payload);
	RUN_TEST(test_roundtrip_window_vent_emits_window_id);
	RUN_TEST(test_roundtrip_sentry_on_emits_burst);
	RUN_TEST(test_roundtrip_lock_emits_burst);
	RUN_TEST(test_roundtrip_unknown_command_does_not_emit_burst);
	RUN_TEST(test_roundtrip_subsequent_commands_overwrite_burst_frame);
	return UNITY_END();
}
