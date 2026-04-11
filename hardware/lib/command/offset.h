#pragma once
#include "core/types.h"

// Forward declarations
void saveSettings(const State& s);
void resetHandlerLogFlags();

// ── Speed Offset Command (offset:N, offset:auto) ────────────────────────────
bool executeOffsetCmd(const char* cmd, State& s) {
  if (strncmp(cmd, "offset:", 7) != 0) return false;
  if (!s.features().speedOffset) return false;
  const char* arg = cmd + 7;

  if (strcmp(arg, "auto") == 0) {
    s.offsetOverride = false;
    resetHandlerLogFlags();
    saveSettings(s);
    return true;
  }
  int o = atoi(arg);
  if (o < 0 || o > 100) return false;
  s.speedOffset = o;
  s.offsetOverride = true;
  resetHandlerLogFlags();
  saveSettings(s);
  return true;
}
