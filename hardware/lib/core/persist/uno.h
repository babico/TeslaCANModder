#pragma once
#include <EEPROM.h>
#include "core/types.h"

// ── EEPROM Persistence ───────────────────────────────────────────────────────
// Saved settings layout — magic + version guards against stale layouts.
// Bump SETTINGS_VERSION whenever fields are added/removed/reordered.

#define SETTINGS_MAGIC   0xCA
#define SETTINGS_VERSION 0x03
#define SETTINGS_ADDR    0

struct SavedSettings {
  uint8_t magic;
  uint8_t version;
  uint8_t variant;           // 0=HW4, 1=HW3, 2=LEGACY
  uint8_t fsdEnabled;
  uint8_t nagSuppress;
  uint8_t speedProfile;      // 0-4
  uint8_t profileOverride;   // 1 = pin to speedProfile, 0 = follow CAN
  uint8_t speedOffset;       // 0-100 (HW3 only)
  uint8_t offsetOverride;    // 1 = pin to speedOffset, 0 = follow CAN
  uint8_t isaChimeSuppress;
  uint8_t summonInject;
};

inline bool loadSettings(State& s) {
  SavedSettings saved;
  EEPROM.get(SETTINGS_ADDR, saved);
  if (saved.magic != SETTINGS_MAGIC) return false;
  if (saved.version != SETTINGS_VERSION) return false;

  s.variant          = (Variant)saved.variant;
  s.fsdEnabled       = saved.fsdEnabled;
  s.nagSuppress      = saved.nagSuppress;
  s.speedProfile     = saved.speedProfile;
  s.profileOverride  = saved.profileOverride;
  s.speedOffset      = saved.speedOffset;
  s.offsetOverride   = saved.offsetOverride;
  s.isaChimeSuppress = saved.isaChimeSuppress;
  s.summonInject     = saved.summonInject;
  return true;
}

inline void saveSettings(const State& s) {
  SavedSettings saved;
  saved.magic            = SETTINGS_MAGIC;
  saved.version          = SETTINGS_VERSION;
  saved.variant          = (uint8_t)s.variant;
  saved.fsdEnabled       = s.fsdEnabled ? 1 : 0;
  saved.nagSuppress      = s.nagSuppress ? 1 : 0;
  saved.speedProfile     = (uint8_t)s.speedProfile;
  saved.profileOverride  = s.profileOverride ? 1 : 0;
  saved.speedOffset      = (uint8_t)s.speedOffset;
  saved.offsetOverride   = s.offsetOverride ? 1 : 0;
  saved.isaChimeSuppress = s.isaChimeSuppress ? 1 : 0;
  saved.summonInject     = s.summonInject ? 1 : 0;
  EEPROM.put(SETTINGS_ADDR, saved);  // put() → update() per byte, only writes changed cells
}
