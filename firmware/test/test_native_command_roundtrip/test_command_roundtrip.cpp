// ── Command Roundtrip Integration ──────────────────────────────────────────
// Verifies the full pipeline: command string → feature dispatch → CAN frame
// burst configured correctly. Each test fires a command and asserts on the
// resulting State.burstFrame (id, dlc, key data bytes) and burst counters.

#include <unity.h>
#include <cstring>

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

#include "core/types.h"
unsigned long millis() { return 0; }
void saveSettings(const State&) {}
void resetHandlerLogFlags() {}
void applyFilters(State&) {}

#include "feature/regen.h"
#include "feature/window.h"
#include "feature/sentry.h"
#include "feature/lock.h"

void setUp() {}
void tearDown() {}

static State makeReady() {
  State s = {};
  s.variant = HW4;
  s.hasDrive = true; s.hasCtrl = true; s.hasCharge = true;
  return s;
}

void test_roundtrip_regen_max_emits_correct_can_id_and_payload() {
  State s = makeReady();
  TEST_ASSERT_TRUE(execRegenCmd("regen:max", s));
  TEST_ASSERT_EQUAL_UINT32(CAN_ID_DRIVE_CONFIG, s.burstFrame.id);
  TEST_ASSERT_EQUAL_UINT8(8, s.burstFrame.dlc);
  TEST_ASSERT_EQUAL_UINT8(200, s.burstFrame.data[2]);
  TEST_ASSERT_TRUE(s.burstRemaining > 0);
  TEST_ASSERT_EQUAL_UINT8(BUS_VEHICLE, s.burstBus);
}

void test_roundtrip_window_vent_emits_window_id() {
  State s = makeReady();
  TEST_ASSERT_TRUE(execWindowCmd("window:vent:75", s));
  TEST_ASSERT_EQUAL_UINT32(CAN_ID_WINDOW_VENT, s.burstFrame.id);
  TEST_ASSERT_EQUAL_UINT8(0x1F, s.burstFrame.data[0]);  // all-window mask
  TEST_ASSERT_EQUAL_UINT8(75, s.burstFrame.data[1]);
  TEST_ASSERT_EQUAL_UINT8(BUS_BODY, s.burstBus);
}

void test_roundtrip_sentry_on_emits_burst() {
  State s = makeReady();
  TEST_ASSERT_TRUE(execSentryCmd("sentry:on", s));
  TEST_ASSERT_EQUAL_UINT32(CAN_ID_SENTRY, s.burstFrame.id);
  TEST_ASSERT_TRUE(s.burstRemaining > 0);
}

void test_roundtrip_lock_emits_burst() {
  State s = makeReady();
  TEST_ASSERT_TRUE(execLockCmd("lock", s));
  TEST_ASSERT_TRUE(s.burstRemaining > 0);
}

void test_roundtrip_unknown_command_does_not_emit_burst() {
  State s = makeReady();
  TEST_ASSERT_FALSE(execRegenCmd("unrecognized:foo", s));
  TEST_ASSERT_FALSE(execWindowCmd("unrecognized:foo", s));
  TEST_ASSERT_FALSE(execSentryCmd("unrecognized:foo", s));
  TEST_ASSERT_FALSE(execLockCmd("unrecognized:foo", s));
  TEST_ASSERT_EQUAL_UINT8(0, s.burstRemaining);
}

void test_roundtrip_subsequent_commands_overwrite_burst_frame() {
  State s = makeReady();
  TEST_ASSERT_TRUE(execRegenCmd("regen:low", s));
  TEST_ASSERT_EQUAL_UINT8(50, s.burstFrame.data[2]);
  TEST_ASSERT_TRUE(execRegenCmd("regen:max", s));
  TEST_ASSERT_EQUAL_UINT8(200, s.burstFrame.data[2]);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_roundtrip_regen_max_emits_correct_can_id_and_payload);
  RUN_TEST(test_roundtrip_window_vent_emits_window_id);
  RUN_TEST(test_roundtrip_sentry_on_emits_burst);
  RUN_TEST(test_roundtrip_lock_emits_burst);
  RUN_TEST(test_roundtrip_unknown_command_does_not_emit_burst);
  RUN_TEST(test_roundtrip_subsequent_commands_overwrite_burst_frame);
  return UNITY_END();
}
