// ── Powertrain Decode Tests ──────────────────────────────────────────────────
// Tests decoding of motor & drivetrain CAN signals.

#include <unity.h>
#include <cstring>

#include "core/types.h"
#include "infra/can.h"
#include "feature/powertrain.h"

void setUp() {}
void tearDown() {}

// ── decodeVehicleSpeed ──────────────────────────────────────────────────────

void test_speed_zero() {
  uint8_t data[8] = {};
  TEST_ASSERT_FLOAT_WITHIN(0.01, 0.0, decodeVehicleSpeed(data));
}

void test_speed_positive() {
  uint8_t data[8] = {};
  // 6000 = 60.00 km/h
  data[2] = 0x17;
  data[3] = 0x70;
  TEST_ASSERT_FLOAT_WITHIN(0.01, 60.0, decodeVehicleSpeed(data));
}

void test_speed_negative() {
  // int16_t -100 = 0xFF9C → -1.00 km/h
  uint8_t data[8] = {};
  int16_t raw = -100;
  data[2] = (raw >> 8) & 0xFF;
  data[3] = raw & 0xFF;
  TEST_ASSERT_FLOAT_WITHIN(0.01, -1.0, decodeVehicleSpeed(data));
}

// ── decodeGearState ─────────────────────────────────────────────────────────

void test_gear_park() {
  uint8_t data[8] = {};
  data[0] = 1 << 1; // gear=1 (P)
  TEST_ASSERT_EQUAL_UINT8(1, decodeGearState(data));
}

void test_gear_drive() {
  uint8_t data[8] = {};
  data[0] = 4 << 1; // gear=4 (D)
  TEST_ASSERT_EQUAL_UINT8(4, decodeGearState(data));
}

void test_gear_reverse() {
  uint8_t data[8] = {};
  data[0] = 2 << 1; // gear=2 (R)
  TEST_ASSERT_EQUAL_UINT8(2, decodeGearState(data));
}

void test_gear_masks_correctly() {
  uint8_t data[8] = {};
  data[0] = 0xFF; // all bits set → should mask to 0x07
  TEST_ASSERT_EQUAL_UINT8(7, decodeGearState(data));
}

// ── decodeAccelPedal ────────────────────────────────────────────────────────

void test_pedal_zero() {
  uint8_t data[8] = {};
  TEST_ASSERT_EQUAL_UINT8(0, decodeAccelPedal(data));
}

void test_pedal_full() {
  uint8_t data[8] = {};
  data[1] = 100;
  TEST_ASSERT_EQUAL_UINT8(100, decodeAccelPedal(data));
}

// ── decodeSteeringAngle ─────────────────────────────────────────────────────

void test_steering_center() {
  uint8_t data[8] = {};
  TEST_ASSERT_FLOAT_WITHIN(0.1, 0.0, decodeSteeringAngle(data));
}

void test_steering_right() {
  uint8_t data[8] = {};
  int16_t raw = 450; // 45.0 degrees right
  data[0] = (raw >> 8) & 0xFF;
  data[1] = raw & 0xFF;
  TEST_ASSERT_FLOAT_WITHIN(0.1, 45.0, decodeSteeringAngle(data));
}

void test_steering_left() {
  uint8_t data[8] = {};
  int16_t raw = -300; // -30.0 degrees left
  data[0] = (raw >> 8) & 0xFF;
  data[1] = raw & 0xFF;
  TEST_ASSERT_FLOAT_WITHIN(0.1, -30.0, decodeSteeringAngle(data));
}

// ── decodeMotorRpm ──────────────────────────────────────────────────────────

void test_motor_rpm_zero() {
  uint8_t data[8] = {};
  TEST_ASSERT_EQUAL_INT16(0, decodeMotorRpm(data));
}

void test_motor_rpm_positive() {
  uint8_t data[8] = {};
  int16_t rpm = 5000;
  data[4] = (rpm >> 8) & 0xFF;
  data[5] = rpm & 0xFF;
  TEST_ASSERT_EQUAL_INT16(5000, decodeMotorRpm(data));
}

void test_motor_rpm_negative_regen() {
  uint8_t data[8] = {};
  int16_t rpm = -1200;
  data[4] = (rpm >> 8) & 0xFF;
  data[5] = rpm & 0xFF;
  TEST_ASSERT_EQUAL_INT16(-1200, decodeMotorRpm(data));
}

// ── gearName ────────────────────────────────────────────────────────────────

void test_gear_name_park()    { TEST_ASSERT_EQUAL_STRING("P", gearName(1)); }
void test_gear_name_reverse() { TEST_ASSERT_EQUAL_STRING("R", gearName(2)); }
void test_gear_name_neutral() { TEST_ASSERT_EQUAL_STRING("N", gearName(3)); }
void test_gear_name_drive()   { TEST_ASSERT_EQUAL_STRING("D", gearName(4)); }
void test_gear_name_unknown() { TEST_ASSERT_EQUAL_STRING("?", gearName(0)); }

// ── main ────────────────────────────────────────────────────────────────────

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_speed_zero);
  RUN_TEST(test_speed_positive);
  RUN_TEST(test_speed_negative);
  RUN_TEST(test_gear_park);
  RUN_TEST(test_gear_drive);
  RUN_TEST(test_gear_reverse);
  RUN_TEST(test_gear_masks_correctly);
  RUN_TEST(test_pedal_zero);
  RUN_TEST(test_pedal_full);
  RUN_TEST(test_steering_center);
  RUN_TEST(test_steering_right);
  RUN_TEST(test_steering_left);
  RUN_TEST(test_motor_rpm_zero);
  RUN_TEST(test_motor_rpm_positive);
  RUN_TEST(test_motor_rpm_negative_regen);
  RUN_TEST(test_gear_name_park);
  RUN_TEST(test_gear_name_reverse);
  RUN_TEST(test_gear_name_neutral);
  RUN_TEST(test_gear_name_drive);
  RUN_TEST(test_gear_name_unknown);
  return UNITY_END();
}
