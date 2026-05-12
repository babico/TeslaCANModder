#pragma once

/**
 * @file firmware/lib/core/platform.h
 * @brief Vehicle platform identity: model, hardware generation, software version, and capability matrix
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <stdint.h>
#include <string.h>
#include "types.h"

/**
 * @brief Physical Tesla vehicle model decoded from 0x398 GTW_carConfig
 */
enum TeslaModel : uint8_t
{
	MODEL_UNKNOWN = 0,
	MODEL_S = 1,
	MODEL_X = 2,
	MODEL_3 = 3,
	MODEL_Y = 4,
	MODEL_CYBERTRUCK = 5
};

/**
 * @brief Return a human-readable name for a TeslaModel value
 * @param m Tesla model enum
 * @return Null-terminated model name string
 */
inline const char *teslaModelName(TeslaModel m)
{
	switch (m)
	{
	case MODEL_S:
		return "Model S";
	case MODEL_X:
		return "Model X";
	case MODEL_3:
		return "Model 3";
	case MODEL_Y:
		return "Model Y";
	case MODEL_CYBERTRUCK:
		return "Cybertruck";
	default:
		return "Unknown";
	}
}

/**
 * @brief Parse a string into a TeslaModel enum value
 * @param name Input string (e.g. "s", "models", "3", "ct")
 * @param out Output TeslaModel on success
 * @return True if parsing succeeded
 */
inline bool parseTeslaModel(const char *name, TeslaModel &out)
{
	if (strcmp(name, "s") == 0 || strcmp(name, "models") == 0)
	{
		out = MODEL_S;
		return true;
	}
	if (strcmp(name, "x") == 0 || strcmp(name, "modelx") == 0)
	{
		out = MODEL_X;
		return true;
	}
	if (strcmp(name, "3") == 0 || strcmp(name, "model3") == 0)
	{
		out = MODEL_3;
		return true;
	}
	if (strcmp(name, "y") == 0 || strcmp(name, "modely") == 0)
	{
		out = MODEL_Y;
		return true;
	}
	if (strcmp(name, "ct") == 0 || strcmp(name, "cybertruck") == 0)
	{
		out = MODEL_CYBERTRUCK;
		return true;
	}
	return false;
}

/**
 * @brief Autopilot computer hardware generation decoded from CAN
 */
enum HWGeneration : uint8_t
{
	HW_UNKNOWN = 0,
	HW_LEGACY = 1, // Pre-HW3 (AP1/AP2)
	HW_3 = 2,      // HW3 autopilot computer
	HW_4 = 3       // HW4 / AI4 autopilot computer
};

/**
 * @brief Return a human-readable name for a HWGeneration value
 * @param g Hardware generation enum
 * @return Null-terminated generation name string
 */
inline const char *hwGenerationName(HWGeneration g)
{
	switch (g)
	{
	case HW_LEGACY:
		return "legacy";
	case HW_3:
		return "hw3";
	case HW_4:
		return "hw4";
	default:
		return "unknown";
	}
}

/**
 * @brief Parse a string into a HWGeneration enum value
 * @param name Input string (e.g. "legacy", "hw3", "hw4")
 * @param out Output HWGeneration on success
 * @return True if parsing succeeded
 */
inline bool parseHWGeneration(const char *name, HWGeneration &out)
{
	if (strcmp(name, "legacy") == 0)
	{
		out = HW_LEGACY;
		return true;
	}
	if (strcmp(name, "hw3") == 0)
	{
		out = HW_3;
		return true;
	}
	if (strcmp(name, "hw4") == 0)
	{
		out = HW_4;
		return true;
	}
	return false;
}

/**
 * @brief Convert a Variant enum to the corresponding HWGeneration
 * @param v Variant value
 * @return Matching HWGeneration, or HW_UNKNOWN if unrecognized
 */
inline HWGeneration variantToHWGen(Variant v)
{
	switch (v)
	{
	case HW4:
		return HW_4;
	case HW3:
		return HW_3;
	case LEGACY:
		return HW_LEGACY;
	default:
		return HW_UNKNOWN;
	}
}

/**
 * @brief Convert a HWGeneration back to the corresponding Variant enum
 * @param hw Hardware generation value
 * @return Matching Variant, or HW4 as fallback
 */
inline Variant hwGenToVariant(HWGeneration hw)
{
	switch (hw)
	{
	case HW_4:
		return HW4;
	case HW_3:
		return HW3;
	case HW_LEGACY:
		return LEGACY;
	default:
		return HW4; // fallback to newest variant
	}
}

/**
 * @brief Tesla software version following YYYY.WW[.release[.patch]] format
 *
 * Examples: 2026.14.1, 2026.2.9.7, 2025.45.9
 */
struct TeslaSoftwareVersion
{
	uint16_t year;   // e.g. 2026
	uint8_t week;    // Week-of-year (1-53)
	uint8_t release; // Major release within week branch
	uint8_t patch;   // Patch number (0 if not present)

	/**
	 * @brief Check if this version has plausible year and week values
	 * @return True if year >= 2019 and week is in [1, 53]
	 */
	bool valid() const
	{
		return year >= 2019 && week >= 1 && week <= 53;
	}

	/**
	 * @brief Lexicographic comparison of two software versions
	 * @param o Other version to compare against
	 * @return Negative if this < o, zero if equal, positive if this > o
	 */
	int compare(const TeslaSoftwareVersion &o) const
	{
		if (year != o.year)
			return (int)year - (int)o.year;
		if (week != o.week)
			return (int)week - (int)o.week;
		if (release != o.release)
			return (int)release - (int)o.release;
		return (int)patch - (int)o.patch;
	}

	bool operator>=(const TeslaSoftwareVersion &o) const
	{
		return compare(o) >= 0;
	}
	bool operator<(const TeslaSoftwareVersion &o) const
	{
		return compare(o) < 0;
	}
};

/**
 * @brief FSD protocol version determined by software version and hardware generation
 */
enum FsdProtocol : uint8_t
{
	FSD_PROTO_UNKNOWN = 0,
	FSD_PROTO_V12 = 1, // HW3 legacy FSD (v12.x)
	FSD_PROTO_V13 = 2, // Transitional (v13.x)
	FSD_PROTO_V14 = 3  // AI4 / HW4 native (v14.x)
};

/**
 * @brief Return a human-readable name for an FsdProtocol value
 * @param p FSD protocol enum
 * @return Null-terminated protocol name string
 */
inline const char *fsdProtoName(FsdProtocol p)
{
	switch (p)
	{
	case FSD_PROTO_V12:
		return "v12";
	case FSD_PROTO_V13:
		return "v13";
	case FSD_PROTO_V14:
		return "v14";
	default:
		return "unknown";
	}
}

/**
 * @brief Determine FSD protocol from software version and hardware generation
 * @param sw Software version struct
 * @param hw Hardware generation
 * @return Detected FSD protocol level
 *
 * @note Based on teslascope.com observations of production vehicles
 */
inline FsdProtocol detectFsdProtocol(const TeslaSoftwareVersion &sw, HWGeneration hw)
{
	if (!sw.valid())
		return FSD_PROTO_UNKNOWN;
	if (hw == HW_LEGACY)
		return FSD_PROTO_V12;
	if (hw == HW_3)
		return FSD_PROTO_V12;

	// HW4 path — version thresholds from teslascope observations
	if (sw.year >= 2026)
	{
		if (sw.week >= 14)
			return FSD_PROTO_V14; // 2026.14+
		if (sw.week >= 8)
			return FSD_PROTO_V13; // 2026.8.x transitional
		if (sw.week == 2 && sw.release >= 9)
			return FSD_PROTO_V14; // 2026.2.9+
		if (sw.week >= 2)
			return FSD_PROTO_V13; // 2026.2.x < 2.9
	}
	if (sw.year == 2025 && sw.week >= 45)
		return FSD_PROTO_V14; // 2025.45+
	if (sw.year == 2025 && sw.week >= 38)
		return FSD_PROTO_V14; // 2025.38+
	return FSD_PROTO_V13;
}

/**
 * @brief Composite vehicle platform identity combining model, hardware, and software
 */
struct VehiclePlatform
{
	TeslaModel model;
	HWGeneration hwGen;
	TeslaSoftwareVersion software;
	FsdProtocol fsdProto;
	bool resolved; // True = at least model + hwGen known

	VehiclePlatform()
		: model(MODEL_UNKNOWN), hwGen(HW_UNKNOWN), software{0, 0, 0, 0}, fsdProto(FSD_PROTO_UNKNOWN), resolved(false)
	{
	}

	/**
	 * @brief Resolve platform identity from explicit model, hardware, and software version
	 * @param m Tesla model
	 * @param hw Hardware generation
	 * @param sw Software version
	 */
	void resolve(TeslaModel m, HWGeneration hw, const TeslaSoftwareVersion &sw)
	{
		model = m;
		hwGen = hw;
		software = sw;
		fsdProto = detectFsdProtocol(sw, hw);
		resolved = (m != MODEL_UNKNOWN && hw != HW_UNKNOWN);
	}

	/**
	 * @brief Resolve platform identity from the global State struct fields
	 * @param s State containing auto-detected HW, variant, and firmware version fields
	 */
	void resolveFromState(const State &s)
	{
		TeslaModel m = (TeslaModel)s.vehicleModel;
		HWGeneration hw = HW_UNKNOWN;

		// Prefer auto-detected HW from 0x398, fall back to variant setting
		if (s.hwAutoDetected)
		{
			if (s.detectedHW == 3)
				hw = HW_4;
			else if (s.detectedHW == 2)
				hw = HW_3;
		}
		if (hw == HW_UNKNOWN)
		{
			hw = variantToHWGen(s.variant);
		}

		TeslaSoftwareVersion sw;
		sw.year = s.fwYear;
		sw.week = s.fwRelease; // fwRelease maps to week-of-year
		sw.release = s.fwMinor;
		sw.patch = 0;

		resolve(m, hw, sw);
	}
};

/**
 * @brief Per-model/HW feature capability flags
 */
struct PlatformCapabilities
{
	bool supportsFsd;
	bool supportsTrackMode;
	bool supportsSummon;
	bool supportsMirrorAutoFold;
	bool supportsDualMotor;
	bool supportsEceR79Bypass;
	bool supportsNagKiller;
	bool supportsBanShield;
};

/**
 * @brief Determine platform capabilities based on vehicle model and hardware generation
 * @param model Tesla model
 * @param hw Hardware generation
 * @return PlatformCapabilities with per-feature support flags
 */
inline PlatformCapabilities getPlatformCapabilities(TeslaModel model, HWGeneration hw)
{
	PlatformCapabilities cap = {
		true,  // supportsFsd
		false, // supportsTrackMode
		true,  // supportsSummon
		true,  // supportsMirrorAutoFold
		false, // supportsDualMotor
		false, // supportsEceR79Bypass
		true,  // supportsNagKiller
		true   // supportsBanShield
	};

	// ECE R79 bypass relevant for EU-market vehicles (any model)
	cap.supportsEceR79Bypass = true;

	switch (model)
	{
	case MODEL_S:
		cap.supportsTrackMode = true;
		cap.supportsDualMotor = true;
		break;
	case MODEL_X:
		cap.supportsDualMotor = true;
		break;
	case MODEL_3:
		cap.supportsTrackMode = true;
		cap.supportsDualMotor = true;
		break;
	case MODEL_Y:
		cap.supportsDualMotor = true;
		break;
	case MODEL_CYBERTRUCK:
		cap.supportsTrackMode = true;
		cap.supportsDualMotor = true;
		break;
	default:
		break;
	}

	// Legacy HW has reduced capability
	if (hw == HW_LEGACY)
	{
		cap.supportsSummon = false;
		cap.supportsBanShield = false;
	}

	return cap;
}

/**
 * @brief Software compatibility level for known-good/bad version combinations
 */
enum SwCompatLevel : uint8_t
{
	SW_COMPAT_UNKNOWN = 0,
	SW_COMPAT_OK = 1,
	SW_COMPAT_WARN = 2,   // Works with limitations
	SW_COMPAT_BLOCKED = 3 // Known incompatible
};

/**
 * @brief Return a human-readable name for a SwCompatLevel value
 * @param c Compatibility level enum
 * @return Null-terminated level name string
 */
inline const char *swCompatName(SwCompatLevel c)
{
	switch (c)
	{
	case SW_COMPAT_OK:
		return "OK";
	case SW_COMPAT_WARN:
		return "WARN";
	case SW_COMPAT_BLOCKED:
		return "BLOCKED";
	default:
		return "UNKNOWN";
	}
}

/**
 * @brief Check software compatibility for a resolved vehicle platform
 * @param p Resolved VehiclePlatform
 * @return Compatibility level based on known-good/bad version combinations
 */
inline SwCompatLevel checkSoftwareCompat(const VehiclePlatform &p)
{
	if (!p.software.valid())
		return SW_COMPAT_UNKNOWN;

	// 2026.8.6 on HW4: known CAN protocol mismatch (v13 protocol with shifted IDs)
	if (p.hwGen == HW_4 && p.software.year == 2026 && p.software.week == 8 && p.software.release == 6)
	{
		return SW_COMPAT_WARN;
	}

	// 2026.2.9+ on HW4: FSD v14 — fully supported
	if (p.hwGen == HW_4 && p.software.year == 2026 && p.software.week == 2 && p.software.release >= 9)
	{
		return SW_COMPAT_OK;
	}

	// Old software (pre-2024) on HW4: untested combination
	if (p.hwGen == HW_4 && p.software.year < 2024)
	{
		return SW_COMPAT_WARN;
	}

	// HW3 with 2025+ software: v12.6.x, generally OK
	if (p.hwGen == HW_3 && p.software.year >= 2025)
	{
		return SW_COMPAT_OK;
	}

	// Legacy with any recent software: OK but limited features
	if (p.hwGen == HW_LEGACY)
	{
		return SW_COMPAT_OK;
	}

	return SW_COMPAT_OK;
}

/**
 * @brief Copy computed platform fields into the flat State struct for serial/JSON output
 * @param p Resolved VehiclePlatform
 * @param s State struct to update
 *
 * @note Call after VehiclePlatform::resolveFromState() to sync computed values
 */
inline void syncPlatformToState(const VehiclePlatform &p, State &s)
{
	s.platformModel = (uint8_t)p.model;
	s.platformHwGen = (uint8_t)p.hwGen;
	s.platformSwYear = p.software.year;
	s.platformSwWeek = p.software.week;
	s.platformSwRelease = p.software.release;
	s.platformSwPatch = p.software.patch;
	s.platformFsdProto = (uint8_t)p.fsdProto;
	s.platformSwCompat = (uint8_t)checkSoftwareCompat(p);
	s.platformResolved = p.resolved;
}
