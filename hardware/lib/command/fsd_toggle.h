#pragma once
#include "core/types.h"

// Forward declarations
void saveSettings(const State& s);
void resetHandlerLogFlags();

// ── FSD Enable/Disable Command ───────────────────────────────────────────────
bool executeFsdCmd(const char* cmd, State& s) {
  if (strncmp(cmd, "fsd:", 4) == 0) {
    if (!parseBoolCmd(cmd + 4, s.fsdEnabled, s.fsdEnabled)) return false;
    resetHandlerLogFlags();
    saveSettings(s);
    return true;
  }
  return false;
}
