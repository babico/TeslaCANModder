#pragma once
#include <string.h>
#include "core/types.h"
#include "protocol/seat.h"

// ── Seat Command Execution ───────────────────────────────────────────────────

static bool execSeatCmd(const char* cmd, State& s) {
  if (!s.hasCtrl) return false;
  if (strncmp(cmd, "seat:", 5) != 0) return false;
  
  const char* pos = cmd + 5;
  char lastChar = cmd[strlen(cmd) - 1];
  if (lastChar < '0' || lastChar > '3') return false;
  
  SeatHeatLevel level = (SeatHeatLevel)(lastChar - '0');
  uint8_t seat = 255;
  
  if (strncmp(pos, "fl:", 3) == 0) seat = 0;
  else if (strncmp(pos, "fr:", 3) == 0) seat = 1;
  else if (strncmp(pos, "rl:", 3) == 0) seat = 2;
  else if (strncmp(pos, "rr:", 3) == 0) seat = 3;
  else if (strncmp(pos, "rc:", 3) == 0) seat = 4;
  else return false;
  
  controlSeatHeat(seat, level, s);
  return true;
}
