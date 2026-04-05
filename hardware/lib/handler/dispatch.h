#pragma once
#include "protocol/can.h"
#include "protocol/summon.h"
#include "handler/hw4.h"
#include "handler/hw3.h"
#include "handler/legacy.h"
#include "driver.h"

// ── Filter Setup ─────────────────────────────────────────────────────────────
void applyFilters(State& s) {
#if BOARD_ENABLE_MCP2515_2
  if (s.rawCanListen) {
    driverSetFilters2(nullptr, 0);
  } else {
    static const uint32_t bus2Ids[] = {CAN_ID_CLIMATE, CAN_ID_CHARGE, CAN_ID_DRIVE_CONFIG, CAN_ID_UI_VEHICLE_CTRL};
    driverSetFilters2(bus2Ids, 4);
  }
#endif

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
  if (s.summonRemaining == 0 || !s.hasCtrl) return;
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
#if BOARD_ENABLE_MCP2515_2
  driverSend(f, s.ctrlBus);
#else
  driverSend(f);
#endif
  s.summonRemaining--;
  if (s.summonRemaining == 0) sendLog("Summon burst complete");
}

// ── Message Dispatch ─────────────────────────────────────────────────────────
void handleMessage(Frame& f, uint8_t bus, State& s) {
  // Cache latest UI_vehicleControl frame for summon injection
  if (f.id == CAN_ID_UI_VEHICLE_CTRL && f.dlc >= 8) {
    memcpy(s.lastCtrl, f.data, 8);
    s.hasCtrl = true;
#if BOARD_ENABLE_MCP2515_2
    s.ctrlBus = bus;
#endif
    return;
  }
  
  // Cache climate frame (0x2F3)
  if (f.id == CAN_ID_CLIMATE && f.dlc >= 5) {
    memcpy(s.lastClimate, f.data, 5);
    s.hasClimate = true;
    return;
  }
  
  // Cache charge frame (0x333)
  if (f.id == CAN_ID_CHARGE && f.dlc >= 5) {
    memcpy(s.lastCharge, f.data, 5);
    s.hasCharge = true;
    return;
  }
  
  // Cache drive config frame (0x334)
  if (f.id == CAN_ID_DRIVE_CONFIG && f.dlc >= 8) {
    memcpy(s.lastDrive, f.data, 8);
    s.hasDrive = true;
    return;
  }

#if BOARD_ENABLE_MCP2515_2
  if (bus != 0) return;
#endif

  switch(s.variant) {
    case HW4: handleHW4(f, s); break;
    case HW3: handleHW3(f, s); break;
    case LEGACY: handleLegacy(f, s); break;
  }
}
