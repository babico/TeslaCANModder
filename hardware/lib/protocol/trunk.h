#pragma once
#include "core/types.h"
#include "protocol/can.h"
void driverSend(const Frame& f, uint8_t bus = 0);

// ── Trunk/Frunk Control ──────────────────────────────────────────────────────
// Unified control for all trunk/frunk operations

enum TrunkTarget {
  TRUNK_FRUNK = 0,      // Front trunk (uses 0x273)
  TRUNK_REAR = 1,       // Rear trunk (uses 0x3B3)
  TRUNK_GLOVEBOX = 2    // Glovebox (uses 0x3B3)
};

enum TrunkAction {
  TRUNK_OPEN = 0,
  TRUNK_CLOSE = 1
};

// Send frunk open command (0x273 bit 5)
static void controlFrunk(const uint8_t* lastCtrl, bool open, State& s) {
  Frame f;
  f.id = CAN_ID_UI_VEHICLE_CTRL;
  f.dlc = 8;
  memcpy(f.data, lastCtrl, 8);
  
  if (open) {
    f.data[0] |= 0x20;  // Set bit 5
  } else {
    f.data[0] &= ~0x20; // Clear bit 5
  }
  
  // Send 20 times over 400ms
  for (uint8_t i = 0; i < 20; i++) {
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}

// Send trunk open/close command (0x3B3)
static void controlTrunk(bool open) {
  Frame f;
  f.id = CAN_ID_TRUNK_CTRL;
  f.dlc = 4;
  f.data[0] = open ? 0x02 : 0x03;  // 0x02 = open, 0x03 = close
  f.data[1] = 0x00;
  f.data[2] = 0x00;
  f.data[3] = 0x00;
  
  // Send 20 times over 2 seconds
  for (uint8_t i = 0; i < 20; i++) {
    driverSend(f, BUS_BODY);
    delay(100);
  }
}

// Send glovebox open command (0x3B3 - glovebox doesn't have close)
static void controlGlovebox() {
  Frame f;
  f.id = CAN_ID_TRUNK_CTRL;
  f.dlc = 4;
  f.data[0] = 0x01;  // Glovebox command (open only)
  f.data[1] = 0x00;
  f.data[2] = 0x00;
  f.data[3] = 0x00;
  
  // Send 20 times over 2 seconds
  for (uint8_t i = 0; i < 20; i++) {
    driverSend(f, BUS_BODY);
    delay(100);
  }
}

// Unified trunk control interface
static bool executeTrunkControl(TrunkTarget target, TrunkAction action, State& s) {
  switch (target) {
    case TRUNK_FRUNK:
      if (!s.hasCtrl) return false;  // Need 0x273 frame cached
      controlFrunk(s.lastCtrl, action == TRUNK_OPEN, s);
      return true;
      
    case TRUNK_REAR:
      controlTrunk(action == TRUNK_OPEN);
      return true;
      
    case TRUNK_GLOVEBOX:
      if (action == TRUNK_CLOSE) return false;  // Glovebox can't close via CAN
      controlGlovebox();
      return true;
      
    default:
      return false;
  }
}
