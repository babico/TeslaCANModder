#pragma once
#include <cstring>
#include "core/types.h"
#include "protocol/can.h"
void driverSend(const Frame& f, uint8_t bus);

// ── Battery Preconditioning Trigger ──────────────────────────────────────────
// Injects CAN 0x082 (UI_tripPlanning) to trigger battery heating for charging.
// Needs periodic injection at 500ms intervals while active.
// Sources: tuncasoftbildik, hypery11-flipper.

static void controlPrecondition(bool enable, State& s) {
  (void)s;
  Frame f;
  f.id = CAN_ID_PRECONDITION;
  f.dlc = 8;
  memset(f.data, 0, 8);
  f.data[0] = enable ? 0x05 : 0x00;
  driverSend(f, BUS_VEHICLE);
}
