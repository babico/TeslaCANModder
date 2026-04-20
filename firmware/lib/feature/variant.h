#pragma once
#include "core/forward.h"

// ── Variant Selection Command ────────────────────────────────────────────────
bool executeVariantCmd(const char* cmd, State& s) {
  if (strncmp(cmd, "variant:", 8) == 0) {
    const char* val = cmd + 8;
    // variant:auto — enable auto-detection from CAN 0x398
    if (strcmp(val, "auto") == 0) {
      s.variantAutoDetect = true;
      saveSettings(s);
      applyFilters(s);
      return true;
    }
    Variant v;
    if (!parseVariant(val, v)) return false;
    s.variant = v;
    s.variantAutoDetect = false; // manual override disables auto-detect
    saveSettings(s);
    applyFilters(s);
    return true;
  }
  return false;
}

