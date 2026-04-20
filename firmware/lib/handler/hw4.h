#pragma once
#include "core/forward.h"
#include "infra/can.h"
#include "feature/profile.h"
#include "feature/isa_chime.h"
#include "feature/region.h"

// Forward declarations (defined by platform-specific serial)
void sendLog(const char* msg);
void sendLog(const __FlashStringHelper* msg);

static bool hw4LoggedFSD = false;
static bool hw4LoggedNag = false;
static bool hw4LoggedISA = false;

void resetHW4LogFlags() { hw4LoggedFSD = false; hw4LoggedNag = false; hw4LoggedISA = false; }

// ── HW4 Handler ──────────────────────────────────────────────────────────────
void handleHW4(Frame& f, State& s) {
  // OTA safety: pass-through unmodified frames during OTA update
  if (s.txPaused) { driverSend(f, 0); return; }

  // ISA speed chime suppression
  if (f.id == CAN_ID_ISA_SPEED && s.isaChimeSuppress) {
    if (f.dlc >= 8) {
      f.data[1] |= 0x20;
      f.data[7] = computeHW4IsaChecksum(f);
      driverSend(f, 0);
      if (!hw4LoggedISA) { sendLog(F("HW4: ISA chime suppressed")); hw4LoggedISA = true; }
      return;
    }
    return;
  }

  // Follow distance → profile mapping (auto-track from stalk unless pinned)
  if (f.id == CAN_ID_FOLLOW_DIST) {
    if (!s.profileOverride) {
      int profile = mapHW4FollowDistToProfile(readFollowDistance(f));
      if (profile >= 0) s.speedProfile = profile;
    }
    return;
  }

  // FSD mux handling
  if (f.id == CAN_ID_FSD_MUX) {
    uint8_t mux = readMuxID(f);
    bool fsdAllowed = s.fsdEnabled && (s.fsdForceEnabled || isFSDSelectedInUI(f));

    if (mux == 0 && fsdAllowed) {
      setBit(f, 38, true);
      setBit(f, 46, true);
      setBit(f, 60, true);
      // Emergency Vehicle Detection: allow AP to respond to emergency vehicles
      // Source: hypery11/flipper-tesla-fsd fsd_handler.c (bit 59 on mux=0)
      if (s.evdEnabled) setBit(f, 59, true);
      driverSend(f, 0);
      if (!hw4LoggedFSD) { sendLog(F("HW4: FSD mod active on CAN")); hw4LoggedFSD = true; }
      return;
    }

    if (mux == 1 && s.nagSuppress) {
      setBit(f, 19, false);
      setBit(f, 47, true);
      // Enhanced Autopilot: set bit 46 on mux=1 to unlock EAP/Summon
      // Source: ev-open-can-tools hw4OffsetRuntime + hypery11 enhanced_autopilot
      if (s.enhancedAutopilot) setBit(f, 46, true);
      // ECE R79 bypass: clear EU speed restriction bit for European vehicles
      if (s.eceR79Bypass && s.hasRegion && isEuropeanMarket(s.regionCode)) {
        applyEceR79Bypass(f);
      }
      driverSend(f, 0);
      if (!hw4LoggedNag) { sendLog(F("HW4: Nag suppressed on CAN")); hw4LoggedNag = true; }
      return;
    }

    if (mux == 2 && s.fsdEnabled) {
      writeHW4SpeedProfile(f, s.speedProfile);
      if (s.speedOffset > 0) {
        f.data[1] = (f.data[1] & 0xC0) | (s.speedOffset & 0x3F);
      }
      driverSend(f, 0);
      return;
    }
  }
}
