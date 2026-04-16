#pragma once
#include "core/types.h"

// ── BMS Telemetry Command ────────────────────────────────────────────────────
// Returns current BMS battery data as JSON.

bool execBmsCmd(const char* cmd, State& s) {
  if (strcmp(cmd, "bms") != 0) return false;
  (void)s;  // data is output as JSON by the serial layer
  return true;
}
