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
#include "feature/bms.h"

static State makeState() { State s = {}; s.variant = HW4; return s; }
void setUp() {}
void tearDown() {}

void test_bms_command_recognized() {
  State s = makeState();
  TEST_ASSERT_TRUE(execBmsCmd("bms", s));
}
void test_bms_unknown() {
  State s = makeState();
  TEST_ASSERT_FALSE(execBmsCmd("bms:foo", s));
  TEST_ASSERT_FALSE(execBmsCmd("foo", s));
  TEST_ASSERT_FALSE(execBmsCmd("", s));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_bms_command_recognized);
  RUN_TEST(test_bms_unknown);
  return UNITY_END();
}
