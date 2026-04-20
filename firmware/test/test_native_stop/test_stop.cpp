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
#include "feature/stop.h"

static State makeState() { State s = {}; s.variant = HW4; s.hasDrive = true; return s; }
void setUp() {}
void tearDown() {}

void test_stop_creep() { State s = makeState(); TEST_ASSERT_TRUE(execStopCmd("stop:creep", s)); }
void test_stop_roll() { State s = makeState(); TEST_ASSERT_TRUE(execStopCmd("stop:roll", s)); }
void test_stop_hold() { State s = makeState(); TEST_ASSERT_TRUE(execStopCmd("stop:hold", s)); }
void test_stop_legacy_blocks() {
  State s = makeState(); s.variant = LEGACY;
  TEST_ASSERT_FALSE(execStopCmd("stop:hold", s));
}
void test_stop_no_drive_blocks() {
  State s = makeState(); s.hasDrive = false;
  TEST_ASSERT_FALSE(execStopCmd("stop:hold", s));
}
void test_stop_unknown() {
  State s = makeState();
  TEST_ASSERT_FALSE(execStopCmd("stop:foo", s));
  TEST_ASSERT_FALSE(execStopCmd("foo", s));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_stop_creep);
  RUN_TEST(test_stop_roll);
  RUN_TEST(test_stop_hold);
  RUN_TEST(test_stop_legacy_blocks);
  RUN_TEST(test_stop_no_drive_blocks);
  RUN_TEST(test_stop_unknown);
  return UNITY_END();
}
