#pragma once
#include "core/types.h"

// Forward declarations (defined by platform-specific dispatch & driver)
void applyFilters(State& s);

// ── System Command Execution ─────────────────────────────────────────────────
// Handles system-level commands: ping, status, stream, can:raw

static bool parseBoolCmd(const char* suffix, bool current, bool& out) {
  if (strcmp(suffix, "on") == 0)     { out = true;     return true; }
  if (strcmp(suffix, "off") == 0)    { out = false;    return true; }
  if (strcmp(suffix, "toggle") == 0) { out = !current; return true; }
  return false;
}

enum SystemCmdType {
  SYS_PING,
  SYS_STATUS,
  SYS_STREAM,
  SYS_CAN_RAW,
  SYS_UNKNOWN
};

SystemCmdType parseSystemCmd(const char* cmd) {
  if (strcmp(cmd, "ping") == 0) return SYS_PING;
  if (strcmp(cmd, "status") == 0) return SYS_STATUS;
  if (strncmp(cmd, "stream:", 7) == 0) return SYS_STREAM;
  if (strncmp(cmd, "can:raw:", 8) == 0) return SYS_CAN_RAW;
  return SYS_UNKNOWN;
}

bool executeStreamCmd(const char* cmd, State& s) {
  if (strncmp(cmd, "stream:", 7) == 0) {
    bool prev = s.streamEnabled;
    if (!parseBoolCmd(cmd + 7, s.streamEnabled, s.streamEnabled)) return false;
    if (s.streamEnabled && !prev) s.streamCount = 0;  // reset seq on fresh enable
    return true;
  }
  return false;
}

bool executeCanRawCmd(const char* cmd, State& s) {
  if (strncmp(cmd, "can:raw:", 8) == 0) {
    if (!parseBoolCmd(cmd + 8, s.rawCanListen, s.rawCanListen)) return false;
    applyFilters(s);
    return true;
  }
  return false;
}
