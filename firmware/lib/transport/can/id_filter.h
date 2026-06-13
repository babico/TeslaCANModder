#pragma once

/**
 * @file firmware/lib/transport/can/id_filter.h
 * @brief O(1) bitmask-based CAN ID accept/reject filter for standard 11-bit IDs
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <stdint.h>

/**
 * @brief Maximum 11-bit CAN identifier (exclusive upper bound)
 *
 * Standard CAN IDs occupy 11 bits, so valid IDs range from 0 to CAN_ID_MAX-1.
 * Used to size the bitmask filter and to bound-range-check add/remove/test.
 */
static constexpr uint32_t CAN_ID_MAX = 2048; // 11-bit ID space

static constexpr size_t ID_FILTER_WORDS = 64; // 2048 bits / 32 bits per word

/**
 * @brief Bitmask filter for constant-time CAN ID acceptance testing
 */
struct IdFilter
{
	uint32_t bits[ID_FILTER_WORDS]; // 256-byte bitmask covering IDs 0-2047
};

/**
 * @brief Clear all accepted IDs from the filter
 * @param f Reference to the filter to clear
 */
inline void idFilterClear(IdFilter &f)
{
	for (uint8_t i = 0; i < ID_FILTER_WORDS; i++)
		f.bits[i] = 0;
}

/**
 * @brief Mark a CAN ID as accepted in the filter
 * @param f Reference to the filter to modify
 * @param id 11-bit CAN identifier to accept (0-2047)
 */
inline void idFilterAdd(IdFilter &f, uint16_t id)
{
	if (id < CAN_ID_MAX)
	{
		f.bits[id >> 5] |= (1UL << (id & 0x1F)); // Set bit at word[id/32], position id%32
	}
}

/**
 * @brief Remove a CAN ID from the accepted set
 * @param f Reference to the filter to modify
 * @param id 11-bit CAN identifier to reject (0-2047)
 */
inline void idFilterRemove(IdFilter &f, uint16_t id)
{
	if (id < CAN_ID_MAX)
	{
		f.bits[id >> 5] &= ~(1UL << (id & 0x1F)); // Clear bit at word[id/32], position id%32
	}
}

/**
 * @brief Test whether a CAN ID is accepted by the filter
 * @param f Reference to the filter to query
 * @param id 11-bit CAN identifier to test (0-2047)
 * @return True if the ID is in the accepted set
 */
inline bool idFilterTest(const IdFilter &f, uint16_t id)
{
	if (id >= CAN_ID_MAX)
		return false;
	return (f.bits[id >> 5] & (1UL << (id & 0x1F))) != 0; // Test bit at word[id/32], position id%32
}

/**
 * @brief Software accept filter for bus 0 (chassis)
 */
static IdFilter swFilterBus0;

/**
 * @brief Software accept filter for bus 1 (vehicle)
 */
static IdFilter swFilterBus1;

/**
 * @brief Initialize both software filters to reject all IDs
 */
inline void swFilterInit()
{
	idFilterClear(swFilterBus0);
	idFilterClear(swFilterBus1);
}

/**
 * @brief Check if a frame ID should be processed on the given bus
 * @param bus Bus index (0=chassis, 1=vehicle, 2=body)
 * @param id 11-bit CAN identifier to test
 * @return True if the frame should be accepted for processing
 */
inline bool swFilterAccept(uint8_t bus, uint16_t id)
{
	switch (bus)
	{
	case 0:
		return idFilterTest(swFilterBus0, id);
	case 1:
		return idFilterTest(swFilterBus1, id);
	default:
		return true; // Bus 2 (body) passes all frames unfiltered
	}
}
