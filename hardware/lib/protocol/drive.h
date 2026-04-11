#pragma once
#include "core/types.h"
#include "protocol/can.h"
void driverSend(const Frame& f, uint8_t bus = 0);

// ── Drive Configuration (0x334) ──────────────────────────────────────────────
// Pedal response, regen level, and stop mode control

enum PedalMode {
  PEDAL_STANDARD = 0,
  PEDAL_CHILL = 1,
  PEDAL_SPORT = 2
};

enum StopMode {
  STOP_CREEP = 0,
  STOP_ROLL = 1,
  STOP_HOLD = 2
};

// Simple checksum calculation for 0x334
static uint8_t calculate334Checksum(const uint8_t* data, uint8_t len) {
  uint8_t sum = 0;
  for (uint8_t i = 0; i < len - 1; i++) {
    sum += data[i];
  }
  return sum & 0xFF;
}

static void controlPedalMode(PedalMode mode, const uint8_t* lastDrive, State& s) {
  Frame f;
  f.id = CAN_ID_DRIVE_CONFIG;
  f.dlc = 8;
  memcpy(f.data, lastDrive, 8);
  
  // Modify byte 0, bits 5-6 (mask 0x60)
  uint8_t value = 0;
  switch (mode) {
    case PEDAL_CHILL: value = 0x20; break;
    case PEDAL_SPORT: value = 0x40; break;
    case PEDAL_STANDARD: value = 0x00; break;
  }
  f.data[0] = (f.data[0] & ~0x60) | value;
  
  // Update checksum (last byte)
  f.data[7] = calculate334Checksum(f.data, 8);
  
  // Send 30 times over 600ms
  for (uint8_t i = 0; i < 30; i++) {
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}

static void controlRegenLevel(uint8_t level, const uint8_t* lastDrive, State& s) {
  Frame f;
  f.id = CAN_ID_DRIVE_CONFIG;
  f.dlc = 8;
  memcpy(f.data, lastDrive, 8);
  
  // Regen level in byte 2 (0-200)
  if (level > 200) level = 200;
  f.data[2] = level;
  
  // Update checksum
  f.data[7] = calculate334Checksum(f.data, 8);
  
  // Send 30 times over 600ms
  for (uint8_t i = 0; i < 30; i++) {
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}

static void controlStopMode(StopMode mode, const uint8_t* lastDrive, State& s) {
  Frame f;
  f.id = CAN_ID_DRIVE_CONFIG;
  f.dlc = 8;
  memcpy(f.data, lastDrive, 8);
  
  // Modify byte 5, bits 0-1 (mask 0x03)
  f.data[5] = (f.data[5] & ~0x03) | (mode & 0x03);
  
  // Update checksum
  f.data[7] = calculate334Checksum(f.data, 8);
  
  // Send 30 times over 600ms
  for (uint8_t i = 0; i < 30; i++) {
    driverSend(f, BUS_VEHICLE);
    delay(20);
  }
}
