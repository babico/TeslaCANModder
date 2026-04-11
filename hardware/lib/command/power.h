#pragma once
#include <string.h>
#include "core/types.h"
#include "protocol/power.h"

// ── Power Command Execution ──────────────────────────────────────────────────

static bool execPowerCmd(const char* cmd, State& s) {
  if (!s.hasCtrl) return false;
  
  if (strcmp(cmd, "power:acc:on") == 0) {
    controlAccessoryPower(true, s);
    return true;
  }
  if (strcmp(cmd, "power:acc:off") == 0) {
    controlAccessoryPower(false, s);
    return true;
  }
  if (strcmp(cmd, "power:ready") == 0) {
    controlDriveState(s);
    return true;
  }
  if (strcmp(cmd, "power:off") == 0) {
    controlPowerOff(s);
    return true;
  }
  return false;
}
