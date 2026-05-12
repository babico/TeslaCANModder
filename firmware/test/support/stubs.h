#pragma once

/**
 * @file firmware/test/support/stubs.h
 * @brief Recording stubs for driverSend and no-op stubs used by handler/dispatch tests
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/types.h"

/**
 * @brief Record of a single driverSend invocation for test assertions
 */
struct SendCall
{
	Frame f;      // Frame that was sent
	uint8_t bus;  // Bus index the frame was sent on
};

static SendCall stub_sends[16];   // Circular buffer of recorded sends (max 16)
static uint8_t stub_send_count = 0;

/**
 * @brief Recording stub for driverSend — captures frame and bus into stub_sends[]
 * @param f Frame to record
 * @param bus Bus index to record
 */
void driverSend(const Frame &f, uint8_t bus)
{
	if (stub_send_count < 16)
	{
		stub_sends[stub_send_count].f = f;
		stub_sends[stub_send_count].bus = bus;
		stub_send_count++;
	}
}

/**
 * @brief No-op sendLog stub (C-string overload)
 * @param msg Unused log message
 */
void sendLog(const char *) {}

/**
 * @brief No-op sendLog stub (Flash-string overload)
 * @param msg Unused flash-string log message
 */
void sendLog(const __FlashStringHelper *) {}

/**
 * @brief No-op saveSettings stub
 * @param state Unused state reference
 */
void saveSettings(const State &) {}

/**
 * @brief No-op resetHandlerLogFlags stub
 */
void resetHandlerLogFlags() {}

/**
 * @brief No-op applyFilters stub
 * @param state Unused state reference
 */
void applyFilters(State &) {}
