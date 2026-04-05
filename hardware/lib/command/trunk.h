#pragma once
#include <string.h>
#include "core/types.h"
#include "protocol/trunk.h"

// ── Trunk Command Execution ──────────────────────────────────────────────────

static bool execTrunkCmd(const char* cmd, State& s) {
  if (strcmp(cmd, "frunk:open") == 0 || strcmp(cmd, "frunk") == 0) {
    return executeTrunkControl(TRUNK_FRUNK, TRUNK_OPEN, s);
  }
  if (strcmp(cmd, "frunk:close") == 0) {
    return executeTrunkControl(TRUNK_FRUNK, TRUNK_CLOSE, s);
  }
  if (strcmp(cmd, "trunk:open") == 0 || strcmp(cmd, "trunk") == 0) {
    return executeTrunkControl(TRUNK_REAR, TRUNK_OPEN, s);
  }
  if (strcmp(cmd, "trunk:close") == 0) {
    return executeTrunkControl(TRUNK_REAR, TRUNK_CLOSE, s);
  }
  if (strcmp(cmd, "glovebox") == 0) {
    return executeTrunkControl(TRUNK_GLOVEBOX, TRUNK_OPEN, s);
  }
  return false;
}
