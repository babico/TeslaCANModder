#pragma once
#include "core/types.h"
#include "protocol/can.h"
void driverSend(const Frame& f, uint8_t bus);

// ── Climate Control (0x2F3) ──────────────────────────────────────────────────
// Climate keeper mode control (bits 33-34)

enum ClimateMode {
  CLIMATE_OFF = 0,
  CLIMATE_KEEP = 1
};

static void controlClimate(ClimateMode mode, const uint8_t* lastClimate, State& s) {
  Frame f;
  f.id = CAN_ID_CLIMATE;
  f.dlc = 5;
  memcpy(f.data, lastClimate, 5);
  
  // Modify bits 33-34 (byte 4, bits 1-2)
  f.data[4] = (f.data[4] & ~0x06) | ((mode & 0x03) << 1);
  
  // Send 30 times over 600ms
  for (uint8_t i = 0; i < 30; i++) {
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}
