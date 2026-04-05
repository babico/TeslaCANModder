#pragma once
#include "core/types.h"
#include "command/mirror.h"
#include "command/lock.h"
#include "command/trunk.h"
#include "command/light.h"
#include "command/wiper.h"
#include "command/seat.h"
#include "command/display.h"
#include "command/power.h"

// ── Main Vehicle Command Router ──────────────────────────────────────────────

bool executeVehicleCmd(const char* cmd, State& s) {
  if (s.variant == LEGACY) return false;
  
  if (execMirrorCmd(cmd, s)) return true;
  if (execLockCmd(cmd, s)) return true;
  if (execTrunkCmd(cmd, s)) return true;
  if (execLightCmd(cmd, s)) return true;
  if (execWiperCmd(cmd, s)) return true;
  if (execSeatCmd(cmd, s)) return true;
  if (execDisplayCmd(cmd, s)) return true;
  if (execPowerCmd(cmd, s)) return true;
  return false;
}
