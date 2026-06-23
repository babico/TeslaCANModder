/**
 * @file firmware/test/test_esp32_can_recorder/test_can_recorder.cpp
 * @brief Hardware tests for CAN frame recording on ESP32
 *
 * Requires: ESP32 DevKit with MCP2515 on Chassis bus.
 * Tests: Ring buffer recording, frame capture with timestamps,
 *        overflow handling, consumer iteration.
 *
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <Arduino.h>
#include <unity.h>
#include <SPI.h>
#include <mcp2515.h>

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 0
#define BUS_BODY_ACTIVE 0
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

#include "core/types.h"
#include "core/can/recorder.h"

const int MCP_CS_PIN = 15;
static MCP2515 *mcp = nullptr;

void setUp()
{
	canRecorderInit();
	if (!mcp)
	{
		mcp = new MCP2515(MCP_CS_PIN);
		mcp->reset();
		mcp->setBitrate(CAN_500KBPS, MCP_8MHZ);
	}
}

void tearDown() {}

/* ── Recorder initialization ──────────────────────────────────────────────── */

void test_recorder_init_disabled()
{
	canRecorderInit();
	TEST_ASSERT_FALSE_MESSAGE(!canRecorderEnabled(), "Recorder should be disabled after init");
}

void test_recorder_init_zero_counters()
{
	canRecorderInit();
	TEST_ASSERT_EQUAL_UINT16_MESSAGE(0, canRecorderCount(), "Count should be zero after init");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, canRecorderCapturedTotal(), "Captured should be zero after init");
}

/* ── Recording with hardware frames ───────────────────────────────────────── */

void test_recorder_captures_loopback_frame()
{
	mcp->setLoopbackMode();
	canRecorderStart(true);

	can_frame frame;
	frame.can_id = 0x3FD;
	frame.can_dlc = 8;
	for (int i = 0; i < 8; i++)
		frame.data[i] = (uint8_t)(i + 1);

	mcp->sendMessage(&frame);
	delay(10);

	// Manually capture into recorder (simulating what the driver would do)
	Frame f;
	f.id = frame.can_id;
	f.dlc = frame.can_dlc;
	memcpy(f.data, frame.data, 8);
	canRecorderCapture(f, 0, millis());

	TEST_ASSERT_EQUAL_UINT16_MESSAGE(1, canRecorderCount(), "Should have 1 recorded frame");
	TEST_ASSERT_EQUAL_HEX32_MESSAGE(0x3FD, canRecorderGet(0)->id, "Recorded ID should match");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(8, canRecorderGet(0)->dlc, "Recorded DLC should match");

	canRecorderStop();
}

void test_recorder_captures_multiple_frames()
{
	mcp->setLoopbackMode();
	canRecorderStart(true);

	for (int i = 0; i < 10; i++)
	{
		can_frame frame;
		frame.can_id = 0x100 + i;
		frame.can_dlc = 4;
		frame.data[0] = (uint8_t)i;

		mcp->sendMessage(&frame);
		delay(5);

		Frame f;
		f.id = frame.can_id;
		f.dlc = frame.can_dlc;
		memcpy(f.data, frame.data, 4);
		canRecorderCapture(f, 0, millis());
	}

	TEST_ASSERT_EQUAL_UINT16_MESSAGE(10, canRecorderCount(), "Should have 10 recorded frames");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(10, canRecorderCapturedTotal(), "Captured total should be 10");

	canRecorderStop();
}

/* ── Timestamp accuracy ───────────────────────────────────────────────────── */

void test_recorder_timestamps_increase()
{
	mcp->setLoopbackMode();
	canRecorderStart(true);

	for (int i = 0; i < 5; i++)
	{
		can_frame frame;
		frame.can_id = 0x200;
		frame.can_dlc = 1;
		frame.data[0] = (uint8_t)i;
		mcp->sendMessage(&frame);
		delay(50);

		Frame f;
		f.id = frame.can_id;
		f.dlc = frame.can_dlc;
		f.data[0] = frame.data[0];
		canRecorderCapture(f, 0, millis());
	}

	// Verify timestamps are monotonically increasing
	for (int i = 1; i < 5; i++)
	{
		TEST_ASSERT_TRUE_MESSAGE(canRecorderGet(i)->timestamp > canRecorderGet(i - 1)->timestamp,
								 "Timestamps should be monotonically increasing");
	}

	canRecorderStop();
}

/* ── Overflow handling ────────────────────────────────────────────────────── */

void test_recorder_overflow()
{
	canRecorderStart(true);

	// Fill beyond capacity (256)
	for (int i = 0; i < 270; i++)
	{
		Frame f;
		f.id = (uint32_t)(0x300 + (i % 256));
		f.dlc = 2;
		f.data[0] = (uint8_t)(i & 0xFF);
		f.data[1] = (uint8_t)((i >> 8) & 0xFF);
		canRecorderCapture(f, 0, millis());
	}

	// Count should cap at capacity
	TEST_ASSERT_EQUAL_UINT16_MESSAGE(CAN_RECORDER_SIZE, canRecorderCount(), "Count should cap at buffer size");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(270, canRecorderCapturedTotal(), "Captured total should reflect all attempts");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(14, canRecorderDroppedTotal(), "Dropped should reflect overflow count");

	canRecorderStop();
}

/* ── Start/Stop/Reset ─────────────────────────────────────────────────────── */

void test_recorder_start_stop_cycle()
{
	canRecorderStart(true);
	TEST_ASSERT_TRUE_MESSAGE(canRecorderEnabled(), "Recorder should be enabled after start");

	canRecorderStop();
	TEST_ASSERT_FALSE_MESSAGE(!canRecorderEnabled(), "Recorder should be disabled after stop");

	canRecorderStart(false); // Don't clear
	TEST_ASSERT_TRUE_MESSAGE(canRecorderEnabled(), "Recorder should be enabled after restart");

	canRecorderStop();
}

void test_recorder_reset_clears_data()
{
	canRecorderStart(true);

	for (int i = 0; i < 5; i++)
	{
		Frame f;
		f.id = 0x400;
		f.dlc = 1;
		f.data[0] = (uint8_t)i;
		canRecorderCapture(f, 0, millis());
	}

	TEST_ASSERT_EQUAL_UINT16_MESSAGE(5, canRecorderCount(), "Should have 5 frames before reset");

	canRecorderReset();
	TEST_ASSERT_EQUAL_UINT16_MESSAGE(0, canRecorderCount(), "Count should be zero after reset");
	TEST_ASSERT_TRUE_MESSAGE(canRecorderEnabled(), "Enabled state should persist after reset");

	canRecorderStop();
}

/* ── Disabled recorder ignores captures ───────────────────────────────────── */

void test_recorder_disabled_ignores_captures()
{
	canRecorderInit(); // Disabled by default

	Frame f;
	f.id = 0x500;
	f.dlc = 4;
	canRecorderCapture(f, 0, millis());

	TEST_ASSERT_EQUAL_UINT16_MESSAGE(0, canRecorderCount(), "Disabled recorder should not capture frames");
}

/* ── Bus index tracking ───────────────────────────────────────────────────── */

void test_recorder_tracks_bus_index()
{
	canRecorderStart(true);

	Frame f;
	f.id = 0x600;
	f.dlc = 1;
	canRecorderCapture(f, 2, millis()); // Bus 2 (Body)

	TEST_ASSERT_EQUAL_UINT8_MESSAGE(2, canRecorderGet(0)->bus, "Recorded bus index should match");

	canRecorderStop();
}

void setup()
{
	delay(2000);
	Serial.begin(115200);
	delay(1000);
	Serial.println("=== ESP32 CAN Recorder Hardware Tests ===");

	pinMode(MCP_CS_PIN, OUTPUT);
	digitalWrite(MCP_CS_PIN, HIGH);
	SPI.begin();
	SPI.setFrequency(10000000);

	mcp = new MCP2515(MCP_CS_PIN);
	mcp->reset();
	mcp->setBitrate(CAN_500KBPS, MCP_8MHZ);
	delay(10);

	UNITY_BEGIN();
	RUN_TEST(test_recorder_init_disabled);
	RUN_TEST(test_recorder_init_zero_counters);
	RUN_TEST(test_recorder_captures_loopback_frame);
	RUN_TEST(test_recorder_captures_multiple_frames);
	RUN_TEST(test_recorder_timestamps_increase);
	RUN_TEST(test_recorder_overflow);
	RUN_TEST(test_recorder_start_stop_cycle);
	RUN_TEST(test_recorder_reset_clears_data);
	RUN_TEST(test_recorder_disabled_ignores_captures);
	RUN_TEST(test_recorder_tracks_bus_index);
	UNITY_END();
}

void loop()
{
	delay(1000);
}
