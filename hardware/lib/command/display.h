#pragma once
#include <string.h>
#include <stdlib.h>
#include "core/types.h"
#include "protocol/display.h"

// ── Display Command Execution ────────────────────────────────────────────────

static bool execDisplayCmd(const char* cmd, State& s) {
  if (!s.hasCtrl) return false;
  if (strncmp(cmd, "maindisplay:", 12) != 0) return false;
  
  int level = atoi(cmd + 12);
  if (level < 0 || level > 127) return false;
  
  controlDisplayBrightness((uint8_t)level, s);
  return true;
}
