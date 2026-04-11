#pragma once
#include "core/types.h"

// Forward declarations
void saveSettings(const State& s);
void applyFilters(State& s);

// ── Variant Selection Command ────────────────────────────────────────────────
bool executeVariantCmd(const char* cmd, State& s) {
  if (strncmp(cmd, "variant:", 8) == 0) {
    Variant v;
    if (!parseVariant(cmd + 8, v)) return false;
    s.variant = v;
    saveSettings(s);
    applyFilters(s);
    return true;
  }
  return false;
}
