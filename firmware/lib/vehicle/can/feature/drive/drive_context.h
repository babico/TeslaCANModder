#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/drive/drive_context.h
 * @brief Drive context decode helpers for door/closure state and speed-limit signals
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/types.h"
#include "vehicle/can/ids.h"

/**
 * @brief Read an arbitrary bit field from a CAN payload in little-endian bit order.
 * @param data Pointer to the raw CAN data bytes.
 * @param start Starting bit position (LSB-first numbering).
 * @param len Number of bits to extract (max 32).
 * @return Extracted value as a 32-bit unsigned integer.
 */
inline uint32_t readBitsLE(const uint8_t *data, uint8_t start, uint8_t len)
{
	uint32_t out = 0;
	for (uint8_t i = 0; i < len; ++i)
	{
		const uint8_t bit = start + i;
		const uint8_t byteIdx = bit / 8;
		const uint8_t bitIdx = bit % 8;
		if (data[byteIdx] & (1u << bitIdx))
			out |= (1u << i);
	}
	return out;
}

/**
 * @brief Determine if a latch status value indicates an open/ajar/moving state.
 * @param status Raw 4-bit latch status from the CAN frame.
 * @return True if the closure is not fully closed and not unknown.
 */
inline bool decodeLatchOpen(uint8_t status)
{
	// 2 = closed, 0 = unknown/SNA; all other values indicate open/ajar/moving
	return status != 2 && status != 0;
}

/**
 * @brief Decode front-left door open state from a closure status frame.
 * @param f CAN frame containing door latch data.
 * @return True if the front-left door is open.
 */
inline bool decodeDoorFrontLeftOpen(const Frame &f)
{
	return decodeLatchOpen((uint8_t)(readBitsLE(f.data, 0, 4) & 0x0F));
}

/**
 * @brief Decode rear-left door open state from a closure status frame.
 * @param f CAN frame containing door latch data.
 * @return True if the rear-left door is open.
 */
inline bool decodeDoorRearLeftOpen(const Frame &f)
{
	return decodeLatchOpen((uint8_t)(readBitsLE(f.data, 4, 4) & 0x0F));
}

/**
 * @brief Decode front-right door open state from a closure status frame.
 * @param f CAN frame containing door latch data.
 * @return True if the front-right door is open.
 */
inline bool decodeDoorFrontRightOpen(const Frame &f)
{
	return decodeLatchOpen((uint8_t)(readBitsLE(f.data, 0, 4) & 0x0F));
}

/**
 * @brief Decode rear-right door open state from a closure status frame.
 * @param f CAN frame containing door latch data.
 * @return True if the rear-right door is open.
 */
inline bool decodeDoorRearRightOpen(const Frame &f)
{
	return decodeLatchOpen((uint8_t)(readBitsLE(f.data, 4, 4) & 0x0F));
}

/**
 * @brief Decode trunk open state from a closure status frame.
 * @param f CAN frame containing trunk latch data.
 * @return True if the trunk is open.
 */
inline bool decodeTrunkOpen(const Frame &f)
{
	return decodeLatchOpen((uint8_t)(readBitsLE(f.data, 56, 4) & 0x0F)); // bits 56-59
}

/**
 * @brief Decode frunk open state from a multiplexed closure frame.
 * @param f CAN frame containing frunk latch data (mux 0 only).
 * @return True if the frunk is open.
 */
inline bool decodeFrunkOpen(const Frame &f)
{
	const uint8_t mux = (uint8_t)(readBitsLE(f.data, 0, 3) & MUX_MASK); // bits 0-2: mux selector
	if (mux != 0)
		return false;
	return decodeLatchOpen((uint8_t)(readBitsLE(f.data, 3, 4) & 0x0F)); // bits 3-6: frunk latch
}

/**
 * @brief Decode aggregate any-door-open flag from a multiplexed closure frame.
 * @param f CAN frame containing aggregate door status (mux 0 only).
 * @return True if any door is reported open.
 */
inline bool decodeAnyDoorOpen(const Frame &f)
{
	const uint8_t mux = (uint8_t)(readBitsLE(f.data, 0, 3) & MUX_MASK); // bits 0-2: mux selector
	if (mux != 0)
		return false;
	return readBitsLE(f.data, 50, 1) != 0; // bit 50: any-door-open flag
}

/**
 * @brief Decode driver door open state from a door status frame.
 * @param f CAN frame containing driver door status.
 * @return True if the driver door is open.
 */
inline bool decodeDriverDoorOpen(const Frame &f)
{
	// DBC convention: 1 = closed, 0 = open
	return readBitsLE(f.data, 31, 1) == 0; // bit 31: driver door closed flag
}

/**
 * @brief Decode cruise control set speed from a DAS status frame.
 * @param f CAN frame containing cruise set speed (12-bit field).
 * @return Cruise set speed in km/h, or 0.0 if unavailable (SNA = 0xFFF).
 */
inline float decodeCruiseSetSpeedKph(const Frame &f)
{
	uint16_t raw = (uint16_t)(readBitsLE(f.data, 0, 12) & 0x0FFF); // bits 0-11: raw speed
	if (raw == 0x0FFF) // SNA sentinel
		return 0.0f;
	return raw * 0.1f; // resolution: 0.1 km/h per count
}

/**
 * @brief Decode ACC speed limit from a DAS constraint frame.
 * @param f CAN frame containing ACC speed limit (10-bit field, mph units).
 * @return Speed limit in km/h, or 0.0 if unavailable.
 */
inline float decodeAccSpeedLimitKph(const Frame &f)
{
	uint16_t raw = (uint16_t)(readBitsLE(f.data, 0, 10) & 0x03FF); // bits 0-9: raw limit
	if (raw == 0 || raw == 0x03FF) // 0 or SNA sentinel
		return 0.0f;
	const float mph = raw * 0.2f; // resolution: 0.2 mph per count
	return mph * 1.60934f; // convert mph to km/h
}

/**
 * @brief Decode map-based speed limit from a navigation frame.
 * @param f CAN frame containing map speed limit and unit flag.
 * @return Speed limit in km/h, or 0.0 if unavailable.
 */
inline float decodeMapSpeedLimitKph(const Frame &f)
{
	uint8_t raw = (uint8_t)(readBitsLE(f.data, 48, 5) & 0x1F); // bits 48-52: raw limit
	if (raw == 0 || raw == 31) // 0 or SNA sentinel
		return 0.0f;
	float v = raw * 5.0f; // resolution: 5 units per count
	const bool unitsKph = readBitsLE(f.data, 46, 1) != 0; // bit 46: unit flag (1=km/h, 0=mph)
	if (!unitsKph)
		v *= 1.60934f; // convert mph to km/h
	return v;
}
