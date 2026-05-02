#pragma once
#include "core/forward.h"
#include "infra/can.h"
#include "infra/parse.h"

// ── Region Detection & Spoofing ──────────────────────────────────────────────
// Detects region from GTW_carConfig (0x398) and supports optional spoofing.
// Spoofing overwrites the region bits in-frame before other modules read them,
// enabling FSD in geographically restricted regions.
//
// Also handles ECE R79 bypass for EU-region vehicles.

// Region codes from 0x398 byte[2] bits[7:4]
#define REGION_UNKNOWN 0
#define REGION_NORTH_AM 1 // North America
#define REGION_EUROPE 2	  // Europe (ECE)
#define REGION_CHINA 3	  // China (PRC)
#define REGION_ASIA_PAC 4 // Asia-Pacific (non-China)
#define REGION_MIDDLE_E 5 // Middle East

inline uint8_t decodeRegionCode(const uint8_t *data)
{
	return (data[2] >> 4) & 0x0F;
}

// Overload: decode region from Frame and populate State
inline void decodeRegionCode(const Frame &f, State &s)
{
	s.regionCode = decodeRegionCode(f.data);
	s.hasRegion = true;
}

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

inline bool isChineseMarket(uint8_t region)
{
	return region == REGION_CHINA;
}

inline bool isEuropeanMarket(uint8_t region)
{
	return region == REGION_EUROPE;
}

// ── ECE R79 Bypass ───────────────────────────────────────────────────────────
// Set UI_applyEceR79 bit in mux 1 to enable full Autopilot in EU vehicles.
// Bit 20 in mux 1 of CAN ID 1021 (FSD_MUX).

inline void applyEceR79Bypass(Frame &f)
{
	if (f.dlc >= 3)
	{
		setBit(f, 20, false); // Clear ECE R79 restriction bit
	}
}

// ── Region Spoofing ──────────────────────────────────────────────────────────
// Overwrites region nibble in 0x398 byte[2] bits[7:4] to the spoofed value.
// Returns true if the frame was modified.
inline bool applySpoofRegion(Frame &f, uint8_t spoofCode)
{
	if (spoofCode == REGION_UNKNOWN)
		return false;
	if (f.dlc < 3)
		return false;
	f.data[2] = (f.data[2] & 0x0F) | ((spoofCode & 0x0F) << 4);
	return true;
}

// Get the effective region code (spoofed if active, otherwise detected)
inline uint8_t effectiveRegion(const State &s)
{
	return (s.regionSpoofCode != REGION_UNKNOWN) ? s.regionSpoofCode : s.regionCode;
}

// ── Region Spoof Command ─────────────────────────────────────────────────────
// Commands:
//   region:spoof:na   — spoof to North America
//   region:spoof:eu   — spoof to Europe
//   region:spoof:cn   — spoof to China
//   region:spoof:apac — spoof to Asia-Pacific
//   region:spoof:me   — spoof to Middle East
//   region:spoof:off  — disable spoofing (use real region)
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

bool executeRegionSpoofCmd(const char *cmd, State &s)
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
