#pragma once
#include "core/types.h"

// Forward declarations
void saveSettings(const State& s);
void resetHandlerLogFlags();

// ── ISA Speed Chime Suppress Command ─────────────────────────────────────────
bool executeIsaChimeCmd(const char* cmd, State& s) {
  if (strncmp(cmd, "isa-chime:", 10) == 0) {
    if (!s.features().isaChime) return false;
    if (!parseBoolCmd(cmd + 10, s.isaChimeSuppress, s.isaChimeSuppress)) return false;
    resetHandlerLogFlags();
    saveSettings(s);
    return true;
  }
  return false;
}
