/**
 * @file firmware/test/test_native_can_recorder/test_can_recorder.cpp
 * @brief Unit tests for CAN frame recorder ring buffer
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
#include "core/can/recorder.h"

/** @brief Reset test state before each test */
void setUp() {}

/** @brief Cleanup after each test */
void tearDown() {}

/** @brief Build a test frame with a given ID, DLC, and sequential payload starting at seed */
static Frame makeFrame(uint32_t id, uint8_t dlc, uint8_t seed)
{
	Frame f;
	f.id = id;
	f.dlc = dlc;
	for (uint8_t i = 0; i < 8; i++)
	{
		f.data[i] = seed + i;
	}
	return f;
}

/** @brief Recorder starts disabled, can be started and stopped, and reports zero count initially */
void test_canRecorder_start_stop()
{
	canRecorderInit();
	TEST_ASSERT_FALSE(canRecorderEnabled());

	canRecorderStart(true);
	TEST_ASSERT_TRUE(canRecorderEnabled());
	TEST_ASSERT_EQUAL_UINT16(0, canRecorderCount());

	canRecorderStop();
	TEST_ASSERT_FALSE(canRecorderEnabled());
}

/** @brief A captured frame is stored with correct ID, bus, DLC, data, and timestamp */
void test_canRecorder_capture_entry()
{
	canRecorderInit();
	canRecorderStart(true);

	Frame f = makeFrame(0x123, 4, 10);
	canRecorderCapture(f, 1, 2500);

	TEST_ASSERT_EQUAL_UINT16(1, canRecorderCount());
	TEST_ASSERT_EQUAL_UINT32(1, canRecorderCapturedTotal());
	TEST_ASSERT_EQUAL_UINT32(0, canRecorderDroppedTotal());
	TEST_ASSERT_EQUAL_UINT32(2500, canRecorderLastCaptureMs());

	const CanRecorderEntry *e = canRecorderGet(0);
	TEST_ASSERT_NOT_NULL(e);
	TEST_ASSERT_EQUAL_UINT32(0x123, e->id);
	TEST_ASSERT_EQUAL_UINT8(1, e->bus);
	TEST_ASSERT_EQUAL_UINT8(4, e->dlc);
	TEST_ASSERT_EQUAL_UINT8(10, e->data[0]);
	TEST_ASSERT_EQUAL_UINT8(13, e->data[3]);
	// Only DLC bytes are copied; remaining slots stay zero
	TEST_ASSERT_EQUAL_UINT8(0, e->data[4]);
}

/** @brief Overflow wraps the ring buffer, dropping oldest entries and tracking drop count */
void test_canRecorder_overflow_drops_oldest()
{
	canRecorderInit();
	canRecorderStart(true);

	for (uint16_t i = 0; i < CAN_RECORDER_SIZE + 5; i++)
	{
		Frame f = makeFrame(0x600 + i, 2, (uint8_t)i);
		canRecorderCapture(f, 0, 1000 + i);
	}

	TEST_ASSERT_EQUAL_UINT16(CAN_RECORDER_SIZE, canRecorderCount());
	TEST_ASSERT_EQUAL_UINT32(CAN_RECORDER_SIZE + 5, canRecorderCapturedTotal());
	TEST_ASSERT_EQUAL_UINT32(5, canRecorderDroppedTotal());

	// Oldest surviving entry should be the 6th captured frame (index 5)
	const CanRecorderEntry *oldest = canRecorderGet(0);
	TEST_ASSERT_NOT_NULL(oldest);
	TEST_ASSERT_EQUAL_UINT32(0x605, oldest->id);
	TEST_ASSERT_EQUAL_UINT32(1005, oldest->timestamp);
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_canRecorder_start_stop);
	RUN_TEST(test_canRecorder_capture_entry);
	RUN_TEST(test_canRecorder_overflow_drops_oldest);
	return UNITY_END();
}
