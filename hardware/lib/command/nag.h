#pragma once
#include "core/types.h"

// Forward declarations
void saveSettings(const State& s);
void resetHandlerLogFlags();

// ── Nag Suppress Command ─────────────────────────────────────────────────────
bool executeNagCmd(const char* cmd, State& s) {
  if (strncmp(cmd, "nag:", 4) == 0) {
    if (!parseBoolCmd(cmd + 4, s.nagSuppress, s.nagSuppress)) return false;
    resetHandlerLogFlags();
    saveSettings(s);
    return true;
  }
  return false;
}
