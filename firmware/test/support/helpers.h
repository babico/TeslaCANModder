#pragma once

/**
 * @file firmware/test/support/helpers.h
 * @brief Shared test helper utilities for native PlatformIO Unity tests
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/types.h"

/**
 * @brief Create a zeroed Frame with the given CAN ID and DLC
 * @param id CAN identifier to assign
 * @param dlc Data length code (defaults to 8)
 * @return Initialized Frame struct with all data bytes zeroed
 */
static Frame makeFrame(uint32_t id, uint8_t dlc = 8)
{
	Frame f = {};
	f.id = id;
	f.dlc = dlc;
	return f;
}
