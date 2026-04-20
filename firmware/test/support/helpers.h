#pragma once
#include "core/types.h"

// ── Shared test helpers ─────────────────────────────────────────────────────

static Frame makeFrame(uint32_t id, uint8_t dlc = 8) {
  Frame f = {};
  f.id = id;
  f.dlc = dlc;
  return f;
}
