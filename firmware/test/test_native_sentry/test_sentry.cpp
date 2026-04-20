#include <unity.h>
#include <cstring>

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

#include "core/types.h"
void saveSettings(const State&) {}
void resetHandlerLogFlags() {}
void applyFilters(State&) {}
#include "feature/sentry.h"

static State makeState() { State s = {}; s.variant = HW4; return s; }
void setUp() {}
void tearDown() {}

void test_sentry_on() {
  State s = makeState();
  TEST_ASSERT_TRUE(execSentryCmd("sentry:on", s));
  TEST_ASSERT_EQUAL_UINT8(0x20, s.burstFrame.data[0]);
}
void test_sentry_off() {
  State s = makeState();
  TEST_ASSERT_TRUE(execSentryCmd("sentry:off", s));
  TEST_ASSERT_EQUAL_UINT8(0x00, s.burstFrame.data[0]);
}
void test_sentry_legacy_blocks() {
  State s = makeState(); s.variant = LEGACY;
  TEST_ASSERT_FALSE(execSentryCmd("sentry:on", s));
}
void test_sentry_unknown() {
  State s = makeState();
  TEST_ASSERT_FALSE(execSentryCmd("sentry:auto", s));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_sentry_on);
  RUN_TEST(test_sentry_off);
  RUN_TEST(test_sentry_legacy_blocks);
  RUN_TEST(test_sentry_unknown);
  return UNITY_END();
}
