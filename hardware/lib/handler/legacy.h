#pragma once
#include "protocol/can.h"
#include "protocol/fsd.h"
#include "driver.h"

// Forward declaration
void sendLog(const char* msg);

static bool legacyLoggedFSD = false;
static bool legacyLoggedNag = false;

// ── Legacy Handler ───────────────────────────────────────────────────────────
void handleLegacy(Frame& f, State& s) {
  // Legacy stalk position → profile mapping (auto-track unless pinned)
  if (f.id == CAN_ID_LEGACY_STALK) {
    if (!s.profileOverride && f.dlc >= 2) {
      uint8_t stalk = f.data[1] >> 5;
      if (stalk <= 1) s.speedProfile = 2;
      else if (stalk == 2) s.speedProfile = 1;
      else s.speedProfile = 0;
    }
    return;
  }
  
  // FSD mux handling
  if (f.id == CAN_ID_LEGACY_FSD_MUX) {
    uint8_t mux = readMuxID(f);
    bool fsdUI = isFSDSelectedInUI(f);
    
    if (mux == 0 && s.fsdEnabled && fsdUI) {
      int steps = readHW3UiOffsetSteps(f);
      if (!s.profileOverride && steps >= 0 && steps <= 2) s.speedProfile = steps;
      setBit(f, 46, true);
      setSpeedProfileV12V13(f, s.speedProfile);
      driverSend(f);
      if (!legacyLoggedFSD) { sendLog("Legacy: FSD mod active on CAN"); legacyLoggedFSD = true; }
      return;
    } else if (mux == 0) { legacyLoggedFSD = false; }
    
    if (mux == 1 && s.nagSuppress) {
      setBit(f, 19, false);
      driverSend(f);
      if (!legacyLoggedNag) { sendLog("Legacy: Nag suppressed on CAN"); legacyLoggedNag = true; }
      return;
    } else if (mux == 1) { legacyLoggedNag = false; }
    }
  }
}
