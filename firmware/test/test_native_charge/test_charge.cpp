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
#include "feature/charge.h"

static State makeState() {
  State s = {};
  s.variant = HW4;
  s.hasCharge = true;
  return s;
}

void setUp() {}
void tearDown() {}

void test_charge_start() {
  State s = makeState();
  TEST_ASSERT_TRUE(executeChargeCmd("charge:start", s));
  TEST_ASSERT_EQUAL_UINT8(0x04, s.burstFrame.data[0] & 0x04);
}
void test_charge_stop() {
  State s = makeState();
  s.lastCharge[0] = 0x04;
  TEST_ASSERT_TRUE(executeChargeCmd("charge:stop", s));
  TEST_ASSERT_EQUAL_UINT8(0x00, s.burstFrame.data[0] & 0x04);
}
void test_charge_port_open() {
  State s = makeState();
  TEST_ASSERT_TRUE(executeChargeCmd("charge:port", s));
  TEST_ASSERT_EQUAL_UINT8(0x01, s.burstFrame.data[0] & 0x01);
}
void test_charge_port_alias() {
  State s = makeState();
  TEST_ASSERT_TRUE(executeChargeCmd("chargeport", s));
}
void test_charge_legacy_blocks() {
  State s = makeState();
  s.variant = LEGACY;
  TEST_ASSERT_FALSE(executeChargeCmd("charge:start", s));
}
void test_charge_no_cache_blocks() {
  State s = makeState();
  s.hasCharge = false;
  TEST_ASSERT_FALSE(executeChargeCmd("charge:start", s));
}
void test_charge_unknown_returns_false() {
  State s = makeState();
  TEST_ASSERT_FALSE(executeChargeCmd("charge:turbo", s));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_charge_start);
  RUN_TEST(test_charge_stop);
  RUN_TEST(test_charge_port_open);
  RUN_TEST(test_charge_port_alias);
  RUN_TEST(test_charge_legacy_blocks);
  RUN_TEST(test_charge_no_cache_blocks);
  RUN_TEST(test_charge_unknown_returns_false);
  return UNITY_END();
}
