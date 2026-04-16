#pragma once
#include <Arduino.h>
#include "core/config/esp32.h"
#include "core/types.h"
#include "protocol/can.h"

// Forward declarations for logging (used by handlers included below)
void sendLog(const char* msg);
void sendLog(const __FlashStringHelper* msg);

#include "command/system.h"
#include "command/fsd_toggle.h"
#include "command/nag.h"
#include "command/nag_killer.h"
#include "command/profile.h"
#include "command/offset.h"
#include "command/isa_chime.h"
#include "command/summon_inject.h"
#include "command/summon_cmd.h"
#include "command/variant.h"
#include "command/bms.h"
#if BUS_VEHICLE_ACTIVE
  #include "command/mirror.h"
  #include "command/lock.h"
  #include "command/light.h"
  #include "command/wiper.h"
  #include "command/seat.h"
  #include "command/display.h"
  #include "command/power.h"
  #include "command/climate.h"
  #include "command/charge.h"
  #include "command/drive.h"
  #include "command/precondition.h"
  #include "command/track_mode.h"
#endif
#if BUS_BODY_ACTIVE
  #include "command/window.h"
  #include "command/sentry.h"
#endif
#if BUS_VEHICLE_ACTIVE || BUS_BODY_ACTIVE
  #include "command/trunk.h"
#endif

#if BOARD_ENABLE_BLE
  #include "io/ble/esp32.h"
#endif

static char usbBuf[32];
static uint8_t usbLen = 0;
#if BOARD_ENABLE_BLE
static char bleBuf[32];
static uint8_t bleLen = 0;
#endif
static unsigned long lastStatusMs = 0;

// ── Output Helpers ───────────────────────────────────────────────────────────
void printStr(const char* s) {
  Serial.print(s);
#if BOARD_ENABLE_BLE
  blePrint(s);
#endif
}

void printStr(const __FlashStringHelper* s) {
  Serial.print(s);
#if BOARD_ENABLE_BLE
  blePrint(s);
#endif
}

void printNum(long n) {
  Serial.print(n);
#if BOARD_ENABLE_BLE
  blePrintNum(n);
#endif
}

void printHex(uint8_t b) {
  if (b < 0x10) printStr("0");
  Serial.print(b, HEX);
#if BOARD_ENABLE_BLE
  blePrintHex(b);
#endif
}

void printLn() {
  Serial.println();
#if BOARD_ENABLE_BLE
  blePrintLn();
#endif
}

// ── JSON Messages ────────────────────────────────────────────────────────────
void sendBoot(State& s) {
  Features f = getFeatures(s.variant);
  printStr(F("{\"t\":\"boot\",\"hw\":\""));
  printStr(F(BOARD_HW_NAME));
  printStr(F("\",\"can\":\""));
  printStr(F(BOARD_CAN_NAME));
  printStr(F("\",\"drv\":\""));
  printStr(F(BOARD_DRIVER_NAME));
  printStr(F("\",\"variant\":\""));
  printStr(variantName(s.variant));
  printStr(F("\",\"cap\":\""));
#if BOARD_ENABLE_WIFI && BOARD_ENABLE_BLE
  printStr(F("usb+wifi+ble"));
#elif BOARD_ENABLE_WIFI
  printStr(F("usb+wifi"));
#elif BOARD_ENABLE_BLE
  printStr(F("usb+ble"));
#else
  printStr(F("usb"));
#endif
  printStr(F("\",\"ready\":\"runtime-ready\",\"btEnabled\":"));
#if BOARD_ENABLE_BLE
  printStr(F("1"));
#else
  printStr(F("0"));
#endif
  printStr(F(",\"wifiEnabled\":"));
#if BOARD_ENABLE_WIFI
  printStr(F("1"));
#else
  printStr(F("0"));
#endif
  printStr(F(",\"busFsd\":"));
  printNum(BUS_FSD_ACTIVE);
  printStr(F(",\"busVehicle\":"));
  printNum(BUS_VEHICLE_ACTIVE);
  printStr(F(",\"busBody\":"));
  printNum(BUS_BODY_ACTIVE);
  printStr(F(",\"fsd\":"));
  printNum(s.fsdEnabled ? 1 : 0);
  printStr(F(",\"nag\":"));
  printNum(s.nagSuppress ? 1 : 0);
  printStr(F(",\"sp\":"));
  printNum(s.speedProfile);
  printStr(F(",\"spPin\":"));
  printNum(s.profileOverride ? 1 : 0);
  printStr(F(",\"offset\":"));
  printNum(s.speedOffset);
  printStr(F(",\"offsetPin\":"));
  printNum(s.offsetOverride ? 1 : 0);
  printStr(F(",\"isaChime\":")); 
  printNum(s.isaChimeSuppress ? 1 : 0);
  printStr(F(",\"summonInject\":"));
  printNum(s.summonInject ? 1 : 0);
  printStr(F(",\"nagKiller\":"));
  printNum(s.nagKillerEnabled ? 1 : 0);
  printStr(F(",\"precondition\":"));
  printNum(s.preconditionEnabled ? 1 : 0);
  printStr(F(",\"trackMode\":"));
  printNum(s.trackModeEnabled ? 1 : 0);
  printStr(F(",\"otaInProgress\":"));
  printNum(s.otaInProgress ? 1 : 0);
  printStr(F(",\"txPaused\":"));
  printNum(s.txPaused ? 1 : 0);
  printStr(F(",\"detectedHW\":"));
  printNum(s.detectedHW);
  printStr(F(",\"features\":{\"fsd\":"));
  printNum(f.fsd ? 1 : 0);
  printStr(F(",\"profile\":"));
  printNum(f.profile ? 1 : 0);
  printStr(F(",\"nag\":"));
  printNum(f.nag ? 1 : 0);
  printStr(F(",\"speedOffset\":"));
  printNum(f.speedOffset ? 1 : 0);
  printStr(F(",\"isaSpeedChime\":"));
  printNum(f.isaChime ? 1 : 0);
  printStr(F(",\"summon\":"));
  printNum(f.summon ? 1 : 0);
  printStr(F(",\"forceFsd\":0},\"stream\":{\"on\":0,\"emitted\":0},\"rawCan\":"));
  printNum(s.rawCanListen ? 1 : 0);
  extern bool mcpAvailable[];
  printStr(F(",\"bus1\":"));
  printNum(mcpAvailable[0] ? 1 : 0);
#if BUS_VEHICLE_ACTIVE
  printStr(F(",\"bus2\":"));
  printNum(mcpAvailable[1] ? 1 : 0);
#else
  printStr(F(",\"bus2\":0"));
#endif
#if BUS_BODY_ACTIVE
  printStr(F(",\"bus3\":"));
  printNum(mcpAvailable[2] ? 1 : 0);
#else
  printStr(F(",\"bus3\":0"));
#endif
  printStr(F(",\"canOnline\":"));
  printNum(s.canOnline ? 1 : 0);
  printStr(F(",\"standby\":"));
  printNum(s.standby ? 1 : 0);
#if BOARD_ENABLE_BLE
  printStr(F(",\"ble\":\""));
  printStr(F(BLE_DEVICE_NAME));
  printStr(F("\""));
#endif
#if BOARD_ENABLE_WIFI
  printStr(F(",\"wifi\":{\"ssid\":\""));
  printStr(WIFI_AP_SSID);
  printStr(F("\",\"port\":"));
  printNum(WIFI_REST_PORT);
  printStr(F("}"));
#endif
  printStr(F("}"));
  printLn();
}

void sendStatus(State& s, unsigned long now) {
  Features f = getFeatures(s.variant);
  printStr(F("{\"t\":\"status\",\"hw\":\""));
  printStr(F(BOARD_HW_NAME));
  printStr(F("\",\"can\":\""));
  printStr(F(BOARD_CAN_NAME));
  printStr(F("\",\"drv\":\""));
  printStr(F(BOARD_DRIVER_NAME));
  printStr(F("\",\"variant\":\""));
  printStr(variantName(s.variant));
  printStr(F("\",\"cap\":\""));
#if BOARD_ENABLE_WIFI && BOARD_ENABLE_BLE
  printStr(F("usb+wifi+ble"));
#elif BOARD_ENABLE_WIFI
  printStr(F("usb+wifi"));
#elif BOARD_ENABLE_BLE
  printStr(F("usb+ble"));
#else
  printStr(F("usb"));
#endif
  printStr(F("\",\"ready\":\"runtime-ready\",\"bt\":"));
#if BOARD_ENABLE_BLE
  printStr(F("1"));
#else
  printStr(F("0"));
#endif
  printStr(F(",\"wifi\":"));
#if BOARD_ENABLE_WIFI
  printStr(F("1"));
#else
  printStr(F("0"));
#endif
  printStr(F(",\"busFsd\":"));
  printNum(BUS_FSD_ACTIVE);
  printStr(F(",\"busVehicle\":"));
  printNum(BUS_VEHICLE_ACTIVE);
  printStr(F(",\"busBody\":"));
  printNum(BUS_BODY_ACTIVE);
  printStr(F(",\"fsd\":"));
  printNum(s.fsdEnabled ? 1 : 0);
  printStr(F(",\"sp\":"));
  printNum(s.speedProfile);
  printStr(F(",\"spPin\":"));
  printNum(s.profileOverride ? 1 : 0);
  printStr(F(",\"offset\":"));
  printNum(s.speedOffset);
  printStr(F(",\"offsetPin\":"));
  printNum(s.offsetOverride ? 1 : 0);
  printStr(F(",\"isaChime\":"));
  printNum(s.isaChimeSuppress ? 1 : 0);
  printStr(F(",\"summonInject\":"));
  printNum(s.summonInject ? 1 : 0);
  printStr(F(",\"nag\":"));
  printNum(s.nagSuppress ? 1 : 0);
  printStr(F(",\"nagKiller\":"));
  printNum(s.nagKillerEnabled ? 1 : 0);
  printStr(F(",\"precondition\":"));
  printNum(s.preconditionEnabled ? 1 : 0);
  printStr(F(",\"trackMode\":"));
  printNum(s.trackModeEnabled ? 1 : 0);
  printStr(F(",\"otaInProgress\":"));
  printNum(s.otaInProgress ? 1 : 0);
  printStr(F(",\"txPaused\":"));
  printNum(s.txPaused ? 1 : 0);
  printStr(F(",\"detectedHW\":"));
  printNum(s.detectedHW);
  printStr(F(",\"features\":{\"fsd\":"));
  printNum(f.fsd ? 1 : 0);
  printStr(F(",\"profile\":"));
  printNum(f.profile ? 1 : 0);
  printStr(F(",\"nag\":"));
  printNum(f.nag ? 1 : 0);
  printStr(F(",\"speedOffset\":"));
  printNum(f.speedOffset ? 1 : 0);
  printStr(F(",\"isaSpeedChime\":"));
  printNum(f.isaChime ? 1 : 0);
  printStr(F(",\"summon\":"));
  printNum(f.summon ? 1 : 0);
  printStr(F(",\"forceFsd\":0},\"stream\":{\"on\":"));
  printNum(s.streamEnabled ? 1 : 0);
  printStr(F(",\"emitted\":"));
  printNum(s.streamCount);
  printStr(F("},\"rawCan\":"));
  printNum(s.rawCanListen ? 1 : 0);
  extern bool mcpAvailable[];
  printStr(F(",\"bus1\":"));
  printNum(mcpAvailable[0] ? 1 : 0);
#if BUS_VEHICLE_ACTIVE
  printStr(F(",\"bus2\":"));
  printNum(mcpAvailable[1] ? 1 : 0);
#else
  printStr(F(",\"bus2\":0"));
#endif
#if BUS_BODY_ACTIVE
  printStr(F(",\"bus3\":"));
  printNum(mcpAvailable[2] ? 1 : 0);
#else
  printStr(F(",\"bus3\":0"));
#endif
  printStr(F(",\"canOnline\":"));
  printNum(s.canOnline ? 1 : 0);
  printStr(F(",\"standby\":"));
  printNum(s.standby ? 1 : 0);
  printStr(F(",\"up\":"));
  printNum(now);
  printStr(F("}"));
  printLn();
}

void sendAck(const char* cmd) {
  printStr(F("{\"t\":\"ack\",\"cmd\":\""));
  printStr(cmd);
  printStr(F("\"}"));
  printLn();
}

void sendError(const char* msg) {
  printStr(F("{\"t\":\"error\",\"msg\":\""));
  printStr(msg);
  printStr(F("\"}"));
  printLn();
}

void sendError(const __FlashStringHelper* msg) {
  printStr(F("{\"t\":\"error\",\"msg\":\""));
  printStr(msg);
  printStr(F("\"}"));
  printLn();
}

void sendLog(const char* msg) {
  printStr(F("{\"t\":\"log\",\"msg\":\""));
  printStr(msg);
  printStr(F("\"}"));
  printLn();
}

void sendLog(const __FlashStringHelper* msg) {
  printStr(F("{\"t\":\"log\",\"msg\":\""));
  printStr(msg);
  printStr(F("\"}"));
  printLn();
}

void sendFrame(const Frame& f, const char* dir, uint8_t bus, unsigned long ms, State& s) {
  if (!s.streamEnabled) return;
  s.streamCount++;

  printStr(F("{\"t\":\"frame\",\"dir\":\""));
  printStr(dir);
  printStr(F("\",\"bus\":"));
  printNum(bus);
  printStr(F(",\"id\":"));
  printNum(f.id);
  printStr(F(",\"seq\":"));
  printNum(s.streamCount);
  printStr(F(",\"ms\":"));
  printNum(ms);
  printStr(F(",\"ext\":0,\"dlc\":"));
  printNum(f.dlc);
  printStr(F(",\"d\":\""));
  for (uint8_t i = 0; i < f.dlc; i++) printHex(f.data[i]);
  printStr(F("\"}"));
  printLn();
}

// ── Command Parser ───────────────────────────────────────────────────────────
void executeCommand(const char* cmd, State& s, unsigned long now) {
  if (strcmp(cmd, "ping") == 0) {
    printStr(F("{\"t\":\"pong\",\"v\":1}")); printLn();
    return;
  }

  if (strcmp(cmd, "status") == 0) {
    sendStatus(s, now);
    return;
  }

  if (executeStreamCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.streamEnabled ? F("Stream started") : F("Stream stopped"));
    sendStatus(s, now);
    return;
  }

  if (executeCanRawCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.rawCanListen ? F("Raw CAN mode enabled") : F("Filtered CAN mode"));
    sendStatus(s, now);
    return;
  }

  if (executeFsdCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.fsdEnabled ? F("FSD enabled - saved to NVS") : F("FSD disabled - saved to NVS"));
    sendStatus(s, now);
    return;
  }

  if (executeNagCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.nagSuppress ? F("Nag suppress ON - saved") : F("Nag suppress OFF - saved"));
    sendStatus(s, now);
    return;
  }

  if (executeProfileCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.profileOverride ? F("Profile pinned - saved") : F("Profile set to auto"));
    sendStatus(s, now);
    return;
  }

  if (executeOffsetCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.offsetOverride ? F("Offset pinned - saved") : F("Offset set to auto"));
    sendStatus(s, now);
    return;
  }

  if (executeIsaChimeCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.isaChimeSuppress ? F("ISA chime suppressed - saved") : F("ISA chime original - saved"));
    sendStatus(s, now);
    return;
  }

  if (executeSummonInjectCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.summonInject ? F("Summon injection ON - saved") : F("Summon injection OFF - saved"));
    sendStatus(s, now);
    return;
  }

  if (executeSummonCmd(cmd, s)) {
    sendAck(cmd);
    if (s.summonRemaining > 0)
      sendLog(F("Summon burst started (30 frames)"));
    else
      sendLog(F("Summon stopped"));
    sendStatus(s, now);
    return;
  }

  if (executeVariantCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(F("Variant changed - filters updated"));
    sendStatus(s, now);
    return;
  }

  if (executeNagKillerCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.nagKillerEnabled ? F("Nag killer ON - saved") : F("Nag killer OFF - saved"));
    sendStatus(s, now);
    return;
  }

  if (execBmsCmd(cmd, s)) {
    // Send BMS telemetry as JSON
    printStr(F("{\"t\":\"bms\",\"v\":"));
    printNum((long)(s.bmsVoltage * 100));
    printStr(F(",\"a\":"));
    printNum((long)(s.bmsCurrent * 10));
    printStr(F(",\"kw\":"));
    printNum((long)(s.bmsPower * 10));
    printStr(F(",\"soc\":"));
    printNum((long)(s.bmsSoc * 10));
    printStr(F(",\"tMin\":"));
    printNum(s.bmsTempMin);
    printStr(F(",\"tMax\":"));
    printNum(s.bmsTempMax);
    printStr(F(",\"whkm\":"));
    printNum((long)(s.bmsWhPerKm * 10));
    printStr(F(",\"ok\":"));
    printNum(s.hasBms ? 1 : 0);
    printStr(F("}"));
    printLn();
    return;
  }

  // Body bus commands (window, sentry)
#if BUS_BODY_ACTIVE
  if (execWindowCmd(cmd, s) ||
      execSentryCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(cmd);
    sendStatus(s, now);
    return;
  }
#endif

  // Vehicle bus commands (climate, charge, drive, precondition, track mode)
#if BUS_VEHICLE_ACTIVE
  if (execPreconditionCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.preconditionEnabled ? F("Precondition ON - saved") : F("Precondition OFF - saved"));
    sendStatus(s, now);
    return;
  }

  if (execTrackModeCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.trackModeEnabled ? F("Track mode ON - saved") : F("Track mode OFF - saved"));
    sendStatus(s, now);
    return;
  }

  if (execClimateCmd(cmd, s) ||
      execChargeCmd(cmd, s) ||
      execDriveCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(cmd);
    sendStatus(s, now);
    return;
  }

  if (!s.hasCtrl) {
    sendError(F("Waiting for 0x273 frame"));
    return;
  }

  if (s.variant != LEGACY && (
      execMirrorCmd(cmd, s) || execLockCmd(cmd, s) ||
      execLightCmd(cmd, s) || execWiperCmd(cmd, s) || execSeatCmd(cmd, s) ||
      execDisplayCmd(cmd, s) || execPowerCmd(cmd, s))) {
    sendAck(cmd);
    sendLog(cmd);
    sendStatus(s, now);
    return;
  }
#endif

  // Trunk commands (frunk = vehicle bus, trunk/glovebox = body bus)
#if BUS_VEHICLE_ACTIVE || BUS_BODY_ACTIVE
  if (s.variant != LEGACY && execTrunkCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(cmd);
    sendStatus(s, now);
    return;
  }
#endif
#endif

  sendError(F("Unknown command"));
}

void handleChar(char* buf, uint8_t& len, char c, State& s) {
  if (c == '\r') return;

  if (c == '\n') {
    if (len > 0 && len < 32) {
      buf[len] = '\0';
      executeCommand(buf, s, millis());
    }
    len = 0;
    return;
  }

  bool valid = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
               (c >= '0' && c <= '9') || c == ':' || c == '-' || c == '_';
  if (!valid) { len = 0; return; }

  if (len < 31) {
    buf[len++] = c;
  } else {
    len = 32;
  }
}

// ── Serial API ───────────────────────────────────────────────────────────────
void serialInit(State& s) {
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {}

#if BOARD_ENABLE_BLE
  bleInit();
#endif

  sendLog(F(BOARD_READY_MSG));
  sendBoot(s);
}

void serialTick(State& s) {
  unsigned long now = millis();

  // Periodic status
  if (now - lastStatusMs >= STATUS_INTERVAL_MS) {
    lastStatusMs = now;
    sendStatus(s, now);
  }

  // USB commands
  while (Serial.available()) {
    char c = Serial.read();
    handleChar(usbBuf, usbLen, c, s);
  }

  // BLE commands
#if BOARD_ENABLE_BLE
  while (bleAvailable()) {
    char c = bleRead();
    handleChar(bleBuf, bleLen, c, s);
  }
#endif
}
