/**
 * @file firmware/test/test_native_can_health/test_can_health.cpp
 * @brief Unit tests for CAN bus health check and reporting
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
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
#include "core/can/health.h"

/** @brief Reset test state before each test */
void setUp() {}

/** @brief Cleanup after each test */
void tearDown() {}

/** @brief All three buses detected when MCP2515 chips report present */
void test_all_buses_detected()
{
	bool mcp[] = {true, true, true};
	CanHealthReport r = checkCanHealth(mcp, false);
	TEST_ASSERT_EQUAL(3, r.configuredCount);
	TEST_ASSERT_EQUAL(3, r.detectedCount);
	TEST_ASSERT_TRUE(r.allDetected);
	TEST_ASSERT_TRUE(r.anyDetected);
}

/** @brief Missing chassis bus reduces detected count and clears allDetected */
void test_chassis_missing()
{
	bool mcp[] = {false, true, true};
	CanHealthReport r = checkCanHealth(mcp, false);
	TEST_ASSERT_EQUAL(3, r.configuredCount);
	TEST_ASSERT_EQUAL(2, r.detectedCount);
	TEST_ASSERT_FALSE(r.allDetected);
	TEST_ASSERT_TRUE(r.anyDetected);
	TEST_ASSERT_FALSE(r.bus[0].detected);
}

/** @brief No buses detected sets both allDetected and anyDetected to false */
void test_no_buses_detected()
{
	bool mcp[] = {false, false, false};
	CanHealthReport r = checkCanHealth(mcp, false);
	TEST_ASSERT_EQUAL(3, r.configuredCount);
	TEST_ASSERT_EQUAL(0, r.detectedCount);
	TEST_ASSERT_FALSE(r.allDetected);
	TEST_ASSERT_FALSE(r.anyDetected);
}

/** @brief Chassis online flag marks bus 0 as receiving */
void test_chassis_online_marks_receiving()
{
	bool mcp[] = {true, true, true};
	CanHealthReport r = checkCanHealth(mcp, true);
	TEST_ASSERT_TRUE(r.bus[0].receiving);
	TEST_ASSERT_EQUAL(1, r.receivingCount);
	TEST_ASSERT_FALSE(r.bus[1].receiving);
	TEST_ASSERT_FALSE(r.bus[2].receiving);
}

/** @brief busHealthName returns "disabled" for an unconfigured bus */
void test_bus_health_name_disabled()
{
	BusHealth b = {false, false, false, 0};
	TEST_ASSERT_EQUAL_STRING("disabled", busHealthName(b));
}

/** @brief busHealthName returns "NOT_DETECTED" when configured but chip not found */
void test_bus_health_name_not_detected()
{
	BusHealth b = {true, false, false, 0};
	TEST_ASSERT_EQUAL_STRING("NOT_DETECTED", busHealthName(b));
}

/** @brief busHealthName returns "online" when detected and receiving frames */
void test_bus_health_name_online()
{
	BusHealth b = {true, true, true, 1234};
	TEST_ASSERT_EQUAL_STRING("online", busHealthName(b));
}

/** @brief busHealthName returns "idle" when detected but not receiving */
void test_bus_health_name_idle()
{
	BusHealth b = {true, true, false, 0};
	TEST_ASSERT_EQUAL_STRING("idle", busHealthName(b));
}

/** @brief busIndexName maps indices 0-2 to Chassis, Vehicle, Body */
void test_bus_index_names()
{
	TEST_ASSERT_EQUAL_STRING("Chassis", busIndexName(0));
	TEST_ASSERT_EQUAL_STRING("Vehicle", busIndexName(1));
	TEST_ASSERT_EQUAL_STRING("Body", busIndexName(2));
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_all_buses_detected);
	RUN_TEST(test_chassis_missing);
	RUN_TEST(test_no_buses_detected);
	RUN_TEST(test_chassis_online_marks_receiving);

	RUN_TEST(test_bus_health_name_disabled);
	RUN_TEST(test_bus_health_name_not_detected);
	RUN_TEST(test_bus_health_name_online);
	RUN_TEST(test_bus_health_name_idle);
	RUN_TEST(test_bus_index_names);

	return UNITY_END();
}
