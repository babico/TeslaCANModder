#pragma once
#include "protocol/can.h"
#include "protocol/fsd.h"
#include "core/driver.h"

// Forward declaration
void sendLog(const char* msg);
void sendLog(const __FlashStringHelper* msg);

static bool hw4LoggedFSD = false;
static bool hw4LoggedNag = false;
static bool hw4LoggedISA = false;

// ── HW4 Handler ──────────────────────────────────────────────────────────────
void handleHW4(Frame& f, State& s) {
  // ISA speed chime suppression
  if (f.id == CAN_ID_ISA_SPEED && s.isaChimeSuppress) {
    if (f.dlc >= 8) {
      f.data[1] |= 0x20;
      f.data[7] = computeHW4IsaChecksum(f);
      driverSend(f);
      if (!hw4LoggedISA) { sendLog(F("HW4: ISA chime suppressed")); hw4LoggedISA = true; }
      return;
    }
    return;
  } else { hw4LoggedISA = false; }
  
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
    
    if (mux == 0 && s.fsdEnabled && isFSDSelectedInUI(f)) {
      setBit(f, 38, true);
      setBit(f, 46, true);
      setBit(f, 60, true);
      driverSend(f);
      if (!hw4LoggedFSD) { sendLog(F("HW4: FSD mod active on CAN")); hw4LoggedFSD = true; }
      return;
    } else if (mux == 0) { hw4LoggedFSD = false; }
    
    if (mux == 1 && s.nagSuppress) {
      setBit(f, 19, false);
      setBit(f, 47, true);
      driverSend(f);
      if (!hw4LoggedNag) { sendLog(F("HW4: Nag suppressed on CAN")); hw4LoggedNag = true; }
      return;
    } else if (mux == 1) { hw4LoggedNag = false; }
    
    if (mux == 2 && s.fsdEnabled) {
      writeHW4SpeedProfile(f, s.speedProfile);
      driverSend(f);
      return;
    }
  }
}
