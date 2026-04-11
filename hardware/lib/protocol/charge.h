#pragma once
#include "core/types.h"
#include "protocol/can.h"
void driverSend(const Frame& f, uint8_t bus = 0);

// ── Charge Control (0x333) ───────────────────────────────────────────────────

enum ChargeAction {
  CHARGE_STOP = 0,
  CHARGE_START = 1,
  CHARGE_PORT_OPEN = 2
};

static void controlCharge(ChargeAction action, const uint8_t* lastCharge, State& s) {
  Frame f;
  f.id = CAN_ID_CHARGE;
  f.dlc = 5;
  memcpy(f.data, lastCharge, 5);
  
  switch (action) {
    case CHARGE_START:
      f.data[0] |= 0x04;  // Set bit 2
      break;
    case CHARGE_STOP:
      f.data[0] &= ~0x04; // Clear bit 2
      break;
    case CHARGE_PORT_OPEN:
      f.data[0] |= 0x01;  // Set bit 0
      break;
  }
  
  // Send 20 times over 400ms
  for (uint8_t i = 0; i < 20; i++) {
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}
