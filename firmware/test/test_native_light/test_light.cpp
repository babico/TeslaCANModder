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
#include "feature/light.h"

static State makeState() { State s = {}; s.variant = HW4; s.hasCtrl = true; return s; }
void setUp() {}
void tearDown() {}

void test_light_fog_front() { State s = makeState(); TEST_ASSERT_TRUE(execLightCmd("light:fog:front", s)); }
void test_light_fog_rear() { State s = makeState(); TEST_ASSERT_TRUE(execLightCmd("light:fog:rear", s)); }
void test_light_highbeam_auto() { State s = makeState(); TEST_ASSERT_TRUE(execLightCmd("light:highbeam:auto", s)); }
void test_light_ambient() { State s = makeState(); TEST_ASSERT_TRUE(execLightCmd("light:ambient", s)); }
void test_light_home() { State s = makeState(); TEST_ASSERT_TRUE(execLightCmd("light:home", s)); }
void test_light_dome_off() { State s = makeState(); TEST_ASSERT_TRUE(execLightCmd("light:dome:off", s)); }
void test_light_dome_on() { State s = makeState(); TEST_ASSERT_TRUE(execLightCmd("light:dome:on", s)); }
void test_light_dome_auto() { State s = makeState(); TEST_ASSERT_TRUE(execLightCmd("light:dome:auto", s)); }
void test_light_no_ctrl() {
  State s = makeState(); s.hasCtrl = false;
  TEST_ASSERT_FALSE(execLightCmd("light:ambient", s));
}
void test_light_unknown() {
  State s = makeState();
  TEST_ASSERT_FALSE(execLightCmd("light:bogus", s));
  TEST_ASSERT_FALSE(execLightCmd("foo", s));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_light_fog_front);
  RUN_TEST(test_light_fog_rear);
  RUN_TEST(test_light_highbeam_auto);
  RUN_TEST(test_light_ambient);
  RUN_TEST(test_light_home);
  RUN_TEST(test_light_dome_off);
  RUN_TEST(test_light_dome_on);
  RUN_TEST(test_light_dome_auto);
  RUN_TEST(test_light_no_ctrl);
  RUN_TEST(test_light_unknown);
  return UNITY_END();
}
