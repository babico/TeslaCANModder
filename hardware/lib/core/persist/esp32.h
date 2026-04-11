#pragma once
#include <Preferences.h>
#include "core/types.h"

// ── NVS Persistence (ESP32 replacement for AVR EEPROM) ──────────────────────
// Uses the ESP32 Preferences library (NVS flash) instead of EEPROM.
// Namespace: "tcm" (TeslaCANModder)

#define NVS_NAMESPACE "tcm"
#define NVS_KEY_MAGIC "magic"
#define NVS_KEY_VERSION "ver"
#define NVS_SETTINGS_MAGIC 0xCA
#define NVS_SETTINGS_VERSION 0x03

static Preferences prefs;

inline bool loadSettings(State& s) {
  prefs.begin(NVS_NAMESPACE, true);  // read-only
  uint8_t magic = prefs.getUChar(NVS_KEY_MAGIC, 0);
  uint8_t ver   = prefs.getUChar(NVS_KEY_VERSION, 0);
  if (magic != NVS_SETTINGS_MAGIC || ver != NVS_SETTINGS_VERSION) {
    prefs.end();
    return false;
  }

  s.variant          = (Variant)prefs.getUChar("variant", 0);
  s.fsdEnabled       = prefs.getUChar("fsd", 0);
  s.nagSuppress      = prefs.getUChar("nag", 0);
  s.speedProfile     = prefs.getUChar("sp", 1);
  s.profileOverride  = prefs.getUChar("spPin", 0);
  s.speedOffset      = prefs.getUChar("offset", 0);
  s.offsetOverride   = prefs.getUChar("offPin", 0);
  s.isaChimeSuppress = prefs.getUChar("isa", 0);
  s.summonInject     = prefs.getUChar("sumInj", 0);
  prefs.end();
  return true;
}

inline void saveSettings(const State& s) {
  prefs.begin(NVS_NAMESPACE, false);  // read-write
  prefs.putUChar(NVS_KEY_MAGIC, NVS_SETTINGS_MAGIC);
  prefs.putUChar(NVS_KEY_VERSION, NVS_SETTINGS_VERSION);
  prefs.putUChar("variant", (uint8_t)s.variant);
  prefs.putUChar("fsd", s.fsdEnabled ? 1 : 0);
  prefs.putUChar("nag", s.nagSuppress ? 1 : 0);
  prefs.putUChar("sp", (uint8_t)s.speedProfile);
  prefs.putUChar("spPin", s.profileOverride ? 1 : 0);
  prefs.putUChar("offset", (uint8_t)s.speedOffset);
  prefs.putUChar("offPin", s.offsetOverride ? 1 : 0);
  prefs.putUChar("isa", s.isaChimeSuppress ? 1 : 0);
  prefs.putUChar("sumInj", s.summonInject ? 1 : 0);
  prefs.end();
}
