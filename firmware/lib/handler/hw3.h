#pragma once
#include "core/forward.h"
#include "infra/can.h"
#include "feature/profile.h"
#include "feature/offsets.h"
#include "feature/region.h"

// Forward declarations (defined by platform-specific serial)
void sendLog(const char* msg);
void sendLog(const __FlashStringHelper* msg);

static bool hw3LoggedFSD = false;
static bool hw3LoggedNag = false;
static bool hw3LoggedOffset = false;

void resetHW3LogFlags() { hw3LoggedFSD = false; hw3LoggedNag = false; hw3LoggedOffset = false; }

// ── HW3 Handler ──────────────────────────────────────────────────────────────
void handleHW3(Frame& f, State& s) {
  // OTA safety: pass-through unmodified frames during OTA update
  if (s.txPaused) { driverSend(f, 0); return; }

  // Follow distance → profile mapping (auto-track from stalk unless pinned)
  if (f.id == CAN_ID_FOLLOW_DIST) {
    if (!s.profileOverride) {
      int profile = mapHW3FollowDistToProfile(readFollowDistance(f));
      if (profile >= 0) s.speedProfile = profile;
    }
    return;
  }

  // FSD mux handling
  if (f.id == CAN_ID_FSD_MUX) {
    uint8_t mux = readMuxID(f);
    bool fsdUI = isFSDSelectedInUI(f);
    bool fsdAllowed = s.fsdEnabled && (s.fsdForceEnabled || fsdUI);

    if (mux == 0 && fsdAllowed) {
      int steps = readHW3UiOffsetSteps(f);
      if (!s.profileOverride && steps >= 0 && steps <= 2) s.speedProfile = steps;
      if (!s.offsetOverride) s.speedOffset = calculateHW3SpeedOffset(steps);

      setBit(f, 38, true);
      setBit(f, 46, true);
      setSpeedProfileV12V13(f, s.speedProfile);
      driverSend(f, 0);
      if (!hw3LoggedFSD) { sendLog(F("HW3: FSD mod active on CAN")); hw3LoggedFSD = true; }
      return;
    }

    if (mux == 1 && s.nagSuppress) {
      setBit(f, 19, false);
      // Enhanced Autopilot: set bit 46 on mux=1 to unlock EAP/Summon
      // Source: ev-open-can-tools HW3Handler enhancedAutopilotRuntime + hypery11
      if (s.enhancedAutopilot) setBit(f, 46, true);
      // ECE R79 bypass: clear EU speed restriction bit for European vehicles
      if (s.eceR79Bypass && s.hasRegion && isEuropeanMarket(s.regionCode)) {
        applyEceR79Bypass(f);
      }
      driverSend(f, 0);
      if (!hw3LoggedNag) { sendLog(F("HW3: Nag suppressed on CAN")); hw3LoggedNag = true; }
      return;
    }

    if (mux == 2 && s.fsdEnabled) {
      writeHW3SpeedOffset(f, s.speedOffset);
      driverSend(f, 0);
      if (!hw3LoggedOffset) { sendLog(F("HW3: Speed offset applied")); hw3LoggedOffset = true; }
      return;
    }
  }
}
