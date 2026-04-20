// ── Multi-Feature Integration ──────────────────────────────────────────────
// Verifies that multiple feature handlers run independently, mutating their
// own State fields without trampling each other's flags.

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

#include "feature/precondition.h"
#include "feature/track_mode.h"
#include "feature/sentry.h"
#include "feature/wiper.h"
#include "feature/mirror.h"
#include "feature/can_raw.h"
#include "feature/stream.h"

void setUp() {}
void tearDown() {}

static State allEnabledState() {
  State s = {};
  s.variant = HW4;
  s.hasDrive = true; s.hasCtrl = true; s.hasCharge = true; s.hasClimate = true;
  return s;
}

void test_independent_persist_flags_dont_collide() {
  State s = allEnabledState();
  TEST_ASSERT_TRUE(execMirrorAutoFoldCmd("mirror:autofold:on", s));
  TEST_ASSERT_TRUE(execWiperPersistCmd("wiperpersist:on", s));
  TEST_ASSERT_TRUE(s.mirrorAutoFoldEnabled);
  TEST_ASSERT_TRUE(s.wiperPersistEnabled);
  // Toggling one must not toggle the other.
  TEST_ASSERT_TRUE(execMirrorAutoFoldCmd("mirror:autofold:off", s));
  TEST_ASSERT_FALSE(s.mirrorAutoFoldEnabled);
  TEST_ASSERT_TRUE(s.wiperPersistEnabled);
}

void test_multiple_state_features_active_simultaneously() {
  State s = allEnabledState();
  TEST_ASSERT_TRUE(execPreconditionCmd("precondition:on", s));
  TEST_ASSERT_TRUE(execTrackModeCmd("trackmode:on", s));
  TEST_ASSERT_TRUE(execSentryCmd("sentry:on", s));
  TEST_ASSERT_TRUE(executeCanRawCmd("can:raw:on", s));
  TEST_ASSERT_TRUE(executeStreamCmd("stream:on", s));

  TEST_ASSERT_TRUE(s.preconditionEnabled);
  TEST_ASSERT_TRUE(s.trackModeEnabled);
  TEST_ASSERT_TRUE(s.rawCanListen);
  TEST_ASSERT_TRUE(s.streamEnabled);
}

void test_disabling_one_feature_leaves_others_intact() {
  State s = allEnabledState();
  s.preconditionEnabled = true; s.trackModeEnabled = true;
  TEST_ASSERT_TRUE(execPreconditionCmd("precondition:off", s));
  TEST_ASSERT_FALSE(s.preconditionEnabled);
  TEST_ASSERT_TRUE(s.trackModeEnabled);
}

void test_handlers_do_not_steal_each_others_commands() {
  // precondition handler must reject trackmode commands and vice-versa.
  State s = allEnabledState();
  TEST_ASSERT_FALSE(execPreconditionCmd("trackmode:on", s));
  TEST_ASSERT_FALSE(execTrackModeCmd("precondition:on", s));
  TEST_ASSERT_FALSE(execSentryCmd("stream:on", s));
  TEST_ASSERT_FALSE(executeStreamCmd("sentry:on", s));
  TEST_ASSERT_FALSE(s.preconditionEnabled);
  TEST_ASSERT_FALSE(s.trackModeEnabled);
  TEST_ASSERT_FALSE(s.streamEnabled);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_independent_persist_flags_dont_collide);
  RUN_TEST(test_multiple_state_features_active_simultaneously);
  RUN_TEST(test_disabling_one_feature_leaves_others_intact);
  RUN_TEST(test_handlers_do_not_steal_each_others_commands);
  return UNITY_END();
}
