/**
 * @file firmware/test/test_native_recorder/test_recorder.cpp
 * @brief Unit tests for CAN frame recorder circular buffer
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
#include "transport/can/recorder.h"

void setUp()
{
	canRecorderInit();
}
void tearDown() {}

/* ── canRecorderInit ─────────────────────────────────────────────────────── */

void test_recorder_init_disabled()
{
	TEST_ASSERT_FALSE(canRecorderEnabled());
}

void test_recorder_init_zero_counters()
{
	TEST_ASSERT_EQUAL_UINT16(0, canRecorderCount());
	TEST_ASSERT_EQUAL_UINT32(0, canRecorderCapturedTotal());
	TEST_ASSERT_EQUAL_UINT32(0, canRecorderDroppedTotal());
}

/* ── canRecorderStart / Stop ─────────────────────────────────────────────── */

void test_recorder_start_enables()
{
	canRecorderStart();
	TEST_ASSERT_TRUE(canRecorderEnabled());
}

void test_recorder_stop_disables()
{
	canRecorderStart();
	canRecorderStop();
	TEST_ASSERT_FALSE(canRecorderEnabled());
}

void test_recorder_start_with_clear_resets()
{
	canRecorderStart(false);
	canRecorder.captured = 10;
	canRecorderStart(true);
	TEST_ASSERT_EQUAL_UINT32(0, canRecorderCapturedTotal());
}

void test_recorder_start_without_clear_preserves()
{
	canRecorderStart(false);
	canRecorder.captured = 10;
	canRecorderStart(false);
	TEST_ASSERT_EQUAL_UINT32(10, canRecorderCapturedTotal());
}

/* ── canRecorderCapture ──────────────────────────────────────────────────── */

void test_recorder_capture_when_disabled_ignored()
{
	Frame f = {};
	f.id = 0x100;
	f.dlc = 8;
	canRecorderCapture(f, 0, 1000);
	TEST_ASSERT_EQUAL_UINT16(0, canRecorderCount());
}

void test_recorder_capture_stores_frame()
{
	canRecorderStart();
	Frame f = {};
	f.id = 0x3FD;
	f.dlc = 8;
	f.data[0] = 0xAB;
	canRecorderCapture(f, 1, 5000);
	TEST_ASSERT_EQUAL_UINT16(1, canRecorderCount());
	TEST_ASSERT_EQUAL_HEX32(0x3FD, canRecorderGet(0)->id);
	TEST_ASSERT_EQUAL_HEX8(0xAB, canRecorderGet(0)->data[0]);
	TEST_ASSERT_EQUAL_UINT8(1, canRecorderGet(0)->bus);
	TEST_ASSERT_EQUAL(5000, canRecorderLastCaptureMs());
}

void test_recorder_capture_increments_counters()
{
	canRecorderStart();
	Frame f = {};
	f.dlc = 8;
	canRecorderCapture(f, 0, 1000);
	canRecorderCapture(f, 0, 1001);
	TEST_ASSERT_EQUAL_UINT16(2, canRecorderCount());
	TEST_ASSERT_EQUAL_UINT32(2, canRecorderCapturedTotal());
}

/* ── Wrap-around ─────────────────────────────────────────────────────────── */

void test_recorder_wraparound()
{
	canRecorderStart();
	Frame f = {};
	f.dlc = 8;
	for (int i = 0; i < CAN_RECORDER_SIZE + 10; i++)
	{
		f.id = (uint32_t)i;
		canRecorderCapture(f, 0, 1000 + i);
	}
	TEST_ASSERT_EQUAL_UINT16(CAN_RECORDER_SIZE, canRecorderCount());
	TEST_ASSERT_EQUAL_UINT32(10, canRecorderDroppedTotal());
}

void test_recorder_get_oldest_first_after_wrap()
{
	canRecorderStart();
	Frame f = {};
	f.dlc = 8;
	for (int i = 0; i < CAN_RECORDER_SIZE + 5; i++)
	{
		f.id = (uint32_t)(100 + i);
		canRecorderCapture(f, 0, 1000 + i);
	}
	const CanRecorderEntry *e = canRecorderGet(0);
	TEST_ASSERT_EQUAL_HEX32(105, e->id);
}

/* ── canRecorderGet bounds ───────────────────────────────────────────────── */

void test_recorder_get_returns_null_for_invalid_index()
{
	canRecorderStart();
	Frame f = {};
	f.dlc = 8;
	canRecorderCapture(f, 0, 1000);
	TEST_ASSERT_NULL(canRecorderGet(1));
}

void test_recorder_get_returns_null_when_empty()
{
	TEST_ASSERT_NULL(canRecorderGet(0));
}

/* ── Capacity ────────────────────────────────────────────────────────────── */

void test_recorder_capacity_is_256()
{
	TEST_ASSERT_EQUAL_UINT16(256, canRecorderCapacity());
}

/* ── canRecorderReset ────────────────────────────────────────────────────── */

void test_recorder_reset_clears_counters()
{
	canRecorderStart();
	Frame f = {};
	f.dlc = 8;
	canRecorderCapture(f, 0, 1000);
	canRecorderReset();
	TEST_ASSERT_EQUAL_UINT16(0, canRecorderCount());
	TEST_ASSERT_EQUAL_UINT32(0, canRecorderCapturedTotal());
	TEST_ASSERT_EQUAL_UINT32(0, canRecorderDroppedTotal());
}

void test_recorder_reset_does_not_change_enabled()
{
	canRecorderStart();
	canRecorderReset();
	TEST_ASSERT_TRUE(canRecorderEnabled());
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_recorder_init_disabled);
	RUN_TEST(test_recorder_init_zero_counters);
	RUN_TEST(test_recorder_start_enables);
	RUN_TEST(test_recorder_stop_disables);
	RUN_TEST(test_recorder_start_with_clear_resets);
	RUN_TEST(test_recorder_start_without_clear_preserves);
	RUN_TEST(test_recorder_capture_when_disabled_ignored);
	RUN_TEST(test_recorder_capture_stores_frame);
	RUN_TEST(test_recorder_capture_increments_counters);
	RUN_TEST(test_recorder_wraparound);
	RUN_TEST(test_recorder_get_oldest_first_after_wrap);
	RUN_TEST(test_recorder_get_returns_null_for_invalid_index);
	RUN_TEST(test_recorder_get_returns_null_when_empty);
	RUN_TEST(test_recorder_capacity_is_256);
	RUN_TEST(test_recorder_reset_clears_counters);
	RUN_TEST(test_recorder_reset_does_not_change_enabled);

	return UNITY_END();
}
