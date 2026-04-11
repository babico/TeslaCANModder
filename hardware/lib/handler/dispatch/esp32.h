#pragma once
#include "protocol/can.h"
#include "protocol/summon.h"
#include "core/driver/esp32.h"
#include "handler/hw4.h"
#include "handler/hw3.h"
#include "handler/legacy.h"

void resetHandlerLogFlags() {
  resetHW4LogFlags();
  resetHW3LogFlags();
  resetLegacyLogFlags();
}

// ── Per-Bus Filter Setup (Hardcoded Tesla X179) ─────────────────────────────
void applyFilters(State& s) {
  // Bus 0 (FSD bus, X179 pins 13-14): variant-specific FSD frames
  if (s.rawCanListen) {
    driverSetBusFilters(0, nullptr, 0);
  } else {
    switch(s.variant) {
      case HW4: {
        static const uint32_t ids[] = {CAN_ID_ISA_SPEED, CAN_ID_FOLLOW_DIST, CAN_ID_FSD_MUX};
        driverSetBusFilters(BUS_FSD, ids, 3);
        break;
      }
      case HW3: {
        static const uint32_t ids[] = {CAN_ID_FOLLOW_DIST, CAN_ID_FSD_MUX};
        driverSetBusFilters(BUS_FSD, ids, 2);
        break;
      }
      case LEGACY: {
        static const uint32_t ids[] = {CAN_ID_LEGACY_STALK, CAN_ID_LEGACY_FSD_MUX};
        driverSetBusFilters(BUS_FSD, ids, 2);
        break;
      }
    }
  }

#if BUS_VEHICLE_ACTIVE
  // Bus 1 (Vehicle bus, X179 pins 9-10): vehicle control frames
  if (s.rawCanListen) {
    driverSetBusFilters(BUS_VEHICLE, nullptr, 0);
  } else {
    static const uint32_t vehIds[] = {CAN_ID_UI_VEHICLE_CTRL, CAN_ID_CLIMATE, CAN_ID_CHARGE, CAN_ID_DRIVE_CONFIG};
    driverSetBusFilters(BUS_VEHICLE, vehIds, 4);
  }
#endif

#if BUS_BODY_ACTIVE
  // Bus 2 (Body bus, X179 pins 2-3): body control frames
  if (s.rawCanListen) {
    driverSetBusFilters(BUS_BODY, nullptr, 0);
  } else {
    static const uint32_t bodyIds[] = {CAN_ID_WINDOW_VENT, CAN_ID_SENTRY, CAN_ID_TRUNK_CTRL};
    driverSetBusFilters(BUS_BODY, bodyIds, 3);
  }
#endif
}

// ── Summon Tick ──────────────────────────────────────────────────────────────
void summonTick(State& s) {
#if !BUS_VEHICLE_ACTIVE
  (void)s;
  return;
#else
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
  driverSend(f, BUS_VEHICLE);
  s.summonRemaining--;
  if (s.summonRemaining == 0) sendLog(F("Summon burst complete"));
#endif
}

// ── Message Dispatch ─────────────────────────────────────────────────────────
void handleMessage(Frame& f, uint8_t bus, State& s) {
  // Bus 0 (FSD): variant-specific FSD frame processing
  if (bus == BUS_FSD) {
    switch(s.variant) {
      case HW4: handleHW4(f, s); break;
      case HW3: handleHW3(f, s); break;
      case LEGACY: handleLegacy(f, s); break;
    }
    return;
  }

#if BUS_VEHICLE_ACTIVE
  // Bus 1 (Vehicle): cache control frames for vehicle commands
  if (bus == BUS_VEHICLE) {
    if (f.id == CAN_ID_UI_VEHICLE_CTRL && f.dlc >= 8) {
      memcpy(s.lastCtrl, f.data, 8);
      s.hasCtrl = true;
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
  }

  // Bus 2 (Body): no caching needed, body commands generate fresh frames
#endif
}
