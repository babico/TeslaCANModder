#pragma once
#include <stdint.h>
#include <string.h>
#include "types.h"

// ── Vehicle Platform ────────────────────────────────────────────────────────
// Top-level vehicle identity: Model → HW Set → Software Version
// Decoded from CAN bus (0x398 GTW_carConfig, 0x392 GTW_versionInfo)
//
// Hierarchy:
//   TeslaModel  — physical vehicle (Model S, 3, X, Y, Cybertruck)
//   HWGeneration — autopilot computer (legacy, hw3, hw4)
//   Software    — year.week.release.patch from 0x392
//
// After platform is resolved, the system validates that all enabled
// MCP2515 CAN bus modules are physically responding on SPI.

// ── Tesla Model ─────────────────────────────────────────────────────────────
enum TeslaModel : uint8_t {
  MODEL_UNKNOWN    = 0,
  MODEL_S          = 1,
  MODEL_X          = 2,
  MODEL_3          = 3,
  MODEL_Y          = 4,
  MODEL_CYBERTRUCK = 5
};

inline const char* teslaModelName(TeslaModel m) {
  switch (m) {
    case MODEL_S:          return "Model S";
    case MODEL_X:          return "Model X";
    case MODEL_3:          return "Model 3";
    case MODEL_Y:          return "Model Y";
    case MODEL_CYBERTRUCK: return "Cybertruck";
    default:               return "Unknown";
  }
}

inline bool parseTeslaModel(const char* name, TeslaModel& out) {
  if (strcmp(name, "s") == 0 || strcmp(name, "models") == 0) { out = MODEL_S; return true; }
  if (strcmp(name, "x") == 0 || strcmp(name, "modelx") == 0) { out = MODEL_X; return true; }
  if (strcmp(name, "3") == 0 || strcmp(name, "model3") == 0) { out = MODEL_3; return true; }
  if (strcmp(name, "y") == 0 || strcmp(name, "modely") == 0) { out = MODEL_Y; return true; }
  if (strcmp(name, "ct") == 0 || strcmp(name, "cybertruck") == 0) { out = MODEL_CYBERTRUCK; return true; }
  return false;
}

// ── HW Generation ───────────────────────────────────────────────────────────
enum HWGeneration : uint8_t {
  HW_UNKNOWN = 0,
  HW_LEGACY  = 1,
  HW_3       = 2,
  HW_4       = 3
};

inline const char* hwGenerationName(HWGeneration g) {
  switch (g) {
    case HW_LEGACY: return "legacy";
    case HW_3:      return "hw3";
    case HW_4:      return "hw4";
    default:        return "unknown";
  }
}

inline bool parseHWGeneration(const char* name, HWGeneration& out) {
  if (strcmp(name, "legacy") == 0) { out = HW_LEGACY; return true; }
  if (strcmp(name, "hw3") == 0)    { out = HW_3; return true; }
  if (strcmp(name, "hw4") == 0)    { out = HW_4; return true; }
  return false;
}

// Map existing Variant enum to HWGeneration
inline HWGeneration variantToHWGen(Variant v) {
  switch (v) {
    case HW4:    return HW_4;
    case HW3:    return HW_3;
    case LEGACY: return HW_LEGACY;
    default:     return HW_UNKNOWN;
  }
}

// Map HWGeneration back to Variant enum (inverse of variantToHWGen)
inline Variant hwGenToVariant(HWGeneration hw) {
  switch (hw) {
    case HW_4:      return HW4;
    case HW_3:      return HW3;
    case HW_LEGACY: return LEGACY;
    default:        return HW4;  // fallback
  }
}

// ── Software Version ────────────────────────────────────────────────────────
// Tesla versions follow: YYYY.WW[.release[.patch]]
// Examples: 2026.14.1, 2026.2.9.7, 2025.45.9
struct TeslaSoftwareVersion {
  uint16_t year;     // e.g. 2026
  uint8_t  week;     // e.g. 14 (week-of-year)
  uint8_t  release;  // e.g. 1  (major release within week branch)
  uint8_t  patch;    // e.g. 7  (patch, 0 if not present)

  bool valid() const { return year >= 2019 && week >= 1 && week <= 53; }

  // Compare: returns <0 if this < other, 0 if equal, >0 if this > other
  int compare(const TeslaSoftwareVersion& o) const {
    if (year != o.year) return (int)year - (int)o.year;
    if (week != o.week) return (int)week - (int)o.week;
    if (release != o.release) return (int)release - (int)o.release;
    return (int)patch - (int)o.patch;
  }

  bool operator>=(const TeslaSoftwareVersion& o) const { return compare(o) >= 0; }
  bool operator<(const TeslaSoftwareVersion& o) const  { return compare(o) < 0; }
};

// ── FSD Protocol Version ────────────────────────────────────────────────────
enum FsdProtocol : uint8_t {
  FSD_PROTO_UNKNOWN = 0,
  FSD_PROTO_V12     = 1,   // HW3 legacy FSD (v12.x)
  FSD_PROTO_V13     = 2,   // Transitional (v13.x)
  FSD_PROTO_V14     = 3    // AI4 / HW4 native (v14.x)
};

inline const char* fsdProtoName(FsdProtocol p) {
  switch (p) {
    case FSD_PROTO_V12: return "v12";
    case FSD_PROTO_V13: return "v13";
    case FSD_PROTO_V14: return "v14";
    default:            return "unknown";
  }
}

// Determine FSD protocol from software version + HW generation
// Based on teslascope.com observations:
//   2026.2.9+  on HW4 → v14 (AI4)
//   2026.8.x   on HW4 → v13 (transitional)
//   2026.2.9.x on HW3 → v12.6.4
//   2025.45.5+ on HW4 → v14.2.2+
//   Older      on HW3 → v12
inline FsdProtocol detectFsdProtocol(const TeslaSoftwareVersion& sw, HWGeneration hw) {
  if (!sw.valid()) return FSD_PROTO_UNKNOWN;
  if (hw == HW_LEGACY) return FSD_PROTO_V12;
  if (hw == HW_3) return FSD_PROTO_V12;

  // HW4 path
  if (sw.year >= 2026) {
    if (sw.week >= 14) return FSD_PROTO_V14;       // 2026.14+
    if (sw.week >= 8)  return FSD_PROTO_V13;        // 2026.8.x
    if (sw.week == 2 && sw.release >= 9)
      return FSD_PROTO_V14;                          // 2026.2.9+
    if (sw.week >= 2)  return FSD_PROTO_V13;        // 2026.2.x < 2.9
  }
  if (sw.year == 2025 && sw.week >= 45) return FSD_PROTO_V14;  // 2025.45+
  if (sw.year == 2025 && sw.week >= 38) return FSD_PROTO_V14;  // 2025.38+
  return FSD_PROTO_V13;
}

// ── Platform Identity (composite) ──────────────────────────────────────────
struct VehiclePlatform {
  TeslaModel          model;
  HWGeneration        hwGen;
  TeslaSoftwareVersion software;
  FsdProtocol         fsdProto;
  bool                resolved;   // true = at least model + hwGen known

  VehiclePlatform()
    : model(MODEL_UNKNOWN), hwGen(HW_UNKNOWN),
      software{0, 0, 0, 0}, fsdProto(FSD_PROTO_UNKNOWN), resolved(false) {}

  void resolve(TeslaModel m, HWGeneration hw, const TeslaSoftwareVersion& sw) {
    model = m;
    hwGen = hw;
    software = sw;
    fsdProto = detectFsdProtocol(sw, hw);
    resolved = (m != MODEL_UNKNOWN && hw != HW_UNKNOWN);
  }

  // Quick resolve from variant auto-detect + vehicle config CAN
  void resolveFromState(const State& s) {
    TeslaModel m = (TeslaModel)s.vehicleModel;
    HWGeneration hw = HW_UNKNOWN;

    // Use auto-detected HW if available, else fallback to variant setting
    if (s.hwAutoDetected) {
      if (s.detectedHW == 3) hw = HW_4;
      else if (s.detectedHW == 2) hw = HW_3;
    }
    if (hw == HW_UNKNOWN) {
      hw = variantToHWGen(s.variant);
    }

    TeslaSoftwareVersion sw;
    sw.year = s.fwYear;
    sw.week = s.fwRelease;   // fwRelease maps to week-of-year
    sw.release = s.fwMinor;
    sw.patch = 0;

    resolve(m, hw, sw);
  }
};

// ── Model-HW Compatibility Matrix ──────────────────────────────────────────
// Not all combinations exist in production
// Model S/X: legacy (pre-2021), hw3 (2021+), hw4 (2024+)
// Model 3:   hw3 (2017+), hw4 (2024+ Highland)
// Model Y:   hw3 (2020+), hw4 (2024+)
// Cybertruck: hw4 only (2024+)

struct PlatformCapabilities {
  bool supportsFsd;
  bool supportsTrackMode;
  bool supportsSummon;
  bool supportsMirrorAutoFold;
  bool supportsDualMotor;
  bool supportsEceR79Bypass;
  bool supportsNagKiller;
  bool supportsBanShield;
};

inline PlatformCapabilities getPlatformCapabilities(TeslaModel model, HWGeneration hw) {
  PlatformCapabilities cap = {
    true,   // supportsFsd
    false,  // supportsTrackMode
    true,   // supportsSummon
    true,   // supportsMirrorAutoFold
    false,  // supportsDualMotor
    false,  // supportsEceR79Bypass
    true,   // supportsNagKiller
    true    // supportsBanShield
  };

  // ECE R79 bypass only relevant for EU-market vehicles (any model)
  cap.supportsEceR79Bypass = true;

  switch (model) {
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
  if (hw == HW_LEGACY) {
    cap.supportsSummon = false;
    cap.supportsBanShield = false;
  }

  return cap;
}

// ── Software Compatibility Check ────────────────────────────────────────────
// Known-bad combinations from teslascope and community reports

enum SwCompatLevel : uint8_t {
  SW_COMPAT_UNKNOWN    = 0,
  SW_COMPAT_OK         = 1,
  SW_COMPAT_WARN       = 2,  // Works with limitations
  SW_COMPAT_BLOCKED    = 3   // Known incompatible
};

inline const char* swCompatName(SwCompatLevel c) {
  switch (c) {
    case SW_COMPAT_OK:      return "OK";
    case SW_COMPAT_WARN:    return "WARN";
    case SW_COMPAT_BLOCKED: return "BLOCKED";
    default:                return "UNKNOWN";
  }
}

inline SwCompatLevel checkSoftwareCompat(const VehiclePlatform& p) {
  if (!p.software.valid()) return SW_COMPAT_UNKNOWN;

  // Chinese gateway lockout: 2026.3.31+ on CN-market, always blocked at GTW
  // (handled separately via region detection, not here)

  // 2026.8.6 on HW4: known CAN protocol mismatch (v13 protocol, but
  // some CAN IDs shifted — use with caution)
  if (p.hwGen == HW_4 && p.software.year == 2026 &&
      p.software.week == 8 && p.software.release == 6) {
    return SW_COMPAT_WARN;
  }

  // 2026.2.9+ on HW4: FSD v14 — fully supported
  if (p.hwGen == HW_4 && p.software.year == 2026 &&
      p.software.week == 2 && p.software.release >= 9) {
    return SW_COMPAT_OK;
  }

  // Old software (pre-2024) on HW4: untested, warn
  if (p.hwGen == HW_4 && p.software.year < 2024) {
    return SW_COMPAT_WARN;
  }

  // HW3 with 2025+ software: v12.6.x, generally OK
  if (p.hwGen == HW_3 && p.software.year >= 2025) {
    return SW_COMPAT_OK;
  }

  // Legacy with any recent sw: OK but limited features
  if (p.hwGen == HW_LEGACY) {
    return SW_COMPAT_OK;
  }

  return SW_COMPAT_OK;
}

// ── Sync platform into State flat fields ────────────────────────────────────
// Call after VehiclePlatform::resolveFromState() to copy computed values
// into the flat State struct for serial/JSON output.
inline void syncPlatformToState(const VehiclePlatform& p, State& s) {
  s.platformModel     = (uint8_t)p.model;
  s.platformHwGen     = (uint8_t)p.hwGen;
  s.platformSwYear    = p.software.year;
  s.platformSwWeek    = p.software.week;
  s.platformSwRelease = p.software.release;
  s.platformSwPatch   = p.software.patch;
  s.platformFsdProto  = (uint8_t)p.fsdProto;
  s.platformSwCompat  = (uint8_t)checkSoftwareCompat(p);
  s.platformResolved  = p.resolved;
}
