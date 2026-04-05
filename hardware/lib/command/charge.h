#pragma once
#include "core/types.h"
#include "protocol/charge.h"

// ── Charge Command Execution ────────────────────────────────────────────────

static bool execChargeCmd(const char* cmd, State& s) {
  if (s.variant == LEGACY) return false;
  if (!s.hasCharge) return false;  // Need 0x333 frame cached

  if (strcmp(cmd, "charge:start") == 0) {
    controlCharge(CHARGE_START, s.lastCharge, s);
    return true;
  }
  if (strcmp(cmd, "charge:stop") == 0) {
    controlCharge(CHARGE_STOP, s.lastCharge, s);
    return true;
  }
  if (strcmp(cmd, "charge:port") == 0 || strcmp(cmd, "chargeport") == 0) {
    controlCharge(CHARGE_PORT_OPEN, s.lastCharge, s);
    return true;
  }
  return false;
}
