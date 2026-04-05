#pragma once
#include "core/types.h"
#include "protocol/drive.h"

// ── Drive Command Execution ─────────────────────────────────────────────────
// Pedal mode, regen level, and stop mode configuration

static bool execDriveCmd(const char* cmd, State& s) {
  if (s.variant == LEGACY) return false;
  if (!s.hasDrive) return false;  // Need 0x334 frame cached

  // Pedal mode
  if (strcmp(cmd, "pedal:standard") == 0 || strcmp(cmd, "pedal:std") == 0) {
    controlPedalMode(PEDAL_STANDARD, s.lastDrive, s);
    return true;
  }
  if (strcmp(cmd, "pedal:chill") == 0) {
    controlPedalMode(PEDAL_CHILL, s.lastDrive, s);
    return true;
  }
  if (strcmp(cmd, "pedal:sport") == 0) {
    controlPedalMode(PEDAL_SPORT, s.lastDrive, s);
    return true;
  }

  // Regen level
  if (strcmp(cmd, "regen:off") == 0) {
    controlRegenLevel(0, s.lastDrive, s);
    return true;
  }
  if (strcmp(cmd, "regen:low") == 0) {
    controlRegenLevel(50, s.lastDrive, s);
    return true;
  }
  if (strcmp(cmd, "regen:standard") == 0 || strcmp(cmd, "regen:std") == 0) {
    controlRegenLevel(100, s.lastDrive, s);
    return true;
  }
  if (strcmp(cmd, "regen:max") == 0) {
    controlRegenLevel(200, s.lastDrive, s);
    return true;
  }

  // Stop mode
  if (strcmp(cmd, "stop:creep") == 0) {
    controlStopMode(STOP_CREEP, s.lastDrive, s);
    return true;
  }
  if (strcmp(cmd, "stop:roll") == 0) {
    controlStopMode(STOP_ROLL, s.lastDrive, s);
    return true;
  }
  if (strcmp(cmd, "stop:hold") == 0) {
    controlStopMode(STOP_HOLD, s.lastDrive, s);
    return true;
  }

  return false;
}
