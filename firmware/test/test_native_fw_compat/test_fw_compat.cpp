/**
 * @file firmware/test/test_native_fw_compat/test_fw_compat.cpp
 * @brief Unit tests for firmware version compatibility detection
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

void saveSettings(const State &) {}
void sendLog(const char *) {}
void sendLog(const __FlashStringHelper *) {}

#include "feature/telemetry/fw_compat.h"

/** @brief Test fixture setup — no per-test state required */
void setUp() {}

/** @brief Test fixture teardown — no cleanup required */
void tearDown() {}

/** @brief Verifies mux 0 frame decodes year, release, and minor version fields */
void test_decode_mux0_sets_year_release_minor()
{
	State s = {};
	Frame f = {};
	f.id = 0x392;
	f.dlc = 8;
	f.data[0] = 0x00;
	f.data[1] = 0x07; // year high byte
	f.data[2] = 0xE8; // year low byte -> 2024
	f.data[3] = 12;
	f.data[4] = 5;
	decodeFwVersion(f, s);
	TEST_ASSERT_EQUAL(2024, s.fwYear);
	TEST_ASSERT_EQUAL(12, s.fwRelease);
	TEST_ASSERT_EQUAL(5, s.fwMinor);
	TEST_ASSERT_TRUE(s.hasFwVersion);
}

/** @brief Verifies mux 1 frame decodes the build number */
void test_decode_mux1_sets_build()
{
	State s = {};
	Frame f = {};
	f.id = 0x392;
	f.dlc = 8;
	f.data[0] = 0x01;
	f.data[1] = 0x00;
	f.data[2] = 0x00;
	f.data[3] = 0x12;
	f.data[4] = 0x34;
	decodeFwVersion(f, s);
	TEST_ASSERT_EQUAL(0x1234, s.fwBuild);
}

/** @brief Verifies 2024+ firmware is classified as FW_COMPAT_OK */
void test_compat_ok_for_2024_plus()
{
	State s = {};
	s.variant = HW4;
	Frame f = {};
	f.id = 0x392;
	f.dlc = 8;
	f.data[0] = 0x00;
	f.data[1] = 0x07;
	f.data[2] = 0xEA; // year 2026
	f.data[3] = 2;
	f.data[4] = 9;
	decodeFwVersion(f, s);
	TEST_ASSERT_EQUAL(FW_COMPAT_OK, s.fwCompat);
}

/** @brief Verifies pre-2024 firmware is classified as FW_COMPAT_WARN */
void test_compat_warn_for_older()
{
	State s = {};
	s.variant = HW4;
	Frame f = {};
	f.id = 0x392;
	f.dlc = 8;
	f.data[0] = 0x00;
	f.data[1] = 0x07;
	f.data[2] = 0xE7; // year 2023
	f.data[3] = 40;
	f.data[4] = 1;
	decodeFwVersion(f, s);
	TEST_ASSERT_EQUAL(FW_COMPAT_WARN, s.fwCompat);
}

/** @brief Verifies "fwcompat" command string is recognized */
void test_exec_fwcompat_cmd_matches()
{
	State s = {};
	TEST_ASSERT_TRUE(executeFwCompatCmd("fwcompat", s));
}

/** @brief Verifies non-matching command string is rejected */
void test_exec_fwcompat_cmd_no_match()
{
	State s = {};
	TEST_ASSERT_FALSE(executeFwCompatCmd("vehicle", s));
}

/** @brief Verifies fwCompatName returns correct string for each enum value */
void test_compat_name()
{
	TEST_ASSERT_EQUAL_STRING("OK", fwCompatName(FW_COMPAT_OK));
	TEST_ASSERT_EQUAL_STRING("WARN", fwCompatName(FW_COMPAT_WARN));
	TEST_ASSERT_EQUAL_STRING("FAIL", fwCompatName(FW_COMPAT_FAIL));
	TEST_ASSERT_EQUAL_STRING("UNKNOWN", fwCompatName(FW_COMPAT_UNKNOWN));
}

int main()
{
	UNITY_BEGIN();
	RUN_TEST(test_decode_mux0_sets_year_release_minor);
	RUN_TEST(test_decode_mux1_sets_build);
	RUN_TEST(test_compat_ok_for_2024_plus);
	RUN_TEST(test_compat_warn_for_older);
	RUN_TEST(test_exec_fwcompat_cmd_matches);
	RUN_TEST(test_exec_fwcompat_cmd_no_match);
	RUN_TEST(test_compat_name);
	return UNITY_END();
}
