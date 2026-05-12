#pragma once

/**
 * @file firmware/lib/core/forward.h
 * @brief Shared forward declarations for platform-specific functions used across the firmware
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/types.h"

// Provide no-op stubs for __FlashStringHelper and F() on native builds
// so headers compile cleanly without the Arduino framework.
#ifndef ARDUINO
class __FlashStringHelper;
#ifndef F
#define F(string_literal) (reinterpret_cast<const __FlashStringHelper *>(string_literal))
#endif
#endif

/**
 * @brief Send a log message via the platform serial layer
 * @param msg Null-terminated log string
 */
void sendLog(const char *msg);

/**
 * @brief Send a flash-stored log message via the platform serial layer
 * @param msg Flash string pointer (F() macro on Arduino)
 */
void sendLog(const __FlashStringHelper *msg);

/**
 * @brief Log a message exactly once per boot (or until the flag is reset)
 *
 * Usage: ONCE_LOG(hw4LoggedNag, F("HW4: Nag suppressed on CAN"));
 */
#define ONCE_LOG(flag, msg) \
	do { if (!(flag)) { sendLog(msg); (flag) = true; } } while (0)

/**
 * @brief Transmit a CAN frame on the specified bus via the MCP2515 driver
 * @param f Frame to send
 * @param bus Bus index (0=Chassis, 1=Vehicle, 2=Body)
 */
void driverSend(const Frame &f, uint8_t bus);

/**
 * @brief Get and atomically reset the accumulated TX failure counter
 * @return Number of MCP2515 sendMessage() failures since last call
 */
uint32_t driverGetAndResetTxFails();

/**
 * @brief Get and atomically reset the accumulated bus-off event counter
 * @return Number of CAN bus-off events since last call
 */
uint32_t driverGetAndResetBusOffEvents();

/**
 * @brief Poll MCP2515 error registers and update internal bus error state
 */
void driverPollBusErrors();

/**
 * @brief Persist the current settings from State to non-volatile storage
 * @param s State struct containing values to save
 */
void saveSettings(const State &s);

/**
 * @brief Reset all one-shot log flags in the handler layer
 */
void resetHandlerLogFlags();

/**
 * @brief Apply CAN ID filters to the MCP2515 hardware based on current state
 * @param s State struct used to determine which filters to configure
 */
void applyFilters(State &s);
