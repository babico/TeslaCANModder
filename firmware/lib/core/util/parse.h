#pragma once

/**
 * @file firmware/lib/core/util/parse.h
 * @brief Shared utility for parsing boolean on/off style command suffixes
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <cstring>

/**
 * @brief Parse a boolean command suffix ("on" or "off") into a bool value
 * @param suffix Null-terminated string to match ("on" or "off")
 * @param current The current boolean state (unused, available for toggle logic)
 * @param out Reference set to true for "on", false for "off"
 * @return true if suffix matched "on" or "off", false otherwise
 */
static bool parseBoolCmd(const char *suffix, bool current, bool &out)
{
	if (strcmp(suffix, "on") == 0)
	{
		out = true;
		return true;
	}
	if (strcmp(suffix, "off") == 0)
	{
		out = false;
		return true;
	}
	return false;
}
