#pragma once
#include "core/types.h"
#include "protocol/can.h"
void driverSend(const Frame& f, uint8_t bus);

// ── Seat Heating Bit Helpers (0x273 UI_vehicleControl) ───────────────────────
enum SeatHeatLevel { SEAT_OFF = 0, SEAT_LOW = 1, SEAT_MED = 2, SEAT_HIGH = 3 };

inline void setSeatHeatFL(Frame& f, SeatHeatLevel level) {
  if (f.dlc < 6) return;
  f.data[5] = (f.data[5] & ~0x0C) | ((level & 0x03) << 2);  // bits 42-43
}

inline void setSeatHeatFR(Frame& f, SeatHeatLevel level) {
  if (f.dlc < 6) return;
  f.data[5] = (f.data[5] & ~0x30) | ((level & 0x03) << 4);  // bits 44-45
}

inline void setSeatHeatRL(Frame& f, SeatHeatLevel level) {
  if (f.dlc < 6) return;
  f.data[5] = (f.data[5] & ~0xC0) | ((level & 0x03) << 6);  // bits 46-47
}

inline void setSeatHeatRR(Frame& f, SeatHeatLevel level) {
  if (f.dlc < 7) return;
  f.data[6] = (f.data[6] & ~0x0C) | ((level & 0x03) << 2);  // bits 50-51
}

inline void setSeatHeatRC(Frame& f, SeatHeatLevel level) {
  if (f.dlc < 7) return;
  f.data[6] = (f.data[6] & ~0x03) | (level & 0x03);  // bits 48-49
}

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
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}
