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
void handleMessage(Frame&, uint8_t, State&) {}
#include "feature/can_sim.h"

static State makeState() { State s = {}; s.variant = HW4; return s; }
void setUp() {}
void tearDown() {}

void test_canSim_start() {
  State s = makeState();
  TEST_ASSERT_TRUE(executeCanSimCmd("simu:start", s));
  TEST_ASSERT_TRUE(s.canSimEnabled);
}
void test_canSim_stop() {
  State s = makeState(); s.canSimEnabled = true;
  TEST_ASSERT_TRUE(executeCanSimCmd("simu:stop", s));
  TEST_ASSERT_FALSE(s.canSimEnabled);
}
void test_canSim_unknown() {
  State s = makeState();
  TEST_ASSERT_FALSE(executeCanSimCmd("simu:foo", s));
  TEST_ASSERT_FALSE(executeCanSimCmd("foo", s));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_canSim_start);
  RUN_TEST(test_canSim_stop);
  RUN_TEST(test_canSim_unknown);
  return UNITY_END();
}
