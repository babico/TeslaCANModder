#pragma once
#include "core/types.h"
#include "infra/can.h"

// ── 1.7 Firmware Version Compatibility ──────────────────────────────────────
// Decode the vehicle firmware version string from CAN 0x392 (GTW_versionInfo)
// and check against known-incompatible combinations:
//   - 2026.2.9+ (FSD v14) + HW4 → OK
//   - 2026.8.x  (FSD v13) + HW4 → WARNING: legacy protocol
//   - 2026.2.9.x & 2026.8.6     → INCOMPATIBLE
//
// CAN ID 0x392, mux field = byte 0 bits [2:0]:
//   mux 0: firmware major.minor (bytes 1-4)
//   mux 1: firmware build number (bytes 1-4)
//
// Command: fwcompat (query only — returns JSON with version & compat status)

#define CAN_ID_GTW_VERSION 0x392

// Firmware compatibility level
enum FwCompatLevel : uint8_t {
  FW_COMPAT_UNKNOWN   = 0,
  FW_COMPAT_OK        = 1,
  FW_COMPAT_WARN      = 2,  // Legacy protocol, limited features
  FW_COMPAT_FAIL      = 3   // Known-incompatible combination
};

inline const char* fwCompatName(FwCompatLevel level) {
  switch (level) {
    case FW_COMPAT_OK:   return "OK";
    case FW_COMPAT_WARN: return "WARN";
    case FW_COMPAT_FAIL: return "FAIL";
    default:             return "UNKNOWN";
  }
}

// Decode firmware version from 0x392 mux 0
// Bytes 1-2: year (big-endian uint16), bytes 3: release, byte 4: minor
inline void decodeFwVersion(const Frame& f, State& s) {
  if (f.dlc < 5) return;
  uint8_t mux = f.data[0] & 0x07;
  if (mux == 0) {
    s.fwYear    = ((uint16_t)f.data[1] << 8) | f.data[2];
    s.fwRelease = f.data[3];
    s.fwMinor   = f.data[4];
    s.hasFwVersion = true;

    // Evaluate compatibility
    if (s.fwYear == 2026 && s.fwRelease >= 8) {
      // FSD v13 legacy protocol — limited on HW4
      s.fwCompat = (s.variant == HW4) ? FW_COMPAT_WARN : FW_COMPAT_OK;
    } else if (s.fwYear == 2026 && s.fwRelease == 2 && s.fwMinor >= 9) {
      // FSD v14 — OK for HW4
      s.fwCompat = FW_COMPAT_OK;
    } else if (s.fwYear >= 2026) {
      s.fwCompat = FW_COMPAT_OK;
    } else {
      s.fwCompat = FW_COMPAT_WARN;
    }
  } else if (mux == 1) {
    s.fwBuild = ((uint32_t)f.data[1] << 24) | ((uint32_t)f.data[2] << 16) |
                ((uint32_t)f.data[3] << 8)  | f.data[4];
  }
}

// Query command
inline bool execFwCompatCmd(const char* cmd, State& s) {
  return strcmp(cmd, "fwcompat") == 0;
}
