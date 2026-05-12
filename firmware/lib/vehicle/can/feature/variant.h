#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/variant.h
 * @brief Variant selection command for manual or auto-detect mode
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"

/**
 * @brief Execute a "variant:" command to set the vehicle variant.
 *
 * Supports "variant:auto" to enable auto-detection from CAN 0x398,
 * or an explicit variant name (e.g. "variant:hw3") to override.
 * Manual override disables auto-detection. Persists the setting
 * and reapplies CAN filters.
 *
 * @param cmd Null-terminated command string (e.g. "variant:auto").
 * @param s Device state for variant storage, persistence, and filters.
 * @return True if the command was recognized and executed successfully.
 */
static bool executeVariantCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "variant:", 8) == 0)
	{
		const char *val = cmd + 8;
		// "auto" enables runtime detection from CAN 0x398
		if (strcmp(val, "auto") == 0)
		{
			s.variantAutoDetect = true;
			saveSettings(s);
			applyFilters(s);
			return true;
		}
		Variant v;
		if (!parseVariant(val, v))
			return false;
		s.variant = v;
		s.variantAutoDetect = false; // Manual override disables auto-detect
		saveSettings(s);
		applyFilters(s);
		return true;
	}
	return false;
}
