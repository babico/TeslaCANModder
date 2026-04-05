#pragma once
#include "core/types.h"
#include "protocol/vehicle.h"
#include "core/driver.h"

// ── Vehicle Control Protocol (0x273) ─────────────────────────────────────────

static void controlVehicle(const uint8_t* lastCtrl, void (*modifier)(Frame&), State& s, uint8_t count, uint8_t delayMs) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, lastCtrl, 8);
  modifier(f);
  
  for (uint8_t i = 0; i < count; i++) {
#if BOARD_ENABLE_MCP2515_2
    driverSend(f, s.ctrlBus);
#else
    driverSend(f);
#endif
    delay(delayMs);
  }
}
