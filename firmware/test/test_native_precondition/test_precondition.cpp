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

static State makeState() { State s = {}; s.variant = HW4; return s; }
void setUp() {}
void tearDown() {}

void test_precondition_on() {
  State s = makeState();
  TEST_ASSERT_TRUE(execPreconditionCmd("precondition:on", s));
  TEST_ASSERT_TRUE(s.preconditionEnabled);
}
void test_precondition_off() {
  State s = makeState(); s.preconditionEnabled = true;
  TEST_ASSERT_TRUE(execPreconditionCmd("precondition:off", s));
  TEST_ASSERT_FALSE(s.preconditionEnabled);
}
void test_precondition_legacy_blocks() {
  State s = makeState(); s.variant = LEGACY;
  TEST_ASSERT_FALSE(execPreconditionCmd("precondition:on", s));
}
void test_precondition_unknown() {
  State s = makeState();
  TEST_ASSERT_FALSE(execPreconditionCmd("precondition:bogus", s));
  TEST_ASSERT_FALSE(execPreconditionCmd("nope", s));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_precondition_on);
  RUN_TEST(test_precondition_off);
  RUN_TEST(test_precondition_legacy_blocks);
  RUN_TEST(test_precondition_unknown);
  return UNITY_END();
}
