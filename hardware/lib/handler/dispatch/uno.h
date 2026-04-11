#pragma once
#include "protocol/can.h"
#include "protocol/summon.h"
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
  driverSend(f);
#endif
  s.summonRemaining--;
  if (s.summonRemaining == 0) sendLog(F("Summon burst complete"));
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

  // Only process FSD/variant-specific frames from bus 0 (MCP2515_1)
  if (bus != 0) return;

  switch(s.variant) {
    case HW4: handleHW4(f, s); break;
    case HW3: handleHW3(f, s); break;
    case LEGACY: handleLegacy(f, s); break;
  }
}
