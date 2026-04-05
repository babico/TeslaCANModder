#pragma once
#include "core/types.h"
#include "protocol/can.h"
#include "protocol/vehicle.h"
#include "core/driver.h"

// ── Power Control (0x273) ────────────────────────────────────────────────────

static void controlAccessoryPower(bool enable, State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setAccessoryPower(f, enable);
  
  for (uint8_t i = 0; i < 30; i++) {
#if BOARD_ENABLE_MCP2515_2
    driverSend(f, s.ctrlBus);
#else
    driverSend(f);
#endif
    delay(20);
  }
}

static void controlDriveState(State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setDriveStateRequest(f, true);
  
  for (uint8_t i = 0; i < 30; i++) {
#if BOARD_ENABLE_MCP2515_2
    driverSend(f, s.ctrlBus);
#else
    driverSend(f);
#endif
    delay(20);
  }
}
