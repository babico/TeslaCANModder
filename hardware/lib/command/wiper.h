#pragma once
#include <string.h>
#include "core/types.h"
#include "protocol/wiper.h"

// ── Wiper Command Execution ──────────────────────────────────────────────────

static bool execWiperCmd(const char* cmd, State& s) {
  if (!s.hasCtrl) return false;
  
  if (strcmp(cmd, "wiper:off") == 0) {
    controlWiper(WIPER_OFF, s);
    return true;
  }
  if (strcmp(cmd, "wiper:1") == 0) {
    controlWiper(WIPER_1, s);
    return true;
  }
  if (strcmp(cmd, "wiper:2") == 0) {
    controlWiper(WIPER_2, s);
    return true;
  }
  if (strcmp(cmd, "wiper:3") == 0) {
    controlWiper(WIPER_3, s);
    return true;
  }
  return false;
}
