/**
 * @file firmware/test/test_native_drive_mode/test_drive_mode.cpp
 * @brief Unit tests for drive mode override commands and tick logic
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <unity.h>
#include <cstring>
#include <cstdio>

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

static int saveCount = 0;
void saveSettings(const State &)
{
	saveCount++;
}
void sendLog(const char *) {}
void sendLog(const __FlashStringHelper *) {}
void resetHandlerLogFlags() {}
void applyFilters(State &) {}

struct SendCall
{
	Frame f;
	uint8_t bus;
};
static SendCall stub_sends[16];
static uint8_t stub_send_count = 0;

void driverSend(const Frame &f, uint8_t bus)
{
	if (stub_send_count < 16)
	{
		stub_sends[stub_send_count].f = f;
		stub_sends[stub_send_count].bus = bus;
		stub_send_count++;
	}
}

#include "feature/drive_mode.h"

/** @brief Creates a default HW4 State for drive mode tests */
static State makeState()
{
	State s = {};
	s.variant = HW4;
	return s;
}

/** @brief Resets save counter and send stub before each test */
void setUp()
{
	saveCount = 0;
	stub_send_count = 0;
}

/** @brief Test fixture teardown — no cleanup required */
void tearDown() {}

/** @brief Verifies drivemode:off sets override to NONE and persists */
void test_drivemode_off()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeDriveModeCmd("drivemode:off", s));
	TEST_ASSERT_EQUAL(DRIVE_MODE_NONE, s.driveModeOverride);
	TEST_ASSERT_EQUAL(1, saveCount);
}

/** @brief Verifies drivemode:chill sets override to CHILL and persists */
void test_drivemode_chill()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeDriveModeCmd("drivemode:chill", s));
	TEST_ASSERT_EQUAL(DRIVE_MODE_CHILL, s.driveModeOverride);
	TEST_ASSERT_EQUAL(1, saveCount);
}

/** @brief Verifies drivemode:standard sets override to STANDARD and persists */
void test_drivemode_standard()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeDriveModeCmd("drivemode:standard", s));
	TEST_ASSERT_EQUAL(DRIVE_MODE_STANDARD, s.driveModeOverride);
	TEST_ASSERT_EQUAL(1, saveCount);
}

/** @brief Verifies drivemode:performance sets override to PERFORMANCE and persists */
void test_drivemode_performance()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeDriveModeCmd("drivemode:performance", s));
	TEST_ASSERT_EQUAL(DRIVE_MODE_PERFORMANCE, s.driveModeOverride);
	TEST_ASSERT_EQUAL(1, saveCount);
}

/** @brief Verifies unknown drive mode argument returns false without saving */
void test_drivemode_unknown_returns_false()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeDriveModeCmd("drivemode:turbo", s));
	TEST_ASSERT_EQUAL(0, saveCount);
}

/** @brief Verifies wrong command prefix returns false */
void test_drivemode_wrong_prefix_returns_false()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeDriveModeCmd("fsd:on", s));
}

/** @brief Verifies chill mode frame uses correct CAN ID and mode bits */
void test_build_frame_chill()
{
	uint8_t lastDrive[8] = {};
	Frame f = buildDriveModeFrame(DRIVE_MODE_CHILL, lastDrive);
	TEST_ASSERT_EQUAL_UINT32(CAN_ID_DRIVE_MODE, f.id);
	TEST_ASSERT_EQUAL_UINT8(8, f.dlc);
	TEST_ASSERT_EQUAL_UINT8(0x00, f.data[0] & 0x60);
}

/** @brief Verifies performance mode frame sets the correct mode bits */
void test_build_frame_performance()
{
	uint8_t lastDrive[8] = {};
	Frame f = buildDriveModeFrame(DRIVE_MODE_PERFORMANCE, lastDrive);
	TEST_ASSERT_EQUAL_UINT32(CAN_ID_DRIVE_MODE, f.id);
	TEST_ASSERT_EQUAL_UINT8(0x40, f.data[0] & 0x60);
}

/** @brief Verifies tick does not send when override is NONE */
void test_tick_off_no_send()
{
	State s = makeState();
	s.driveModeOverride = DRIVE_MODE_NONE;
	driveModeTick(s, 1000);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

/** @brief Verifies tick sends a drive mode frame when override is active */
void test_tick_active_sends_frame()
{
	State s = makeState();
	s.hasDrive = true;
	s.driveModeOverride = DRIVE_MODE_CHILL;
	s.driveModeLastMs = 0;
	driveModeTick(s, 100);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL_UINT32(CAN_ID_DRIVE_MODE, stub_sends[0].f.id);
}

/** @brief Verifies tick respects the 100ms send interval */
void test_tick_respects_interval()
{
	State s = makeState();
	s.hasDrive = true;
	s.driveModeOverride = DRIVE_MODE_STANDARD;
	s.driveModeLastMs = 0;
	driveModeTick(s, 100);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	driveModeTick(s, 120);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	driveModeTick(s, 200);
	TEST_ASSERT_EQUAL(2, stub_send_count);
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_drivemode_off);
	RUN_TEST(test_drivemode_chill);
	RUN_TEST(test_drivemode_standard);
	RUN_TEST(test_drivemode_performance);
	RUN_TEST(test_drivemode_unknown_returns_false);
	RUN_TEST(test_drivemode_wrong_prefix_returns_false);
	RUN_TEST(test_build_frame_chill);
	RUN_TEST(test_build_frame_performance);
	RUN_TEST(test_tick_off_no_send);
	RUN_TEST(test_tick_active_sends_frame);
	RUN_TEST(test_tick_respects_interval);
	return UNITY_END();
}
