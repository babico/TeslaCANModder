#pragma once
#include <cstring>
#include <Arduino.h>
#include "core/types.h"
#include "command/system.h"
#include "protocol/precondition.h"

// Forward declarations
void saveSettings(const State& s);

// ── Preconditioning Command ──────────────────────────────────────────────────
// Enables/disables battery preconditioning for supercharging.

bool execPreconditionCmd(const char* cmd, State& s) {
  if (strncmp(cmd, "precondition:", 13) == 0) {
    if (!parseBoolCmd(cmd + 13, s.preconditionEnabled, s.preconditionEnabled)) return false;
    if (s.preconditionEnabled) {
      controlPrecondition(true, s);
      s.precondLastMs = millis();
    } else {
      controlPrecondition(false, s);
    }
    saveSettings(s);
    return true;
  }
  return false;
}
