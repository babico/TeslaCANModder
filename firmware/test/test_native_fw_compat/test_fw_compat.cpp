// ── Firmware Version Compatibility Tests ────────────────────────────────────
// Tests CAN 0x392 decode and compat evaluation.

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
#include "infra/can.h"

// ── Stubs ────────────────────────────────────────────────────────────────────
void saveSettings(const State &) {}
void sendLog(const char *) {}
void sendLog(const __FlashStringHelper *) {}

#include "feature/fw_compat.h"

void setUp() {}
void tearDown() {}

// ── Tests ───────────────────────────────────────────────────────────────────

void test_decode_mux0_sets_year_release_minor()
{
	State s = {};
	Frame f = {};
	f.id = 0x392;
	f.dlc = 8;
	// mux = 0 (byte 0 & 0x07)
	f.data[0] = 0x00;
	// year 2024 = 0x07E8 big-endian
	f.data[1] = 0x07;
	f.data[2] = 0xE8;
	f.data[3] = 12; // release
	f.data[4] = 5;	// minor
	decodeFwVersion(f, s);
	TEST_ASSERT_EQUAL(2024, s.fwYear);
	TEST_ASSERT_EQUAL(12, s.fwRelease);
	TEST_ASSERT_EQUAL(5, s.fwMinor);
	TEST_ASSERT_TRUE(s.hasFwVersion);
}

void test_decode_mux1_sets_build()
{
	State s = {};
	Frame f = {};
	f.id = 0x392;
	f.dlc = 8;
	// mux = 1
	f.data[0] = 0x01;
	// build 0x1234 big-endian uint32
	f.data[1] = 0x00;
	f.data[2] = 0x00;
	f.data[3] = 0x12;
	f.data[4] = 0x34;
	decodeFwVersion(f, s);
	TEST_ASSERT_EQUAL(0x1234, s.fwBuild);
}

void test_compat_ok_for_2024_plus()
{
	State s = {};
	s.variant = HW4;
	Frame f = {};
	f.id = 0x392;
	f.dlc = 8;
	f.data[0] = 0x00;
	// year 2026 = 0x07EA
	f.data[1] = 0x07;
	f.data[2] = 0xEA;
	f.data[3] = 2; // release
	f.data[4] = 9; // minor (FSD v14 path)
	decodeFwVersion(f, s);
	TEST_ASSERT_EQUAL(FW_COMPAT_OK, s.fwCompat);
}

void test_compat_warn_for_older()
{
	State s = {};
	s.variant = HW4;
	Frame f = {};
	f.id = 0x392;
	f.dlc = 8;
	f.data[0] = 0x00;
	// year 2023 = 0x07E7 (older than 2026 → WARN)
	f.data[1] = 0x07;
	f.data[2] = 0xE7;
	f.data[3] = 40;
	f.data[4] = 1;
	decodeFwVersion(f, s);
	TEST_ASSERT_EQUAL(FW_COMPAT_WARN, s.fwCompat);
}

void test_exec_fwcompat_cmd_matches()
{
	State s = {};
	TEST_ASSERT_TRUE(execFwCompatCmd("fwcompat", s));
}

void test_exec_fwcompat_cmd_no_match()
{
	State s = {};
	TEST_ASSERT_FALSE(execFwCompatCmd("vehicle", s));
}

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
