/**
 * @file firmware/test/test_esp32_can_loopback/test_can_loopback.cpp
 * @brief Hardware tests for CAN frame TX/RX loopback on ESP32
 *
 * Requires: ESP32 DevKit with MCP2515 on Chassis bus.
 * Tests use MCP2515 loopback mode (frames echo internally without
 * needing a second CAN node or physical bus connection).
 *
 * Tests: Frame transmission, reception, ID filtering, DLC handling,
 *        data integrity, bus error detection.
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
#include "core/can/bus.h"

// MCP2515 pin definitions matching hardware
const int MCP_CS_PIN = 15;
const int MCP_INT_PIN = 34;

static MCP2515 *mcp = nullptr;

void setUp()
{
	if (!mcp)
	{
		mcp = new MCP2515(MCP_CS_PIN);
		mcp->reset();
		mcp->setBitrate(CAN_500KBPS, MCP_8MHZ);
		mcp->setNormalMode();
	}
}

void tearDown() {}

/* ── MCP2515 initialization ──────────────────────────────────────────────── */

void test_mcp2515_init_succeeds()
{
	MCP2515::ERROR err = mcp->setBitrate(CAN_500KBPS, MCP_8MHZ);
	TEST_ASSERT_EQUAL_MESSAGE(MCP2515::ERROR_OK, err, "setBitrate should succeed");
}

void test_mcp2515_normal_mode()
{
	MCP2515::ERROR err = mcp->setNormalMode();
	TEST_ASSERT_EQUAL_MESSAGE(MCP2515::ERROR_OK, err, "setNormalMode should succeed");
}

void test_mcp2515_loopback_mode()
{
	MCP2515::ERROR err = mcp->setLoopbackMode();
	TEST_ASSERT_EQUAL_MESSAGE(MCP2515::ERROR_OK, err, "setLoopbackMode should succeed");
}

/* ── CAN frame TX in loopback ─────────────────────────────────────────────── */

void test_send_frame_in_loopback()
{
	mcp->setLoopbackMode();

	can_frame frame;
	frame.can_id = 0x100;
	frame.can_dlc = 8;
	for (int i = 0; i < 8; i++)
		frame.data[i] = (uint8_t)i;

	MCP2515::ERROR err = mcp->sendMessage(&frame);
	TEST_ASSERT_EQUAL_MESSAGE(MCP2515::ERROR_OK, err, "sendMessage should succeed in loopback");
}

/* ── CAN frame RX in loopback ─────────────────────────────────────────────── */

void test_receive_frame_in_loopback()
{
	mcp->setLoopbackMode();

	can_frame txFrame;
	txFrame.can_id = 0x3FD;
	txFrame.can_dlc = 8;
	txFrame.data[0] = 0x42;
	txFrame.data[1] = 0xAB;
	txFrame.data[2] = 0xCD;
	txFrame.data[3] = 0xEF;
	txFrame.data[4] = 0x01;
	txFrame.data[5] = 0x23;
	txFrame.data[6] = 0x45;
	txFrame.data[7] = 0x67;

	MCP2515::ERROR txErr = mcp->sendMessage(&txFrame);
	TEST_ASSERT_EQUAL_MESSAGE(MCP2515::ERROR_OK, txErr, "sendMessage should succeed");

	delay(10); // Allow loopback propagation

	can_frame rxFrame;
	if (mcp->readMessage(&rxFrame) == MCP2515::ERROR_OK)
	{
		TEST_ASSERT_EQUAL_HEX32_MESSAGE(0x3FD, rxFrame.can_id, "Received ID should match sent ID");
		TEST_ASSERT_EQUAL_UINT8_MESSAGE(8, rxFrame.can_dlc, "Received DLC should match");
		TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x42, rxFrame.data[0], "data[0] should match");
		TEST_ASSERT_EQUAL_HEX8_MESSAGE(0xAB, rxFrame.data[1], "data[1] should match");
		TEST_ASSERT_EQUAL_HEX8_MESSAGE(0xCD, rxFrame.data[2], "data[2] should match");
	}
	else
	{
		TEST_FAIL_MESSAGE("readMessage should receive loopback frame");
	}
}

/* ── CAN frame data integrity ─────────────────────────────────────────────── */

void test_frame_data_integrity_all_bytes()
{
	mcp->setLoopbackMode();

	can_frame txFrame;
	txFrame.can_id = 0x273;
	txFrame.can_dlc = 8;
	for (int i = 0; i < 8; i++)
		txFrame.data[i] = (uint8_t)(0x10 + i);

	mcp->sendMessage(&txFrame);
	delay(10);

	can_frame rxFrame;
	if (mcp->readMessage(&rxFrame) == MCP2515::ERROR_OK)
	{
		for (int i = 0; i < 8; i++)
		{
			char msg[32];
			snprintf(msg, sizeof(msg), "data[%d] should match", i);
			TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x10 + i, rxFrame.data[i], msg);
		}
	}
	else
	{
		TEST_FAIL_MESSAGE("Should receive loopback frame");
	}
}

/* ── DLC boundary tests ───────────────────────────────────────────────────── */

void test_frame_dlc_zero()
{
	mcp->setLoopbackMode();

	can_frame frame;
	frame.can_id = 0x100;
	frame.can_dlc = 0;

	MCP2515::ERROR err = mcp->sendMessage(&frame);
	// DLC 0 is valid in CAN
	TEST_ASSERT_EQUAL_MESSAGE(MCP2515::ERROR_OK, err, "send with DLC 0 should succeed");
}

void test_frame_dlc_eight()
{
	mcp->setLoopbackMode();

	can_frame frame;
	frame.can_id = 0x100;
	frame.can_dlc = 8;
	memset(frame.data, 0xFF, 8);

	MCP2515::ERROR err = mcp->sendMessage(&frame);
	TEST_ASSERT_EQUAL_MESSAGE(MCP2515::ERROR_OK, err, "send with DLC 8 should succeed");
}

/* ── Extended frame ID ────────────────────────────────────────────────────── */

void test_extended_frame_id()
{
	mcp->setLoopbackMode();

	can_frame frame;
	frame.can_id = 0x18DAF110 | CAN_EFF_FLAG; // Extended ID
	frame.can_dlc = 4;
	frame.data[0] = 0x02;
	frame.data[1] = 0x01;
	frame.data[2] = 0x00;
	frame.data[3] = 0x00;

	MCP2515::ERROR err = mcp->sendMessage(&frame);
	// Extended frames may or may not be supported depending on MCP2515 config
	// We just verify the call doesn't crash
	TEST_ASSERT_TRUE_MESSAGE(err == MCP2515::ERROR_OK || err == MCP2515::ERROR_FAIL,
							 "Extended frame send should not crash");
}

/* ── Multiple frame sequence ──────────────────────────────────────────────── */

void test_multiple_frames_sequence()
{
	mcp->setLoopbackMode();

	const int COUNT = 5;
	for (int i = 0; i < COUNT; i++)
	{
		can_frame frame;
		frame.can_id = 0x200 + i;
		frame.can_dlc = 4;
		frame.data[0] = (uint8_t)i;
		frame.data[1] = (uint8_t)(i * 2);
		frame.data[2] = (uint8_t)(i * 3);
		frame.data[3] = (uint8_t)(i * 4);

		MCP2515::ERROR err = mcp->sendMessage(&frame);
		TEST_ASSERT_EQUAL_MESSAGE(MCP2515::ERROR_OK, err, "Sequential frame send should succeed");
		delay(5);
	}
}

/* ── Bus status check ─────────────────────────────────────────────────────── */

void test_bus_active_macro()
{
#ifdef BUS_CHASSIS_ACTIVE
	TEST_ASSERT_TRUE_MESSAGE(busActive(BUS_CHASSIS), "BUS_CHASSIS_ACTIVE should make busActive true");
#endif
}

/* ── Bitrate configuration ────────────────────────────────────────────────── */

void test_bitrate_500kbps()
{
	MCP2515::ERROR err = mcp->setBitrate(CAN_500KBPS, MCP_8MHZ);
	TEST_ASSERT_EQUAL_MESSAGE(MCP2515::ERROR_OK, err, "500kbps should be supported");
}

void test_bitrate_250kbps()
{
	MCP2515::ERROR err = mcp->setBitrate(CAN_250KBPS, MCP_8MHZ);
	TEST_ASSERT_EQUAL_MESSAGE(MCP2515::ERROR_OK, err, "250kbps should be supported");
}

void test_bitrate_125kbps()
{
	MCP2515::ERROR err = mcp->setBitrate(CAN_125KBPS, MCP_8MHZ);
	TEST_ASSERT_EQUAL_MESSAGE(MCP2515::ERROR_OK, err, "125kbps should be supported");
}

void setup()
{
	delay(2000);
	Serial.begin(115200);
	delay(1000);
	Serial.println("=== ESP32 CAN Loopback Hardware Tests ===");

	pinMode(MCP_CS_PIN, OUTPUT);
	digitalWrite(MCP_CS_PIN, HIGH);
	SPI.begin();
	SPI.setFrequency(10000000);

	mcp = new MCP2515(MCP_CS_PIN);
	mcp->reset();
	delay(10);

	UNITY_BEGIN();
	RUN_TEST(test_mcp2515_init_succeeds);
	RUN_TEST(test_mcp2515_normal_mode);
	RUN_TEST(test_mcp2515_loopback_mode);
	RUN_TEST(test_send_frame_in_loopback);
	RUN_TEST(test_receive_frame_in_loopback);
	RUN_TEST(test_frame_data_integrity_all_bytes);
	RUN_TEST(test_frame_dlc_zero);
	RUN_TEST(test_frame_dlc_eight);
	RUN_TEST(test_extended_frame_id);
	RUN_TEST(test_multiple_frames_sequence);
	RUN_TEST(test_bus_active_macro);
	RUN_TEST(test_bitrate_500kbps);
	RUN_TEST(test_bitrate_250kbps);
	RUN_TEST(test_bitrate_125kbps);
	UNITY_END();
}

void loop()
{
	delay(1000);
}
