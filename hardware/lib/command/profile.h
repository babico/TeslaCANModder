#pragma once
#include "core/types.h"

// Forward declarations
void saveSettings(const State& s);
void resetHandlerLogFlags();

// ── Speed Profile Command (profile:N, sp:N, profile:auto) ───────────────────
bool executeProfileCmd(const char* cmd, State& s) {
  const char* arg = nullptr;
  if (strncmp(cmd, "profile:", 8) == 0) arg = cmd + 8;
  else if (strncmp(cmd, "sp:", 3) == 0) arg = cmd + 3;
  else return false;

  if (strcmp(arg, "auto") == 0) {
    s.profileOverride = false;
    resetHandlerLogFlags();
    saveSettings(s);
    return true;
  }
  int p = atoi(arg);
  if (p < 0 || p > 4) return false;
  s.speedProfile = p;
  s.profileOverride = true;
  resetHandlerLogFlags();
  saveSettings(s);
  return true;
}
