#pragma once
#include "core/types.h"
#include "protocol/track_mode.h"

// Forward declarations
void saveSettings(const State& s);

// ── Track Mode Command ───────────────────────────────────────────────────────
// Enables/disables Track Mode via CAN 0x313 injection.

bool execTrackModeCmd(const char* cmd, State& s) {
  if (strncmp(cmd, "trackmode:", 10) == 0) {
    if (!parseBoolCmd(cmd + 10, s.trackModeEnabled, s.trackModeEnabled)) return false;
    controlTrackMode(s.trackModeEnabled, s);
    saveSettings(s);
    return true;
  }
  return false;
}
