#pragma once
#include <string.h>
#include "core/types.h"
#include "command/system.h"

// Forward declarations
void saveSettings(const State& s);
void resetHandlerLogFlags();

// ── Nag Killer Command ───────────────────────────────────────────────────────
// Enables/disables EPAS torque spoofing (hands-free driving).
// More aggressive than nag bit-19 suppress — echoes 0x370 with zeroed torque.

bool executeNagKillerCmd(const char* cmd, State& s) {
  if (strncmp(cmd, "nagkiller:", 10) == 0) {
    if (!parseBoolCmd(cmd + 10, s.nagKillerEnabled, s.nagKillerEnabled)) return false;
    resetHandlerLogFlags();
    saveSettings(s);
    return true;
  }
  return false;
}
