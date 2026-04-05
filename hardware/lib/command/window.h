#pragma once
#include "core/types.h"
#include "protocol/window.h"

// ── Window Command Execution ────────────────────────────────────────────────

static bool execWindowCmd(const char* cmd, State& s) {
  if (s.variant == LEGACY) return false;
  
  if (strcmp(cmd, "window:vent:open") == 0 || strcmp(cmd, "vent:open") == 0) {
    controlWindowVent(WINDOW_VENT_OPEN, s);
    return true;
  }
  if (strcmp(cmd, "window:vent:close") == 0 || strcmp(cmd, "vent:close") == 0) {
    controlWindowVent(WINDOW_VENT_CLOSE, s);
    return true;
  }
  return false;
}
