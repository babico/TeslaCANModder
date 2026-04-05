#pragma once
#include "core/types.h"
#include "core/eeprom.h"
#include "handler/dispatch.h"

// ── FSD Command Execution ────────────────────────────────────────────────────
// Handles all FSD-related commands: fsd, nag, profile, offset, isa-chime, summon

static bool parseBoolCmd(const char* suffix, bool current, bool& out) {
  if (strcmp(suffix, "on") == 0)     { out = true;     return true; }
  if (strcmp(suffix, "off") == 0)    { out = false;    return true; }
  if (strcmp(suffix, "toggle") == 0) { out = !current; return true; }
  return false;
}

bool executeFsdCmd(const char* cmd, State& s) {
  if (strncmp(cmd, "fsd:", 4) == 0) {
    if (!parseBoolCmd(cmd + 4, s.fsdEnabled, s.fsdEnabled)) return false;
    saveSettings(s);
    return true;
  }
  return false;
}

bool executeNagCmd(const char* cmd, State& s) {
  if (strncmp(cmd, "nag:", 4) == 0) {
    if (!parseBoolCmd(cmd + 4, s.nagSuppress, s.nagSuppress)) return false;
    saveSettings(s);
    return true;
  }
  return false;
}

bool executeProfileCmd(const char* cmd, State& s) {
  const char* arg = nullptr;
  if (strncmp(cmd, "profile:", 8) == 0) arg = cmd + 8;
  else if (strncmp(cmd, "sp:", 3) == 0) arg = cmd + 3;
  else return false;

  if (strcmp(arg, "auto") == 0) {
    s.profileOverride = false;
    saveSettings(s);
    return true;
  }
  int p = atoi(arg);
  if (p < 0 || p > 4) return false;
  s.speedProfile = p;
  s.profileOverride = true;
  saveSettings(s);
  return true;
}

bool executeOffsetCmd(const char* cmd, State& s) {
  if (strncmp(cmd, "offset:", 7) != 0) return false;
  if (!s.features().speedOffset) return false;
  const char* arg = cmd + 7;

  if (strcmp(arg, "auto") == 0) {
    s.offsetOverride = false;
    saveSettings(s);
    return true;
  }
  int o = atoi(arg);
  if (o < 0 || o > 100) return false;
  s.speedOffset = o;
  s.offsetOverride = true;
  saveSettings(s);
  return true;
}

bool executeIsaChimeCmd(const char* cmd, State& s) {
  if (strncmp(cmd, "isa-chime:", 10) == 0) {
    if (!s.features().isaChime) return false;
    if (!parseBoolCmd(cmd + 10, s.isaChimeSuppress, s.isaChimeSuppress)) return false;
    saveSettings(s);
    return true;
  }
  return false;
}

bool executeSummonCmd(const char* cmd, State& s) {
  if (strcmp(cmd, "summon") == 0 || strncmp(cmd, "summon:", 7) == 0) {
    if (!s.features().summon) return false;
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
  
  if (strcmp(cmd, "summon:stop") == 0) {
    if (!s.features().summon) return false;
    s.summonMode = SUMMON_STOP;
    s.summonRemaining = 0;
    return true;
  }
  
  return false;
}

bool executeVariantCmd(const char* cmd, State& s) {
  if (strncmp(cmd, "variant:", 8) == 0) {
    Variant v;
    if (!parseVariant(cmd + 8, v)) return false;
    s.variant = v;
    saveSettings(s);
    applyFilters(s);
    return true;
  }
  return false;
}
