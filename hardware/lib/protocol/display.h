#pragma once
#include "core/types.h"
#include "protocol/can.h"
#include "protocol/vehicle.h"
#include "core/driver.h"

// ── Display Control (0x273) ──────────────────────────────────────────────────

static void controlDisplayBrightness(uint8_t level, State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setDisplayBrightness(f, level);
  
  for (uint8_t i = 0; i < 20; i++) {
#if BOARD_ENABLE_MCP2515_2
    driverSend(f, s.ctrlBus);
#else
    driverSend(f);
#endif
    delay(20);
  }
}
