#pragma once
#include "core/types.h"

// Forward declarations
void saveSettings(const State& s);

// ── Summon Command (summon, summon:forward, summon:reverse, summon:stop) ─────
// Requires summonInject to be enabled (except for stop, which always works).
bool executeSummonCmd(const char* cmd, State& s) {
  if (strcmp(cmd, "summon:stop") == 0) {
    if (!s.features().summon) return false;
    s.summonMode = SUMMON_STOP;
    s.summonRemaining = 0;
    return true;
  }
  
  if (strcmp(cmd, "summon") == 0 || strncmp(cmd, "summon:", 7) == 0) {
    if (!s.features().summon) return false;
    if (!s.summonInject) return false;
    if (!s.hasCtrl) return false;
    
    if (strncmp(cmd, "summon:", 7) == 0) {
      const char* dir = cmd + 7;
      if (strcmp(dir, "forward") == 0 || strcmp(dir, "fwd") == 0) {
        s.summonDirection = SUMMON_FORWARD;
      } else if (strcmp(dir, "reverse") == 0 || strcmp(dir, "rev") == 0) {
        s.summonDirection = SUMMON_REVERSE;
      } else {
        return false;
      }
    }
    
    s.summonMode = SUMMON_START;
    s.summonRemaining = 30;
    return true;
  }
  
  return false;
}
