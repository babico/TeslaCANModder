#pragma once
#include "core/types.h"
#include "protocol/can.h"
void driverSend(const Frame& f, uint8_t bus);

// ── Window Vent Control (0x119) ──────────────────────────────────────────────
// Controls window vent position

enum WindowVentPosition {
  WINDOW_VENT_CLOSE = 0,
  WINDOW_VENT_OPEN = 100
};

static void controlWindowVent(WindowVentPosition pos, State& s) {
  Frame f;
  f.id = CAN_ID_WINDOW_VENT;
  f.dlc = 2;
  f.data[0] = 0x1F;
  f.data[1] = (uint8_t)pos;
  
  for (uint8_t i = 0; i < 30; i++) {
    driverSend(f, BUS_BODY);
    delay(20);
  }
}
