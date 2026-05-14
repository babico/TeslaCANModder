#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/region.h
 * @brief Region detection, spoofing, and ECE R79 bypass for Tesla CAN
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "vehicle/can/ids.h"
#include "core/util/parse.h"

// Region codes extracted from GTW_carConfig (0x398) byte[2] bits[7:4]
#define REGION_UNKNOWN 0
#define REGION_NORTH_AM 1
#define REGION_EUROPE 2
#define REGION_CHINA 3
#define REGION_ASIA_PAC 4
#define REGION_MIDDLE_E 5

/**
 * @brief Decode the region code from raw CAN data
 * @param data Pointer to frame payload (requires at least 3 bytes)
 * @return Region code from the upper nibble of byte 2
 */
inline uint8_t decodeRegionCode(const uint8_t *data)
{
	return (data[2] >> 4) & 0x0F;
}

/**
 * @brief Decode region code from a CAN frame and store in vehicle state
 * @param f CAN frame containing GTW_carConfig data
 * @param s Global vehicle state to populate
 */
inline void decodeRegionCode(const Frame &f, State &s)
{
	s.regionCode = decodeRegionCode(f.data);
	s.hasRegion = true;
}

/**
 * @brief Get a short human-readable name for a region code
 * @param code Region code constant
 * @return Two-to-four character region abbreviation, or "UNK" if unrecognized
 */
inline const char *regionName(uint8_t code)
{
	switch (code)
	{
	case REGION_NORTH_AM:
		return "NA";
	case REGION_EUROPE:
		return "EU";
	case REGION_CHINA:
		return "CN";
	case REGION_ASIA_PAC:
		return "APAC";
	case REGION_MIDDLE_E:
		return "ME";
	default:
		return "UNK";
	}
}

/**
 * @brief Check if the region code indicates the Chinese market
 * @param region Region code to test
 * @return True if the region is China (PRC)
 */
inline bool isChineseMarket(uint8_t region)
{
	return region == REGION_CHINA;
}

/**
 * @brief Check if the region code indicates the European market
 * @param region Region code to test
 * @return True if the region is Europe (ECE)
 */
inline bool isEuropeanMarket(uint8_t region)
{
	return region == REGION_EUROPE;
}

/**
 * @brief Apply ECE R79 bypass by clearing the restriction bit in the FSD mux frame
 * @param f CAN frame (mux 1 of CAN ID 1021) to modify
 *
 * @note Clears bit 20 in mux 1 to enable full Autopilot functionality
 *       on EU-region vehicles that are otherwise restricted by ECE R79.
 */
inline void applyEceR79Bypass(Frame &f)
{
	if (f.dlc >= 3)
	{
		setBit(f, 20, false);
	}
}

/**
 * @brief Overwrite the region nibble in a GTW_carConfig frame with a spoofed value
 * @param f CAN frame (0x398) to modify
 * @param spoofCode Region code to inject into bits[7:4] of byte 2
 * @return True if the frame was modified, false if spoofing is disabled or DLC too short
 */
inline bool applySpoofRegion(Frame &f, uint8_t spoofCode)
{
	if (spoofCode == REGION_UNKNOWN)
		return false;
	if (f.dlc < 3)
		return false;
	f.data[2] = (f.data[2] & 0x0F) | ((spoofCode & 0x0F) << 4);
	return true;
}

/**
 * @brief Get the effective region code, preferring the spoofed value if active
 * @param s Global vehicle state
 * @return Spoofed region code if set, otherwise the detected region code
 */
inline uint8_t effectiveRegion(const State &s)
{
	return (s.regionSpoofCode != REGION_UNKNOWN) ? s.regionSpoofCode : s.regionCode;
}

/**
 * @brief Parse a region name string into its corresponding region code
 * @param name Short region identifier ("na", "eu", "cn", "apac", "me", "off")
 * @param out Output region code on success
 * @return True if the name was recognized
 */
inline bool parseRegionCode(const char *name, uint8_t &out)
{
	if (strcmp(name, "na") == 0)
	{
		out = REGION_NORTH_AM;
		return true;
	}
	if (strcmp(name, "eu") == 0)
	{
		out = REGION_EUROPE;
		return true;
	}
	if (strcmp(name, "cn") == 0)
	{
		out = REGION_CHINA;
		return true;
	}
	if (strcmp(name, "apac") == 0)
	{
		out = REGION_ASIA_PAC;
		return true;
	}
	if (strcmp(name, "me") == 0)
	{
		out = REGION_MIDDLE_E;
		return true;
	}
	if (strcmp(name, "off") == 0)
	{
		out = REGION_UNKNOWN;
		return true;
	}
	return false;
}

/**
 * @brief Execute a region spoof command
 * @param cmd Command string (e.g. "region:spoof:na", "region:spoof:eu", "region:spoof:off")
 * @param s Global vehicle state
 * @return True if the command was recognized and executed
 */
static bool executeRegionSpoofCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "region:spoof:", 13) != 0)
		return false;
	uint8_t code;
	if (!parseRegionCode(cmd + 13, code))
		return false;
	s.regionSpoofCode = code;
	resetHandlerLogFlags();
	saveSettings(s);
	return true;
}
