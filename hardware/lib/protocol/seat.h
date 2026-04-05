#pragma once
#include "core/types.h"
#include "protocol/can.h"
#include "protocol/vehicle.h"
#include "core/driver.h"

// ── Seat Heating Control (0x273) ─────────────────────────────────────────────

static void controlSeatHeat(uint8_t seat, SeatHeatLevel level, State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  
  if (seat == 0) setSeatHeatFL(f, level);
  else if (seat == 1) setSeatHeatFR(f, level);
  else if (seat == 2) setSeatHeatRL(f, level);
  else if (seat == 3) setSeatHeatRR(f, level);
  else if (seat == 4) setSeatHeatRC(f, level);
  
  for (uint8_t i = 0; i < 30; i++) {
#if BOARD_ENABLE_MCP2515_2
    driverSend(f, s.ctrlBus);
#else
    driverSend(f);
#endif
    delay(20);
  }
}
