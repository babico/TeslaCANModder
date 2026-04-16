#pragma once
#include "protocol/can.h"
#include "protocol/summon.h"
#include "protocol/bms.h"
#include "protocol/nag_killer.h"
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
  // Secondary buses (Bus 1..N) — vehicle control frames
#if (BUS_VEHICLE_ACTIVE || BUS_BODY_ACTIVE)
  static const uint32_t secIds[] = {CAN_ID_CLIMATE, CAN_ID_CHARGE, CAN_ID_DRIVE_CONFIG, CAN_ID_UI_VEHICLE_CTRL};
  for (uint8_t i = 1; i < BUS_MAX; i++) {
    if (!busActive(i)) continue;
    if (s.rawCanListen)
      driverSetBusFilters(i, nullptr, 0);
    else
      driverSetBusFilters(i, secIds, 4);
  }
#endif

  // Bus 0 (MCP2515_1) — variant-specific filter
  if (s.rawCanListen) {
    driverSetFilters(nullptr, 0);
    return;
  }

  switch(s.variant) {
    case HW4: {
      static const uint32_t ids[] = {CAN_ID_ISA_SPEED, CAN_ID_FOLLOW_DIST, CAN_ID_FSD_MUX, CAN_ID_UI_VEHICLE_CTRL};
      driverSetFilters(ids, 4);
      break;
    }
    case HW3: {
      static const uint32_t ids[] = {CAN_ID_FOLLOW_DIST, CAN_ID_FSD_MUX, CAN_ID_UI_VEHICLE_CTRL};
      driverSetFilters(ids, 3);
      break;
    }
    case LEGACY: {
      static const uint32_t ids[] = {CAN_ID_LEGACY_STALK, CAN_ID_LEGACY_FSD_MUX};
      driverSetFilters(ids, 2);
      break;
    }
  }
}

// ── Summon Tick ──────────────────────────────────────────────────────────────
void summonTick(State& s) {
  if (s.summonRemaining == 0 || !s.hasCtrl || !s.summonInject) return;
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
    s.hasBms = true;
    return;
  }
  if (f.id == CAN_ID_BMS_SOC && f.dlc >= 2) {
    s.bmsSoc = decodeBmsSoc(f.data);
    s.hasBms = true;
    return;
  }
  if (f.id == CAN_ID_BMS_THERMAL && f.dlc >= 2) {
    s.bmsTempMin = decodeBmsTempMin(f.data);
    s.bmsTempMax = decodeBmsTempMax(f.data);
    s.hasBms = true;
    return;
  }
  if (f.id == CAN_ID_BMS_ENERGY && f.dlc >= 2) {
    s.bmsWhPerKm = decodeBmsWhPerKm(f.data);
    s.hasBms = true;
    return;
  }

  // Nag killer: intercept EPAS torque frame and echo modified
  if (f.id == CAN_ID_EPAS_TORQUE && f.dlc >= 8) {
    if (s.nagKillerEnabled && !s.txPaused) {
      Frame echo = f;
      nagKillerModify(echo);
#if (BUS_VEHICLE_ACTIVE || BUS_BODY_ACTIVE)
      driverSend(echo, bus);
#else
      driverSend(echo);
#endif
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
