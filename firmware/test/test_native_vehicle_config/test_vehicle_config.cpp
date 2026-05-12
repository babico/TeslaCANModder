/** @file firmware/test/test_native_vehicle_config/test_vehicle_config.cpp
 *  @brief Unit tests for vehicle configuration decoding
 *  @author Tesla CAN Mod Contributors
 *  @license GPL-3.0
 */

#include <unity.h>
#include <cstring>

class __FlashStringHelper;
#define F(s) (reinterpret_cast<const __FlashStringHelper *>(s))

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

#include "core/types.h"
#include "vehicle/can/ids.h"

void saveSettings(const State &) {}
void sendLog(const char *) {}
void sendLog(const __FlashStringHelper *) {}

#include "feature/vehicle_config.h"

void setUp() {}
void tearDown() {}


void test_decode_model3()
{
	State s = {};
	Frame f = {};
	f.id = 0x398;
	f.dlc = 8;
	f.data[1] = 0x30;
	f.data[2] = 10;
	decodeVehicleConfig(f, s);
	TEST_ASSERT_EQUAL(VEHICLE_MODEL_3, s.vehicleModel);
	TEST_ASSERT_EQUAL(2026, s.vehicleYear);
	TEST_ASSERT_TRUE(s.hasVehicleConfig);
}

void test_decode_model_y()
{
	State s = {};
	Frame f = {};
	f.id = 0x398;
	f.dlc = 8;
	f.data[1] = 0x40;
	f.data[2] = 6;
	decodeVehicleConfig(f, s);
	TEST_ASSERT_EQUAL(VEHICLE_MODEL_Y, s.vehicleModel);
	TEST_ASSERT_EQUAL(2022, s.vehicleYear);
}

void test_decode_model_s()
{
	State s = {};
	Frame f = {};
	f.id = 0x398;
	f.dlc = 8;
	f.data[1] = 0x10;
	f.data[2] = 0;
	decodeVehicleConfig(f, s);
	TEST_ASSERT_EQUAL(VEHICLE_MODEL_S, s.vehicleModel);
}

void test_decode_model_x()
{
	State s = {};
	Frame f = {};
	f.id = 0x398;
	f.dlc = 8;
	f.data[1] = 0x20;
	f.data[2] = 5;
	decodeVehicleConfig(f, s);
	TEST_ASSERT_EQUAL(VEHICLE_MODEL_X, s.vehicleModel);
}

void test_decode_cybertruck()
{
	State s = {};
	Frame f = {};
	f.id = 0x398;
	f.dlc = 8;
	f.data[1] = 0x50;
	f.data[2] = 5;
	decodeVehicleConfig(f, s);
	TEST_ASSERT_EQUAL(VEHICLE_CYBERTRUCK, s.vehicleModel);
}

void test_decode_unknown_platform()
{
	State s = {};
	Frame f = {};
	f.id = 0x398;
	f.dlc = 8;
	f.data[1] = 0xF0;
	decodeVehicleConfig(f, s);
	TEST_ASSERT_EQUAL(VEHICLE_UNKNOWN, s.vehicleModel);
	TEST_ASSERT_TRUE(s.hasVehicleConfig);
}

void test_capabilities_model3()
{
	VehicleCapabilities caps = getVehicleCapabilities(VEHICLE_MODEL_3);
	TEST_ASSERT_TRUE(caps.supportsFsd);
	TEST_ASSERT_TRUE(caps.supportsDualMotor);
}

void test_exec_vehicle_cmd()
{
	State s = {};
	TEST_ASSERT_TRUE(executeVehicleConfigCmd("vehicle", s));
	TEST_ASSERT_FALSE(executeVehicleConfigCmd("fsd:on", s));
}

int main()
{
	UNITY_BEGIN();
	RUN_TEST(test_decode_model3);
	RUN_TEST(test_decode_model_y);
	RUN_TEST(test_decode_model_s);
	RUN_TEST(test_decode_model_x);
	RUN_TEST(test_decode_cybertruck);
	RUN_TEST(test_decode_unknown_platform);
	RUN_TEST(test_capabilities_model3);
	RUN_TEST(test_exec_vehicle_cmd);
	return UNITY_END();
}

