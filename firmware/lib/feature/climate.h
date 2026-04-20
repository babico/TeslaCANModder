#pragma once
#include <string.h>
#include "core/forward.h"
#include "infra/can.h"
#include "infra/burst.h"

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
  
  startBurst(s, f, BUS_VEHICLE, 30, 20);
}

// ── Climate Command Execution ───────────────────────────────────────────────

static bool execClimateCmd(const char* cmd, State& s) {
  if (s.variant == LEGACY) return false;
  if (!s.hasClimate) return false;  // Need 0x2F3 frame cached

  if (strcmp(cmd, "climate:keep") == 0) {
    controlClimate(CLIMATE_KEEP, s.lastClimate, s);
    return true;
  }
  if (strcmp(cmd, "climate:off") == 0) {
    controlClimate(CLIMATE_OFF, s.lastClimate, s);
    return true;
  }
  return false;
}

