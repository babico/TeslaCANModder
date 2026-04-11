#pragma once
#include "core/types.h"

// Forward declarations
void saveSettings(const State& s);
void resetHandlerLogFlags();

// ── Summon Injection Enable/Disable Command ──────────────────────────────────
// Controls whether summon injection is allowed. Persisted to EEPROM/NVS.
bool executeSummonInjectCmd(const char* cmd, State& s) {
  if (strncmp(cmd, "summon-inject:", 14) == 0) {
    if (!s.features().summon) return false;
    if (!parseBoolCmd(cmd + 14, s.summonInject, s.summonInject)) return false;
    // If injection is disabled, stop any active burst
    if (!s.summonInject) {
      s.summonMode = SUMMON_STOP;
      s.summonRemaining = 0;
    }
    resetHandlerLogFlags();
    saveSettings(s);
    return true;
  }
  return false;
}
