#pragma once
#include "core/types.h"
#include "protocol/can.h"
#include "protocol/vehicle.h"
#include "core/driver.h"

// ── Lock Control (0x273) ─────────────────────────────────────────────────────

static void controlLock(LockRequest req, State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setLockRequest(f, req);
  
  for (uint8_t i = 0; i < 30; i++) {
#if BOARD_ENABLE_MCP2515_2
    driverSend(f, s.ctrlBus);
#else
    driverSend(f);
#endif
    delay(20);
  }
}

static void controlChildLock(State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setChildDoorLock(f, true);
  
  for (uint8_t i = 0; i < 30; i++) {
#if BOARD_ENABLE_MCP2515_2
    driverSend(f, s.ctrlBus);
#else
    driverSend(f);
#endif
    delay(20);
  }
}

static void controlHorn(State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setHornRequest(f, true);
  
  for (uint8_t i = 0; i < 30; i++) {
#if BOARD_ENABLE_MCP2515_2
    driverSend(f, s.ctrlBus);
#else
    driverSend(f);
#endif
    delay(20);
  }
}
