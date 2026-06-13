#pragma once

#include <cstdint>

namespace tesla_fsd {

struct can_frame {
  uint32_t can_id;
  uint8_t can_dlc;
  uint8_t data[8];
};

struct FrameSink {
  void* context;
  bool (*send)(void*, const can_frame&);

  bool write(const can_frame& frame) const {
    return send != nullptr && send(context, frame);
  }
};

struct HandleResult {
  bool handled;
  bool attemptedSend;
  bool sent;
};

#if defined(__GNUC__)
#define TESLA_FSD_ALWAYS_INLINE inline __attribute__((always_inline))
#else
#define TESLA_FSD_ALWAYS_INLINE inline
#endif

// docs/can_meaning.dbc: BO_ 1016 ID3F8UI_driverAssistControl
constexpr uint32_t UI_DRIVER_ASSIST_CONTROL_ID = 1016;
// docs/can_meaning.dbc: BO_ 1021 ID3FDUI_autopilotControl
constexpr uint32_t UI_AUTOPILOT_CONTROL_ID = 1021;

// docs/can_meaning.dbc: SG_ UI_drivingSide : 40|2@1+ (BO_ 1016)
constexpr int UI_DRIVING_SIDE_BIT = 40;
// docs/can_meaning.dbc: SG_ UI_apmv3Branch m1 : 40|3@1+ (BO_ 1021)
constexpr int UI_APMV3_BRANCH_BIT = 40;

constexpr uint8_t UI_DRIVING_SIDE_LEFT = 0;
constexpr uint8_t UI_DRIVING_SIDE_RIGHT = 1;
constexpr uint8_t UI_DRIVING_SIDE_UNKNOWN = 2;
constexpr uint8_t UI_DRIVING_SIDE_OVERRIDE = UI_DRIVING_SIDE_LEFT;

constexpr uint8_t UI_APMV3_BRANCH_LIVE = 0;
constexpr uint8_t UI_APMV3_BRANCH_STAGE = 1;
constexpr uint8_t UI_APMV3_BRANCH_DEV = 2;
constexpr uint8_t UI_APMV3_BRANCH_STAGE2 = 3;
constexpr uint8_t UI_APMV3_BRANCH_EAP = 4;
constexpr uint8_t UI_APMV3_BRANCH_DEMO = 5;
constexpr uint8_t UI_APMV3_BRANCH_OVERRIDE = UI_APMV3_BRANCH_DEV;

TESLA_FSD_ALWAYS_INLINE HandleResult makeUnhandledResult() {
  return {};
}

TESLA_FSD_ALWAYS_INLINE HandleResult makeHandledResult(bool attemptedSend, bool sent) {
  return {true, attemptedSend, sent};
}

TESLA_FSD_ALWAYS_INLINE uint8_t readMuxID(const can_frame& frame) {
  return static_cast<uint8_t>(frame.data[0] & 0x07u);
}

TESLA_FSD_ALWAYS_INLINE bool isFSDSelectedInUI(const can_frame& frame) {
  return ((frame.data[4] >> 6) & 0x01u) != 0;
}

TESLA_FSD_ALWAYS_INLINE void setSpeedProfileV12V13(can_frame& frame, uint8_t profile) {
  frame.data[6] = static_cast<uint8_t>((frame.data[6] & ~0x06u) | ((profile & 0x03u) << 1));
}

TESLA_FSD_ALWAYS_INLINE void setBit(can_frame& frame, int bit, bool value) {
  const int byteIndex = bit >> 3;
  const uint8_t mask = static_cast<uint8_t>(1u << (bit & 0x07));
  if (value) {
    frame.data[byteIndex] |= mask;
  } else {
    frame.data[byteIndex] &= static_cast<uint8_t>(~mask);
  }
}

// The current hot signals all live in byte 5, so use direct masks instead of
// per-bit loops in the fast path.
TESLA_FSD_ALWAYS_INLINE uint8_t readDrivingSide(const can_frame& frame) {
  return static_cast<uint8_t>(frame.data[5] & 0x03u);
}

TESLA_FSD_ALWAYS_INLINE uint8_t readApmv3Branch(const can_frame& frame) {
  return static_cast<uint8_t>(frame.data[5] & 0x07u);
}

TESLA_FSD_ALWAYS_INLINE void setDrivingSide(can_frame& frame, uint8_t value) {
  frame.data[5] = static_cast<uint8_t>((frame.data[5] & ~0x03u) | (value & 0x03u));
}

TESLA_FSD_ALWAYS_INLINE void setApmv3Branch(can_frame& frame, uint8_t value) {
  frame.data[5] = static_cast<uint8_t>((frame.data[5] & ~0x07u) | (value & 0x07u));
}

TESLA_FSD_ALWAYS_INLINE int clampSpeedOffset(int rawOffset) {
  if (rawOffset <= 0) return 0;
  if (rawOffset >= 20) return 100;
  return rawOffset * 5;
}

TESLA_FSD_ALWAYS_INLINE void setAutopilotMode(can_frame& frame, uint8_t value) {
  frame.data[5] = static_cast<uint8_t>((frame.data[5] & ~0x1Cu) | ((value & 0x07u) << 2));
}

struct LegacyHandler {
  int speedProfile = 1;

  HandleResult handleMessage(can_frame& frame, const FrameSink& sink) {
    switch (frame.can_id) {
      case 1006:
        // Legacy CAN ID path; BO_ 1006 is not present in docs/can_meaning.dbc.
        switch (readMuxID(frame)) {
          case 0: {
            const int off = static_cast<int>((frame.data[3] >> 1) & 0x3Fu) - 30;
            switch (off) {
              case 2: speedProfile = 2; break;
              case 1: speedProfile = 1; break;
              case 0: speedProfile = 0; break;
              default: break;
            }
            setBit(frame, 46, true);  // docs/can_meaning.dbc: SG_ UI_showTrackLabels m1 : 46|1@1+ (BO_ 1021, mux 1)
            setSpeedProfileV12V13(frame, static_cast<uint8_t>(speedProfile));
            return makeHandledResult(true, sink.write(frame));
          }
          case 1:
            setBit(frame, 19, false);  // docs/can_meaning.dbc: SG_ UI_applyEceR79 m1 : 19|1@1+ (BO_ 1021, mux 1)
            return makeHandledResult(true, sink.write(frame));
          default:
            return makeHandledResult(false, false);
        }
      case 2047:
        if (frame.data[0] != 2u) return makeUnhandledResult();
        // docs/can_meaning.dbc: BO_ 2047 ID7FFcarConfig, SG_ GTW_autopilot m2 : 42|3@1+
        setAutopilotMode(frame, 3);
        return makeHandledResult(true, sink.write(frame));
      default:
        return makeUnhandledResult();
    }
  }
};

struct HW3Handler {
  int speedProfile = 1;
  int speedOffset = 0;
  bool navStopsEnabled = true;

  HandleResult handleMessage(can_frame& frame, const FrameSink& sink) {
    switch (frame.can_id) {
      case UI_DRIVER_ASSIST_CONTROL_ID: {
        const uint8_t followDistance = static_cast<uint8_t>((frame.data[5] >> 5) & 0x07u);
        switch (followDistance) {
          case 1: speedProfile = 2; break;
          case 2: speedProfile = 1; break;
          case 3: speedProfile = 0; break;
          default: break;
        }
        setBit(frame, 5, true);                  // docs/can_meaning.dbc: SG_ UI_dasDeveloper : 5|1@1+ (BO_ 1016)
        setBit(frame, 13, navStopsEnabled);       // docs/can_meaning.dbc: SG_ UI_driveOnMapsEnable : 13|1@1+ (BO_ 1016)
        setBit(frame, 14, true);                  // docs/can_meaning.dbc: SG_ UI_handsOnRequirementDisable : 14|1@1+ (BO_ 1016)
        setBit(frame, 48, navStopsEnabled);       // docs/can_meaning.dbc: SG_ UI_hasDriveOnNav : 48|1@1+ (BO_ 1016)
        setBit(frame, 49, navStopsEnabled);       // docs/can_meaning.dbc: SG_ UI_followNavRouteEnable : 49|1@1+ (BO_ 1016)
        setDrivingSide(frame, UI_DRIVING_SIDE_OVERRIDE);
        return makeHandledResult(true, sink.write(frame));
      }
      case UI_AUTOPILOT_CONTROL_ID:
        switch (readMuxID(frame)) {
          case 0: {
            const int rawOffset = static_cast<int>((frame.data[3] >> 1) & 0x3Fu) - 30;
            speedOffset = clampSpeedOffset(rawOffset);
            setBit(frame, 38, navStopsEnabled);   // docs/can_meaning.dbc: SG_ UI_fsdStopsControlEnabled m0 : 38|1@1+ (BO_ 1021, mux 0)
            setBit(frame, 46, true);              // docs/can_meaning.dbc: SG_ UI_showTrackLabels m1 : 46|1@1+ (BO_ 1021, mux 1)
            setSpeedProfileV12V13(frame, static_cast<uint8_t>(speedProfile));
            return makeHandledResult(true, sink.write(frame));
          }
          case 1:
            setApmv3Branch(frame, UI_APMV3_BRANCH_OVERRIDE);
            setBit(frame, 19, false);  // docs/can_meaning.dbc: SG_ UI_applyEceR79 m1 : 19|1@1+ (BO_ 1021, mux 1)
            setBit(frame, 45, true);   // docs/can_meaning.dbc: SG_ UI_showLaneGraph m1 : 45|1@1+ (BO_ 1021, mux 1)
            return makeHandledResult(true, sink.write(frame));
          case 2:
            frame.data[0] = static_cast<uint8_t>((frame.data[0] & ~0xC0u) | ((speedOffset & 0x03) << 6));
            frame.data[1] = static_cast<uint8_t>((frame.data[1] & ~0x3Fu) | ((speedOffset >> 2) & 0x3Fu));
            return makeHandledResult(true, sink.write(frame));
          default:
            return makeHandledResult(false, false);
        }
      case 2047:
        if (frame.data[0] != 2u) return makeUnhandledResult();
        // docs/can_meaning.dbc: BO_ 2047 ID7FFcarConfig, SG_ GTW_autopilot m2 : 42|3@1+
        setAutopilotMode(frame, 3);
        return makeHandledResult(true, sink.write(frame));
      default:
        return makeUnhandledResult();
    }
  }
};

struct HW4Handler {
  int speedProfile = 1;
  bool navStopsEnabled = true;

  HandleResult handleMessage(can_frame& frame, const FrameSink& sink) {
    switch (frame.can_id) {
      case UI_DRIVER_ASSIST_CONTROL_ID: {
        const uint8_t followDistance = static_cast<uint8_t>((frame.data[5] >> 5) & 0x07u);
        switch (followDistance) {
          case 1: speedProfile = 3; break;
          case 2: speedProfile = 2; break;
          case 3: speedProfile = 1; break;
          case 4: speedProfile = 0; break;
          case 5: speedProfile = 4; break;
          default: break;
        }
        setBit(frame, 5, true);                  // docs/can_meaning.dbc: SG_ UI_dasDeveloper : 5|1@1+ (BO_ 1016)
        setBit(frame, 13, navStopsEnabled);       // docs/can_meaning.dbc: SG_ UI_driveOnMapsEnable : 13|1@1+ (BO_ 1016)
        setBit(frame, 14, true);                  // docs/can_meaning.dbc: SG_ UI_handsOnRequirementDisable : 14|1@1+ (BO_ 1016)
        setBit(frame, 48, navStopsEnabled);       // docs/can_meaning.dbc: SG_ UI_hasDriveOnNav : 48|1@1+ (BO_ 1016)
        setBit(frame, 49, navStopsEnabled);       // docs/can_meaning.dbc: SG_ UI_followNavRouteEnable : 49|1@1+ (BO_ 1016)
        setDrivingSide(frame, UI_DRIVING_SIDE_OVERRIDE);
        return makeHandledResult(true, sink.write(frame));
      }
      case UI_AUTOPILOT_CONTROL_ID:
        switch (readMuxID(frame)) {
          case 0:
            setBit(frame, 38, navStopsEnabled);   // docs/can_meaning.dbc: SG_ UI_fsdStopsControlEnabled m0 : 38|1@1+ (BO_ 1021, mux 0)
            setBit(frame, 46, true);              // docs/can_meaning.dbc: SG_ UI_showTrackLabels m1 : 46|1@1+ (BO_ 1021, mux 1)
            setBit(frame, 60, navStopsEnabled);   // docs/can_meaning.dbc: SG_ UI_enableVisionOnlyStops : 60|1@1+ (BO_ 1016)
            return makeHandledResult(true, sink.write(frame));
          case 1:
            setApmv3Branch(frame, UI_APMV3_BRANCH_OVERRIDE);
            setBit(frame, 19, false);  // docs/can_meaning.dbc: SG_ UI_applyEceR79 m1 : 19|1@1+ (BO_ 1021, mux 1)
            setBit(frame, 45, true);   // docs/can_meaning.dbc: SG_ UI_showLaneGraph m1 : 45|1@1+ (BO_ 1021, mux 1)
            setBit(frame, 47, true);   // docs/can_meaning.dbc: SG_ UI_hardCoreSummon m1 : 47|1@1+ (BO_ 1021, mux 1)
            return makeHandledResult(true, sink.write(frame));
          case 2:
            frame.data[7] = static_cast<uint8_t>((frame.data[7] & ~0x70u) | ((speedProfile & 0x07) << 4));
            return makeHandledResult(true, sink.write(frame));
          default:
            return makeHandledResult(false, false);
        }
      case 2047:
        if (frame.data[0] != 2u) return makeUnhandledResult();
        // docs/can_meaning.dbc: BO_ 2047 ID7FFcarConfig, SG_ GTW_autopilot m2 : 42|3@1+
        setAutopilotMode(frame, 4);
        return makeHandledResult(true, sink.write(frame));
      default:
        return makeUnhandledResult();
    }
  }
};

#undef TESLA_FSD_ALWAYS_INLINE

}  // namespace tesla_fsd
