#pragma once
#include <stdint.h>

// ── CAN Frame ────────────────────────────────────────────────────────────────
struct Frame {
  uint32_t id;
  uint8_t dlc;
  uint8_t data[8];
};

// ── Variant ──────────────────────────────────────────────────────────────────
enum Variant { HW4, HW3, LEGACY };

inline const char* variantName(Variant v) {
  switch(v) {
    case HW4: return "hw4";
    case HW3: return "hw3";
    case LEGACY: return "legacy";
    default: return "hw4";
  }
}

inline bool parseVariant(const char* name, Variant& out) {
  if (strcmp(name, "hw4") == 0) { out = HW4; return true; }
  if (strcmp(name, "hw3") == 0) { out = HW3; return true; }
  if (strcmp(name, "legacy") == 0) { out = LEGACY; return true; }
  return false;
}

// ── Features ─────────────────────────────────────────────────────────────────
struct Features {
  bool fsd;
  bool profile;
  bool nag;
  bool speedOffset;
  bool isaChime;
  bool summon;
};

inline Features getFeatures(Variant v) {
  Features f = {true, true, true, false, false, false};
  if (v == HW3) { f.speedOffset = true; f.summon = true; }
  if (v == HW4) { f.isaChime = true;    f.summon = true; }
  return f;
}

// ── CAN Bus Health ───────────────────────────────────────────────────────────
#define CAN_TIMEOUT_MS       10000  // 10s without frames → standby
#define CAN_REINIT_INTERVAL  5000   // Re-try MCP2515 init every 5s in standby
#define LED_STANDBY_INTERVAL 2000   // Slow LED blink period in standby

// ── Summon Enums ─────────────────────────────────────────────────────────────
enum SummonDirection {
  SUMMON_FORWARD = 0,
  SUMMON_REVERSE = 1
};

enum SummonMode {
  SUMMON_STOP = 0,
  SUMMON_START = 1
};

// ── State ────────────────────────────────────────────────────────────────────
struct State {
  Variant variant;
  bool fsdEnabled;
  bool nagSuppress;
  int speedProfile;
  bool profileOverride;   // true = user-pinned, false = track CAN (stalk)
  int speedOffset;
  bool offsetOverride;    // true = user-pinned, false = track CAN (HW3 UI)
  bool isaChimeSuppress;
  bool streamEnabled;
  unsigned long streamCount;
  bool rawCanListen;

  // CAN bus health tracking
  unsigned long lastFrameMs;   // millis() of last received CAN frame
  bool canOnline;              // true when CAN frames are flowing
  bool standby;                // true when in standby (no CAN traffic)
  unsigned long lastReinitMs;  // last MCP2515 reinit attempt

  // Summon burst state (0x273 UI_vehicleControl injection)
  uint8_t summonRemaining;
  unsigned long summonLastMs;
  uint8_t lastCtrl[8];
  bool hasCtrl;
  SummonDirection summonDirection;
  SummonMode summonMode;
  
  // Additional CAN frame caching for advanced features
  uint8_t lastClimate[5];
  bool hasClimate;
  uint8_t lastCharge[5];
  bool hasCharge;
  uint8_t lastDrive[8];
  bool hasDrive;
  
#if BOARD_ENABLE_MCP2515_2
  uint8_t ctrlBus;
#endif

  State() : variant(HW4), fsdEnabled(false), nagSuppress(false),
            speedProfile(1), profileOverride(false),
            speedOffset(0), offsetOverride(false), isaChimeSuppress(false),
            streamEnabled(false), streamCount(0), rawCanListen(false),
            lastFrameMs(0), canOnline(false), standby(false), lastReinitMs(0),
            summonRemaining(0), summonLastMs(0), hasCtrl(false),
            summonDirection(SUMMON_FORWARD), summonMode(SUMMON_STOP),
            hasClimate(false), hasCharge(false), hasDrive(false)
#if BOARD_ENABLE_MCP2515_2
            , ctrlBus(0)
#endif
  {
    for (uint8_t i = 0; i < 8; i++) lastCtrl[i] = 0;
    for (uint8_t i = 0; i < 5; i++) lastClimate[i] = 0;
    for (uint8_t i = 0; i < 5; i++) lastCharge[i] = 0;
    for (uint8_t i = 0; i < 8; i++) lastDrive[i] = 0;
  }

  Features features() const { return getFeatures(variant); }
};
