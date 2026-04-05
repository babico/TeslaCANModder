#pragma once
#include <string.h>
#include "core/types.h"
#include "protocol/power.h"

// ── Power Command Execution ──────────────────────────────────────────────────

static void controlPowerOff(State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setPowerOff(f, true);

  for (uint8_t i = 0; i < 30; i++) {
#if BOARD_ENABLE_MCP2515_2
    driverSend(f, s.ctrlBus);
#else
    driverSend(f);
#endif
    delay(20);
  }
}

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
