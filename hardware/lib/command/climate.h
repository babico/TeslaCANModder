#pragma once
#include "core/types.h"
#include "protocol/climate.h"

// ── Climate Command Execution ───────────────────────────────────────────────

static bool execClimateCmd(const char* cmd, State& s) {
  if (s.variant == LEGACY) return false;
  if (!s.hasClimate) return false;  // Need 0x2F3 frame cached

  if (strcmp(cmd, "climate:keep") == 0) {
    controlClimate(CLIMATE_KEEP, s.lastClimate, s);
    return true;
  }
  if (strcmp(cmd, "climate:off") == 0) {
    controlClimate(CLIMATE_OFF, s.lastClimate, s);
    return true;
  }
  return false;
}
