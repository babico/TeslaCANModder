/**
 * @file firmware/test/test_native_drive_context/test_drive_context.cpp
 * @brief Unit tests for drive context CAN frame decoding
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <unity.h>
#include <cstring>

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

#include "core/types.h"
#include "vehicle/can/ids.h"
#include "feature/drive_context.h"

/** @brief Test fixture setup — no per-test state required */
void setUp() {}

/** @brief Test fixture teardown — no cleanup required */
void tearDown() {}

/** @brief Verifies door latch and trunk open state decoding from frame bytes */
void test_decode_latch_states()
{
	Frame f = {};
	f.dlc = 8;
	f.data[0] = 0x21;
	f.data[7] = 0x05;

	TEST_ASSERT_TRUE(decodeDoorFrontRightOpen(f));
	TEST_ASSERT_FALSE(decodeDoorRearRightOpen(f));
	TEST_ASSERT_TRUE(decodeTrunkOpen(f));
}

/** @brief Verifies frunk open and any-door-open flags from mux 0 frame */
void test_decode_frunk_and_any_door_open_from_mux0()
{
	Frame f = {};
	f.dlc = 8;
	f.data[0] = 0x08;
	f.data[6] = 0x04;

	TEST_ASSERT_TRUE(decodeFrunkOpen(f));
	TEST_ASSERT_TRUE(decodeAnyDoorOpen(f));
}

/** @brief Verifies driver door open/closed decoding from bit 7 of byte 3 */
void test_decode_driver_door_open()
{
	Frame f = {};
	f.dlc = 8;
	f.data[3] = 0x00;
	TEST_ASSERT_TRUE(decodeDriverDoorOpen(f));

	f.data[3] = 0x80;
	TEST_ASSERT_FALSE(decodeDriverDoorOpen(f));
}

/** @brief Verifies cruise set speed, ACC speed limit, and map speed limit decoding */
void test_decode_cruise_and_limits()
{
	Frame setSpeed = {};
	setSpeed.dlc = 8;
	setSpeed.data[0] = 0x58; // 600 raw = 60.0 kph
	setSpeed.data[1] = 0x02;
	TEST_ASSERT_FLOAT_WITHIN(0.1f, 60.0f, decodeCruiseSetSpeedKph(setSpeed));

	Frame accLimit = {};
	accLimit.dlc = 8;
	accLimit.data[0] = 0x2C; // 300 raw = 96.56 kph
	accLimit.data[1] = 0x01;
	TEST_ASSERT_FLOAT_WITHIN(0.2f, 96.56f, decodeAccSpeedLimitKph(accLimit));

	Frame mapLimit = {};
	mapLimit.dlc = 8;
	mapLimit.data[5] = 0x40; // encodes 100.0 kph across bytes 5-6
	mapLimit.data[6] = 0x54;
	TEST_ASSERT_FLOAT_WITHIN(0.1f, 100.0f, decodeMapSpeedLimitKph(mapLimit));
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_decode_latch_states);
	RUN_TEST(test_decode_frunk_and_any_door_open_from_mux0);
	RUN_TEST(test_decode_driver_door_open);
	RUN_TEST(test_decode_cruise_and_limits);
	return UNITY_END();
}
