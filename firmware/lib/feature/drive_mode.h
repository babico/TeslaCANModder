#pragma once
#include "core/forward.h"
#include "infra/can.h"

// ── Drive Mode Override ("Ghost Mode") ───────────────────────────────────────
// Injects CAN ID 0x334 to override drive mode mapping at runtime.
// Motor controller accepts the latest CAN value at 50ms intervals.
//
// Byte 0 Bits 5-6: 00=Chill, 01=Standard, 10=Performance
// Also sets regen torque limit byte for consistent feel.

#define CAN_ID_DRIVE_MODE  0x334
#define DRIVE_MODE_INTERVAL_MS 50

enum DriveMode {
  DRIVE_MODE_NONE = 0,       // No injection (pass-through)
  DRIVE_MODE_CHILL = 1,
  DRIVE_MODE_STANDARD = 2,
  DRIVE_MODE_PERFORMANCE = 3
};

inline const char* driveModeName(uint8_t mode) {
  switch (mode) {
    case DRIVE_MODE_CHILL:       return "chill";
    case DRIVE_MODE_STANDARD:    return "standard";
    case DRIVE_MODE_PERFORMANCE: return "performance";
    default:                     return "none";
  }
}

inline bool parseDriveMode(const char* name, uint8_t& out) {
  if (strcmp(name, "chill") == 0)       { out = DRIVE_MODE_CHILL; return true; }
  if (strcmp(name, "standard") == 0)    { out = DRIVE_MODE_STANDARD; return true; }
  if (strcmp(name, "performance") == 0) { out = DRIVE_MODE_PERFORMANCE; return true; }
  if (strcmp(name, "off") == 0 || strcmp(name, "none") == 0) { out = DRIVE_MODE_NONE; return true; }
  return false;
}

// Build a drive mode injection frame
inline Frame buildDriveModeFrame(uint8_t mode, const uint8_t* lastDrive) {
  Frame f;
  f.id = CAN_ID_DRIVE_MODE;
  f.dlc = 8;
  // Copy base from last cached drive config frame
  for (uint8_t i = 0; i < 8; i++) f.data[i] = lastDrive[i];
  // Set mode bits: byte[0] bits[6:5]
  uint8_t modeBits = 0;
  switch (mode) {
    case DRIVE_MODE_CHILL:       modeBits = 0x00; break;
    case DRIVE_MODE_STANDARD:    modeBits = 0x01; break;
    case DRIVE_MODE_PERFORMANCE: modeBits = 0x02; break;
    default:                     modeBits = 0x01; break;
  }
  f.data[0] = (f.data[0] & ~0x60) | ((modeBits & 0x03) << 5);
  // Recalculate checksum (same as pedal/regen/stop)
  f.data[7] = driveChecksum(f.data, 8);
  return f;
}

// Drive mode tick — call from main loop, injects at DRIVE_MODE_INTERVAL_MS
inline void driveModeTick(State& s, unsigned long now) {
  if (s.driveModeOverride == DRIVE_MODE_NONE) return;
  if (!s.hasDrive) return;
  if (s.txPaused) return;
  if (now - s.driveModeLastMs < DRIVE_MODE_INTERVAL_MS) return;
  s.driveModeLastMs = now;
  Frame f = buildDriveModeFrame(s.driveModeOverride, s.lastDrive);
  driverSend(f, BUS_VEHICLE);
}

// ── Drive Mode Command ───────────────────────────────────────────────────────
bool executeDriveModeCmd(const char* cmd, State& s) {
  if (strncmp(cmd, "drivemode:", 10) != 0) return false;
  const char* arg = cmd + 10;
  uint8_t mode;
  if (!parseDriveMode(arg, mode)) return false;
  s.driveModeOverride = mode;
  s.driveModeLastMs = 0;
  saveSettings(s);
  return true;
}
