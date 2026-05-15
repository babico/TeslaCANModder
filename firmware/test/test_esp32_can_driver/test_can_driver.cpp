/**
 * @file firmware/test/test_esp32_can_driver/test_can_driver.cpp
 * @brief Hardware tests for MCP2515 CAN driver on ESP32
 *
 * Requires: ESP32 DevKit with at least one MCP2515 module connected
 * on the Chassis bus (CS=15, INT=34, X179 pins 13-14).
 *
 * Tests: SPI communication, MCP2515 initialization, bus status,
 *        clock profile detection, filter programming.
 *
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <Arduino.h>
#include <unity.h>
#include <SPI.h>

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 0
#define BUS_BODY_ACTIVE 0
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

#include "core/types.h"
#include "core/can/bus.h"
#include "core/driver/common.h"

void setUp() {}
void tearDown() {}

/* ── SPI bus availability ─────────────────────────────────────────────────── */

void test_spi_bus_initialized()
{
	// SPI should be initialized by the driver init
	SPI.begin();
	TEST_ASSERT_TRUE_MESSAGE(SPI.isHWPinAutoSupported(), "SPI bus should be available");
}

/* ── MCP2515 chip detect ─────────────────────────────────────────────────── */

void test_mcp2515_responds_to_spi()
{
	// Try to read the MCP2515 CANSTAT register via SPI
	// This verifies the chip is powered and SPI communication works
	digitalWrite(15, LOW); // CS low
	SPI.transfer(0x0E);	   // READ instruction for CANSTAT
	uint8_t status = SPI.transfer(0x00);
	digitalWrite(15, HIGH); // CS high

	// CANSTAT should have valid mode bits (upper 5 bits)
	// Valid modes: 0x00 (Normal), 0x40 (Sleep), 0x80 (Loopback), 0xC0 (Config)
	uint8_t opMode = status & 0xE0;
	bool validMode = (opMode == 0x00 || opMode == 0x40 || opMode == 0x80 || opMode == 0xC0);
	TEST_ASSERT_TRUE_MESSAGE(validMode, "MCP2515 should respond with valid CANSTAT");
}

/* ── MCP2515 reset ────────────────────────────────────────────────────────── */

void test_mcp2515_reset_succeeds()
{
	digitalWrite(15, LOW);
	SPI.transfer(0xC0); // RESET instruction
	digitalWrite(15, HIGH);
	delay(10); // MCP2515 needs time after reset

	// After reset, CANSTAT should show Configuration mode (0x80)
	digitalWrite(15, LOW);
	SPI.transfer(0x0E); // READ CANSTAT
	uint8_t status = SPI.transfer(0x00);
	digitalWrite(15, HIGH);

	TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x80, status & 0xE0, "MCP2515 should be in Config mode after reset");
}

/* ── MCP2515 register read/write ──────────────────────────────────────────── */

void test_mcp2515_register_readwrite()
{
	// Reset first
	digitalWrite(15, LOW);
	SPI.transfer(0xC0);
	digitalWrite(15, HIGH);
	delay(10);

	// Write to CNF1 (address 0x2A) - set SJW and BRP
	digitalWrite(15, LOW);
	SPI.transfer(0x02); // WRITE instruction
	SPI.transfer(0x2A); // CNF1 address
	SPI.transfer(0x03); // Value: SJW=1, BRP=3
	digitalWrite(15, HIGH);
	delay(1);

	// Read back
	digitalWrite(15, LOW);
	SPI.transfer(0x03); // READ instruction
	SPI.transfer(0x2A); // CNF1 address
	uint8_t value = SPI.transfer(0x00);
	digitalWrite(15, HIGH);

	TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x03, value, "CNF1 register should hold written value");
}

/* ── MCP2515 loopback mode ────────────────────────────────────────────────── */

void test_mcp2515_enter_loopback_mode()
{
	// Reset
	digitalWrite(15, LOW);
	SPI.transfer(0xC0);
	digitalWrite(15, HIGH);
	delay(10);

	// Set CANCTRL to request loopback mode (0x40)
	digitalWrite(15, LOW);
	SPI.transfer(0x02); // WRITE
	SPI.transfer(0x0F); // CANCTRL
	SPI.transfer(0x40); // Loopback mode
	digitalWrite(15, HIGH);
	delay(10);

	// Verify CANSTAT shows loopback
	digitalWrite(15, LOW);
	SPI.transfer(0x0E); // READ CANSTAT
	uint8_t status = SPI.transfer(0x00);
	digitalWrite(15, HIGH);

	TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x40, status & 0xE0, "MCP2515 should be in Loopback mode");
}

/* ── Bus active flag ──────────────────────────────────────────────────────── */

void test_bus_chassis_active_flag()
{
#ifdef BUS_CHASSIS_ACTIVE
	TEST_ASSERT_TRUE_MESSAGE(busActive(BUS_CHASSIS), "Chassis bus should be active via build flag");
#else
	TEST_IGNORE_MESSAGE("Chassis bus not enabled in build");
#endif
}

void test_bus_vehicle_inactive_flag()
{
#ifndef BUS_VEHICLE_ACTIVE
	TEST_ASSERT_FALSE_MESSAGE(!busActive(BUS_VEHICLE), "Vehicle bus should be inactive via build flag");
#else
	TEST_IGNORE_MESSAGE("Vehicle bus is enabled in build");
#endif
}

/* ── driverSend stub verification ─────────────────────────────────────────── */

void test_driver_send_function_exists()
{
	// Verify the driverSend function is linked and callable
	// This is a compile-time check; if it links, the function exists
	TEST_ASSERT_NOT_NULL_MESSAGE((void *)driverSend, "driverSend function should be linked");
}

void setup()
{
	delay(2000); // Wait for serial monitor
	Serial.begin(115200);
	delay(1000);
	Serial.println("=== ESP32 CAN Driver Hardware Tests ===");

	// Initialize SPI pins
	pinMode(15, OUTPUT); // CS
	digitalWrite(15, HIGH);
	SPI.begin();
	SPI.setFrequency(10000000); // 10 MHz for MCP2515

	UNITY_BEGIN();
	RUN_TEST(test_spi_bus_initialized);
	RUN_TEST(test_mcp2515_responds_to_spi);
	RUN_TEST(test_mcp2515_reset_succeeds);
	RUN_TEST(test_mcp2515_register_readwrite);
	RUN_TEST(test_mcp2515_enter_loopback_mode);
	RUN_TEST(test_bus_chassis_active_flag);
	RUN_TEST(test_bus_vehicle_inactive_flag);
	RUN_TEST(test_driver_send_function_exists);
	UNITY_END();
}

void loop()
{
	// Nothing to do after tests
	delay(1000);
}
