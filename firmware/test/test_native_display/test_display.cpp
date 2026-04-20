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
#include "feature/display.h"

static State makeState() { State s = {}; s.variant = HW4; s.hasCtrl = true; return s; }
void setUp() {}
void tearDown() {}

void test_display_zero() {
  State s = makeState();
  TEST_ASSERT_TRUE(execDisplayCmd("maindisplay:0", s));
}
void test_display_max() {
  State s = makeState();
  TEST_ASSERT_TRUE(execDisplayCmd("maindisplay:127", s));
}
void test_display_out_of_range() {
  State s = makeState();
  TEST_ASSERT_FALSE(execDisplayCmd("maindisplay:128", s));
  TEST_ASSERT_FALSE(execDisplayCmd("maindisplay:-1", s));
}
void test_display_no_ctrl() {
  State s = makeState(); s.hasCtrl = false;
  TEST_ASSERT_FALSE(execDisplayCmd("maindisplay:50", s));
}
void test_display_unknown() {
  State s = makeState();
  TEST_ASSERT_FALSE(execDisplayCmd("foo", s));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_display_zero);
  RUN_TEST(test_display_max);
  RUN_TEST(test_display_out_of_range);
  RUN_TEST(test_display_no_ctrl);
  RUN_TEST(test_display_unknown);
  return UNITY_END();
}
