#pragma once
#include "core/types.h"
#include "protocol/can.h"
#include "protocol/vehicle.h"
#include "core/driver.h"

// ── Light Control (0x273) ────────────────────────────────────────────────────

static void controlFrontFog(State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setFrontFogSwitch(f, true);
  
  for (uint8_t i = 0; i < 30; i++) {
#if BOARD_ENABLE_MCP2515_2
    driverSend(f, s.ctrlBus);
#else
    driverSend(f);
#endif
    delay(20);
  }
}

static void controlRearFog(State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setRearFogSwitch(f, true);
  
  for (uint8_t i = 0; i < 30; i++) {
#if BOARD_ENABLE_MCP2515_2
    driverSend(f, s.ctrlBus);
#else
    driverSend(f);
#endif
    delay(20);
  }
}

static void controlAutoHighBeam(State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setAutoHighBeam(f, true);
  
  for (uint8_t i = 0; i < 30; i++) {
#if BOARD_ENABLE_MCP2515_2
    driverSend(f, s.ctrlBus);
#else
    driverSend(f);
#endif
    delay(20);
  }
}

static void controlAmbientLight(State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setAmbientLighting(f, true);
  
  for (uint8_t i = 0; i < 30; i++) {
#if BOARD_ENABLE_MCP2515_2
    driverSend(f, s.ctrlBus);
#else
    driverSend(f);
#endif
    delay(20);
  }
}

static void controlHomeLight(State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setSeeYouHomeLighting(f, true);
  
  for (uint8_t i = 0; i < 30; i++) {
#if BOARD_ENABLE_MCP2515_2
    driverSend(f, s.ctrlBus);
#else
    driverSend(f);
#endif
    delay(20);
  }
}

static void controlDomeLight(DomeLightSwitch mode, State& s) {
  Frame f = { CAN_ID_UI_VEHICLE_CTRL, 8 };
  memcpy(f.data, s.lastCtrl, 8);
  setDomeLightSwitch(f, mode);
  
  for (uint8_t i = 0; i < 30; i++) {
#if BOARD_ENABLE_MCP2515_2
    driverSend(f, s.ctrlBus);
#else
    driverSend(f);
#endif
    delay(20);
  }
}
