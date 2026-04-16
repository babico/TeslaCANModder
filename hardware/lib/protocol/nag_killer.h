#pragma once
#include "core/types.h"
#include "protocol/can.h"
void driverSend(const Frame& f, uint8_t bus);

// ── Nag Killer — EPAS Torque Spoofing ────────────────────────────────────────
// Echoes CAN 0x370 (EPAS3P_sysStatus) with counter+1 and zeroed torque request.
// This convinces Autopilot that hands are on the wheel.
// Sources: ev-open-can-tools, hypery11-flipper, tesla-fsd.netlify.app.
//
// Checksum: sum(byte0..byte6) + (CAN_ID & 0xFF) + (CAN_ID >> 8) = sum + 0x73

inline uint8_t nagKillerChecksum(const uint8_t* data) {
  uint16_t sum = 0;
  for (uint8_t i = 0; i < 7; i++) sum += data[i];
  sum += (CAN_ID_EPAS_TORQUE & 0xFF);       // 0x70
  sum += ((CAN_ID_EPAS_TORQUE >> 8) & 0xFF); // 0x03
  return (uint8_t)(sum & 0xFF);
}

inline void nagKillerModify(Frame& f) {
  if (f.dlc < 8) return;
  // Increment counter (byte 1, bits 3:0)
  uint8_t counter = f.data[1] & 0x0F;
  counter = (counter + 1) & 0x0F;
  f.data[1] = (f.data[1] & 0xF0) | counter;
  // Zero the torque request (bytes 2-3)
  f.data[2] = 0x00;
  f.data[3] = 0x00;
  // Recalculate checksum (byte 7)
  f.data[7] = nagKillerChecksum(f.data);
}
