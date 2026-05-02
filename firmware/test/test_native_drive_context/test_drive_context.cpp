#include <unity.h>
#include <cstring>

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

#include "core/types.h"
#include "infra/can.h"
#include "feature/drive_context.h"

void setUp() {}
void tearDown() {}

void test_decode_latch_states()
{
	Frame f = {};
	f.dlc = 8;
	// Front right latch opened (1), rear right latch closed (2), trunk ajar (5)
	f.data[0] = 0x21;
	f.data[7] = 0x05;

	TEST_ASSERT_TRUE(decodeDoorFrontRightOpen(f));
	TEST_ASSERT_FALSE(decodeDoorRearRightOpen(f));
	TEST_ASSERT_TRUE(decodeTrunkOpen(f));
}

void test_decode_frunk_and_any_door_open_from_mux0()
{
	Frame f = {};
	f.dlc = 8;
	// mux=0, frunk latch opened (1)
	f.data[0] = 0x08;
	// anyDoorOpen bit 50 => byte6 bit2
	f.data[6] = 0x04;

	TEST_ASSERT_TRUE(decodeFrunkOpen(f));
	TEST_ASSERT_TRUE(decodeAnyDoorOpen(f));
}

void test_decode_driver_door_open()
{
	Frame f = {};
	f.dlc = 8;
	// bit31 = 0 means open
	f.data[3] = 0x00;
	TEST_ASSERT_TRUE(decodeDriverDoorOpen(f));

	f.data[3] = 0x80;
	TEST_ASSERT_FALSE(decodeDriverDoorOpen(f));
}

void test_decode_cruise_and_limits()
{
	Frame setSpeed = {};
	setSpeed.dlc = 8;
	// raw 600 => 60.0 kph
	setSpeed.data[0] = 0x58;
	setSpeed.data[1] = 0x02;
	TEST_ASSERT_FLOAT_WITHIN(0.1f, 60.0f, decodeCruiseSetSpeedKph(setSpeed));

	Frame accLimit = {};
	accLimit.dlc = 8;
	// raw 300 => 60 mph => 96.56 kph
	accLimit.data[0] = 0x2C;
	accLimit.data[1] = 0x01;
	TEST_ASSERT_FLOAT_WITHIN(0.2f, 96.56f, decodeAccSpeedLimitKph(accLimit));

	Frame mapLimit = {};
	mapLimit.dlc = 8;
	// raw=20 => 100, unitsKph=1
	mapLimit.data[5] = 0x40;
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
