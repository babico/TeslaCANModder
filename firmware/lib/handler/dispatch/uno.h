#pragma once
#include "infra/can.h"
#include "feature/summon.h"
#include "feature/bms.h"
#include "feature/nag.h"
#include "feature/ban_detect.h"
#include "feature/auto_lane_change.h"
#include "feature/turn_signal.h"
#include "feature/drive_context.h"
#include "core/driver/uno.h"
#include "handler/hw4.h"
#include "handler/hw3.h"
#include "handler/legacy.h"

void resetHandlerLogFlags() {
  resetHW4LogFlags();
  resetHW3LogFlags();
  resetLegacyLogFlags();
}

// ── Filter Setup ─────────────────────────────────────────────────────────────
void applyFilters(State& s) {
  // Vehicle bus (Bus 1) — vehicle control + BMS + OTA + nag killer frames
#if BUS_VEHICLE_ACTIVE
  if (s.rawCanListen) {
    driverSetBusFilters(BUS_VEHICLE, nullptr, 0);
  } else {
    static const uint32_t vehIds[] = {
      CAN_ID_PRECONDITION, CAN_ID_BMS_HV_BUS, CAN_ID_UI_VEHICLE_CTRL,
      CAN_ID_BMS_SOC, CAN_ID_CLIMATE, CAN_ID_BMS_THERMAL,
      CAN_ID_TRACK_MODE, CAN_ID_GTW_CAR_STATE, CAN_ID_CHARGE,
      CAN_ID_DRIVE_CONFIG, CAN_ID_BMS_ENERGY, CAN_ID_EPAS_TORQUE,
      CAN_ID_GTW_CAR_CFG, CAN_ID_DAS_STATUS, CAN_ID_GTW_CONFIG_ETH,
      CAN_ID_BLIND_SPOT,
      CAN_ID_VCLEFT_DOOR_STATUS, CAN_ID_VCRIGHT_DOOR_STATUS,
      CAN_ID_VCFRONT_STATUS, CAN_ID_VCFRONT_VEH_STATUS,
      CAN_ID_BMS_ENERGY_ST, CAN_ID_BMS_MIN_MAX, CAN_ID_BMS_POWER_AV,
      CAN_ID_BMS_STATUS, CAN_ID_BMS_DRIVE_LIM, CAN_ID_BMS_KWH_CNT,
      CAN_ID_BMS_KWH_MUX, CAN_ID_BMS_BRICK_V,
      CAN_ID_VCFRONT_LIGHTS
    };
    driverSetBusFilters(BUS_VEHICLE, vehIds, sizeof(vehIds)/sizeof(vehIds[0]));
  }
#endif

  // Body bus (Bus 2) — body control frames
#if BUS_BODY_ACTIVE
  if (s.rawCanListen) {
    driverSetBusFilters(BUS_BODY, nullptr, 0);
  } else {
    static const uint32_t bodyIds[] = {CAN_ID_WINDOW_VENT, CAN_ID_SENTRY, CAN_ID_TRUNK_CTRL};
    driverSetBusFilters(BUS_BODY, bodyIds, sizeof(bodyIds)/sizeof(bodyIds[0]));
  }
#endif

  // Bus 0 (MCP2515_1) — dynamic filters based on enabled features
  // Toggle-inject pattern: feature ON → intercept + inject, feature OFF → don't intercept CAN line
  if (s.rawCanListen) {
    driverSetFilters(nullptr, 0);
    return;
  }

  uint32_t ids[10];
  uint8_t count = 0;
  ids[count++] = CAN_ID_DAS_CONTROL;
  ids[count++] = CAN_ID_DAS_STATUS2;
  ids[count++] = CAN_ID_UI_GPS_SPEED;

  switch(s.variant) {
    case HW4:
      if (s.isaChimeSuppress) ids[count++] = CAN_ID_ISA_SPEED;
      if (s.fsdEnabled) ids[count++] = CAN_ID_FOLLOW_DIST;
      if (s.fsdEnabled || s.nagSuppress) ids[count++] = CAN_ID_FSD_MUX;
      ids[count++] = CAN_ID_UI_VEHICLE_CTRL;
      break;
    case HW3:
      if (s.fsdEnabled) ids[count++] = CAN_ID_FOLLOW_DIST;
      if (s.fsdEnabled || s.nagSuppress) ids[count++] = CAN_ID_FSD_MUX;
      ids[count++] = CAN_ID_UI_VEHICLE_CTRL;
      break;
    case LEGACY:
      if (s.fsdEnabled) ids[count++] = CAN_ID_LEGACY_STALK;
      if (s.fsdEnabled || s.nagSuppress) ids[count++] = CAN_ID_LEGACY_FSD_MUX;
      break;
  }

  if (count > 0) {
    driverSetFilters(ids, count);
  } else {
    static const uint32_t none[] = {0x000};
    driverSetFilters(none, 1);
  }
}

// ── Summon Tick ──────────────────────────────────────────────────────────────
void summonTick(State& s) {
  if (s.summonRemaining == 0 || !s.hasCtrl || !s.summonInject) return;
  if (s.txPaused) { s.summonRemaining = 0; return; }
  unsigned long now = millis();
  if (now - s.summonLastMs < 20) return;
  s.summonLastMs = now;

  Frame f;
  f.id = CAN_ID_UI_VEHICLE_CTRL;
  f.dlc = 8;
  memcpy(f.data, s.lastCtrl, 8);
  setSummonActive(f, true);
  setSummonDirection(f, s.summonDirection);
  setSummonMode(f, s.summonMode);
#if (BUS_VEHICLE_ACTIVE || BUS_BODY_ACTIVE)
  driverSend(f, s.ctrlBus);
#else
  driverSend(f, 0);
#endif
  s.summonRemaining--;
  if (s.summonRemaining == 0) sendLog(F("Summon burst complete"));
}

// ── Preconditioning Tick ──────────────────────────────────────────────────────
void preconditionTick(State& s) {
  if (!s.preconditionEnabled) return;
  if (s.txPaused) return;
  unsigned long now = millis();
  if (now - s.precondLastMs < 500) return;
  s.precondLastMs = now;
  Frame f;
  f.id = CAN_ID_PRECONDITION;
  f.dlc = 8;
  memset(f.data, 0, 8);
  f.data[0] = 0x05;
#if (BUS_VEHICLE_ACTIVE || BUS_BODY_ACTIVE)
  driverSend(f, BUS_VEHICLE);
#else
  driverSend(f, 0);
#endif
}

// ── Burst Tick (non-blocking one-shot sends) ────────────────────────────────────
void burstTick(State& s) {
  if (s.burstRemaining == 0) return;
  if (s.txPaused) { s.burstRemaining = 0; return; }
  unsigned long now = millis();
  if (now - s.burstLastMs < s.burstDelayMs) return;
  s.burstLastMs = now;
  driverSend(s.burstFrame, s.burstBus);
  s.burstRemaining--;
}

// ── Message Dispatch ─────────────────────────────────────────────────────────
void handleMessage(Frame& f, uint8_t bus, State& s) {
  if (f.id == CAN_ID_UI_VEHICLE_CTRL && f.dlc >= 8) {
    memcpy(s.lastCtrl, f.data, 8);
    s.hasCtrl = true;
#if (BUS_VEHICLE_ACTIVE || BUS_BODY_ACTIVE)
    s.ctrlBus = bus;
#endif
    return;
  }

  if (f.id == CAN_ID_CLIMATE && f.dlc >= 5) {
    memcpy(s.lastClimate, f.data, 5);
    s.hasClimate = true;
    return;
  }

  if (f.id == CAN_ID_CHARGE && f.dlc >= 5) {
    memcpy(s.lastCharge, f.data, 5);
    s.hasCharge = true;
    return;
  }

  if (f.id == CAN_ID_DRIVE_CONFIG && f.dlc >= 8) {
    memcpy(s.lastDrive, f.data, 8);
    s.hasDrive = true;
    return;
  }

  // BMS battery telemetry (read-only decode)
  if (f.id == CAN_ID_BMS_HV_BUS && f.dlc >= 4) {
    s.bmsVoltage = decodeBmsVoltage(f.data);
    s.bmsCurrent = decodeBmsCurrent(f.data);
    s.bmsPower = decodeBmsPower(f.data);
    if (f.dlc >= 8) s.bmsChargeTimeToFull = decodeBmsChargeTimeToFull(f.data);
    s.hasBms = true;
    return;
  }
  if (f.id == CAN_ID_BMS_SOC && f.dlc >= 2) {
    s.bmsSoc = decodeBmsSoc(f.data);
    if (f.dlc >= 5) {
      s.bmsSocUI = decodeBmsSocUI(f.data);
      s.bmsSocMax = decodeBmsSocMax(f.data);
      s.bmsSocAvg = decodeBmsSocAvg(f.data);
    }
    if (f.dlc >= 7) s.bmsInitialFullPack = decodeBmsInitialFullPack(f.data);
    s.hasBms = true;
    return;
  }
  if (f.id == CAN_ID_BMS_THERMAL && f.dlc >= 2) {
    s.bmsTempMin = decodeBmsTempMin(f.data);
    s.bmsTempMax = decodeBmsTempMax(f.data);
    if (f.dlc >= 8) {
      s.bmsPowerDissipation = decodeBmsPowerDissipation(f.data);
      s.bmsFlowRequest = decodeBmsFlowRequest(f.data);
      s.bmsCoolTarget = decodeBmsCoolTarget(f.data);
      s.bmsPassiveTarget = decodeBmsPassiveTarget(f.data);
      s.bmsHeatTarget = decodeBmsHeatTarget(f.data);
      s.bmsPackTMin = decodeBmsPackTMin(f.data);
      s.bmsPackTMax = decodeBmsPackTMax(f.data);
    }
    s.hasBms = true;
    return;
  }
  if (f.id == CAN_ID_BMS_ENERGY && f.dlc >= 2) {
    s.bmsWhPerKm = decodeBmsWhPerKm(f.data);
    if (f.dlc >= 8) {
      s.bmsExpectedRange = decodeBmsExpectedRange(f.data);
      s.bmsIdealRange = decodeBmsIdealRange(f.data);
      s.bmsRatedConsumption = decodeBmsRatedConsumption(f.data);
      s.bmsActualSocInt = decodeBmsActualSocInt(f.data);
      s.bmsUsableSocInt = decodeBmsUsableSocInt(f.data);
    }
    s.hasBms = true;
    return;
  }

  if (f.id == CAN_ID_DAS_STATUS && f.dlc >= 6) {
    s.dasHandsOnState = readDasHandsOnState(f);
    s.dasLaneChangeState = readDasLaneChangeState(f);
    s.dasSeen = true;
    // ALC auto-confirm on Uno (stalk-only, no Palladium)
    unsigned long now = millis();
    if (alcShouldConfirm(s, now)) {
      int8_t dir = alcDirectionFromTurnSignal(s);
      if (dir != 0) {
        Frame confirm;
        buildStalkFrame(confirm, dir < 0);
#if (BUS_VEHICLE_ACTIVE || BUS_BODY_ACTIVE)
        driverSend(confirm, bus);
#else
        driverSend(confirm, 0);
#endif
        s.alcLastConfirmMs = now;
        sendLog(dir < 0 ? F("ALC: confirmed LEFT") : F("ALC: confirmed RIGHT"));
      }
    }
    return;
  }

  if (f.id == CAN_ID_DAS_CONTROL && f.dlc >= 2) {
    s.cruiseSetSpeedKph = decodeCruiseSetSpeedKph(f);
    s.maxSpeedKph = s.cruiseSetSpeedKph;
    return;
  }

  if (f.id == CAN_ID_DAS_STATUS2 && f.dlc >= 2) {
    s.accSpeedLimitKph = decodeAccSpeedLimitKph(f);
    if (s.accSpeedLimitKph > s.maxSpeedKph) s.maxSpeedKph = s.accSpeedLimitKph;
    return;
  }

  if (f.id == CAN_ID_UI_GPS_SPEED && f.dlc >= 7) {
    s.mapSpeedLimitKph = decodeMapSpeedLimitKph(f);
    if (s.mapSpeedLimitKph > s.maxSpeedKph) s.maxSpeedKph = s.mapSpeedLimitKph;
    return;
  }

  // D-05 safety cues: turn signal status from VCFRONT lights
  if (f.id == CAN_ID_VCFRONT_LIGHTS && f.dlc >= 7) {
    s.turnSignalLeft = decodeTurnSignalLeftActive(f);
    s.turnSignalRight = decodeTurnSignalRightActive(f);
    return;
  }

  // D-05 safety cues: blind-spot levels
  if (f.id == CAN_ID_BLIND_SPOT && f.dlc >= 1) {
    s.bsmLeftLevel = decodeBlindSpotLeftLevel(f);
    s.bsmRightLevel = decodeBlindSpotRightLevel(f);
    return;
  }

  if (f.id == CAN_ID_VCLEFT_DOOR_STATUS && f.dlc >= 2) {
    s.doorFrontLeftOpen = decodeDoorFrontLeftOpen(f);
    s.doorRearLeftOpen = decodeDoorRearLeftOpen(f);
    return;
  }

  if (f.id == CAN_ID_VCRIGHT_DOOR_STATUS && f.dlc >= 8) {
    s.doorFrontRightOpen = decodeDoorFrontRightOpen(f);
    s.doorRearRightOpen = decodeDoorRearRightOpen(f);
    s.trunkOpen = decodeTrunkOpen(f);
    return;
  }

  if (f.id == CAN_ID_VCFRONT_STATUS && f.dlc >= 8) {
    s.frunkOpen = decodeFrunkOpen(f);
    s.anyDoorOpen = decodeAnyDoorOpen(f);
    return;
  }

  if (f.id == CAN_ID_VCFRONT_VEH_STATUS && f.dlc >= 4) {
    s.driverDoorOpen = decodeDriverDoorOpen(f);
    return;
  }

// Enhanced BMS: degradation / capacity (mux=0) + energy status (mux=1)
    if (f.id == CAN_ID_BMS_ENERGY_ST && f.dlc >= 8) {
      uint8_t mux = f.data[0] & 0x0F;
      if (mux == 0) {
        s.bmsNominalFullPack  = decodeBmsNominalFullPack(f.data);
        s.bmsNominalRemaining = decodeBmsNominalRemaining(f.data);
        s.bmsIdealRemaining   = decodeBmsIdealRemaining(f.data);
        s.hasEnhancedBms = true;
      } else if (mux == 1) {
        s.bmsEnergyBuffer       = decodeBmsEnergyBuffer(f.data);
        s.bmsExpectedRemaining  = decodeBmsExpectedRemaining(f.data);
        s.bmsEnergyToCharge     = decodeBmsEnergyToChargeComplete(f.data);
        s.bmsFullyCharged       = decodeBmsFullyCharged(f.data);
        s.hasEnhancedBms = true;
      }
      return;
    }
    if (f.id == CAN_ID_BMS_MIN_MAX && f.dlc >= 4) {
      uint8_t mux = f.data[0] & 0x0F;
      if (mux == 1) {
        s.bmsCellVoltageMax = decodeBmsCellVoltageMax(f.data);
        s.bmsCellVoltageMin = decodeBmsCellVoltageMin(f.data);
        s.hasEnhancedBms = true;
      } else if (mux == 0 && f.dlc >= 6) {
        s.bmsThermistorTMax = decodeBmsThermistorTMax(f.data);
        s.bmsThermistorTMin = decodeBmsThermistorTMin(f.data);
        s.bmsModelTMax = decodeBmsModelTMax(f.data);
        s.bmsModelTMin = decodeBmsModelTMin(f.data);
        s.hasEnhancedBms = true;
      }
      return;
    }
    if (f.id == CAN_ID_BMS_POWER_AV && f.dlc >= 4) {
      s.bmsMaxRegenPower     = decodeBmsMaxRegenPower(f.data);
      s.bmsMaxDischargePower = decodeBmsMaxDischargePower(f.data);
      if (f.dlc >= 8) {
        s.bmsStationaryHeatPower = decodeBmsStationaryHeatPower(f.data);
        s.bmsHvacPowerBudget = decodeBmsHvacPowerBudget(f.data);
      }
      s.hasEnhancedBms = true;
      return;
    }

    // BMS_status (0x212): precondition flags, HV state, contactor
    if (f.id == CAN_ID_BMS_STATUS && f.dlc >= 3) {
      s.bmsPrecondAllowed = decodeBmsPrecondAllowed(f.data);
      s.bmsHeatingWorthwhile = decodeBmsHeatingWorthwhile(f.data);
      s.bmsContactorState = decodeBmsContactorState(f.data);
      s.bmsHvState = decodeBmsHvState(f.data);
      s.hasEnhancedBms = true;
      return;
    }

    // BMS_driveLimits (0x2D2): bus voltage/current limits
    if (f.id == CAN_ID_BMS_DRIVE_LIM && f.dlc >= 8) {
      s.bmsMinBusVoltage = decodeBmsMinBusVoltage(f.data);
      s.bmsMaxBusVoltage = decodeBmsMaxBusVoltage(f.data);
      s.bmsMaxChargeCurrent = decodeBmsMaxChargeCurrent(f.data);
      s.bmsMaxDischargeCurrent = decodeBmsMaxDischargeCurrent(f.data);
      s.hasEnhancedBms = true;
      return;
    }

    // BMS_kwhCounter (0x3D2): lifetime counters
    if (f.id == CAN_ID_BMS_KWH_CNT && f.dlc >= 8) {
      s.bmsKwhDischargeTotal = decodeBmsKwhDischargeTotal(f.data);
      s.bmsKwhChargeTotal = decodeBmsKwhChargeTotal(f.data);
      s.hasEnhancedBms = true;
      return;
    }

    // BMS_kwhCountersMultiplexed (0x3F2)
    if (f.id == CAN_ID_BMS_KWH_MUX && f.dlc >= 5) {
      uint8_t mux = f.data[0];
      float val = decodeBmsKwhMuxCounter(f.data);
      switch (mux) {
        case 0: s.bmsAcChargeTotal = val; break;
        case 1: s.bmsDcChargeTotal = val; break;
        case 2: s.bmsRegenTotal = val; break;
        case 3: s.bmsDriveDischargeTotal = val; break;
      }
      s.hasEnhancedBms = true;
      return;
    }

    // Nag killer + steering mode: intercept EPAS torque frame
    if (f.id == CAN_ID_EPAS_TORQUE && f.dlc >= 8) {
      s.steeringMode = (f.data[0] >> 4) & 0x0F;
      s.hasSteeringMode = true;
      if (!s.txPaused && nagKillerShouldEcho(s)) {
        Frame echo = f;
        if (s.nagKillerMode == NAG_KILLER_NATURAL &&
            nagNaturalIntervalReady(s, millis())) {
          float torque = nagNaturalTorque(s.steeringAngle, s.dasHandsOnState);
          nagKillerModifyNatural(echo, torque);
#if (BUS_VEHICLE_ACTIVE || BUS_BODY_ACTIVE)
          driverSend(echo, bus);
#else
          driverSend(echo, 0);
#endif
        } else if (s.nagKillerMode != NAG_KILLER_NATURAL) {
          nagKillerModify(echo);
#if (BUS_VEHICLE_ACTIVE || BUS_BODY_ACTIVE)
          driverSend(echo, bus);
#else
          driverSend(echo, 0);
#endif
        }
      }
      return;
    }

  // OTA safety check
  if (f.id == CAN_ID_GTW_CAR_STATE && f.dlc >= 1) {
    bool otaActive = (f.data[0] & 0x01) != 0;
    if (otaActive && !s.otaInProgress) {
      s.otaInProgress = true;
      s.txPaused = true;
      sendLog(F("OTA detected - TX paused"));
    } else if (!otaActive && s.otaInProgress) {
      s.otaInProgress = false;
      s.txPaused = false;
      sendLog(F("OTA complete - TX resumed"));
    }
    return;
  }

  // Auto HW detection
  if (f.id == CAN_ID_GTW_CAR_CFG && f.dlc >= 1) {
    uint8_t hw = (f.data[0] >> 6) & 0x03;
    if (hw == 2 || hw == 3) {
      s.detectedHW = hw;
      s.hwAutoDetected = true;
      // Auto-switch variant if enabled (inspired by hypery11 detection)
      if (s.variantAutoDetect) {
        Variant detected = (hw == 3) ? HW4 : HW3;
        if (s.variant != detected) {
          s.variant = detected;
          applyFilters(s);
          resetHandlerLogFlags();
          sendLog(hw == 3 ? F("Auto-detected HW4") : F("Auto-detected HW3"));
        }
      }
    }
    return;
  }

  // GTW autopilot tier readback from mixed/Ethernet bridge frame
  if (f.id == CAN_ID_GTW_CONFIG_ETH) {
    int8_t tier = readGtwAutopilotTier(f);
    if (tier >= 0) {
      int8_t prev = s.gtwAutopilotTier;
      s.gtwAutopilotTier = tier;
      s.gtwAutopilotSeen = true;
      if (checkBanDetection(prev, tier, s)) {
        sendLog(F("BAN DETECTED: AP tier dropped"));
      }
    }
    return;
  }

  // Only process FSD/variant-specific frames from bus 0 (MCP2515_1)
  if (bus != 0) return;

  switch(s.variant) {
    case HW4: handleHW4(f, s); break;
    case HW3: handleHW3(f, s); break;
    case LEGACY: handleLegacy(f, s); break;
  }
}
