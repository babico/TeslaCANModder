#pragma once
#include <Arduino.h>
#include "core/config.h"
#include "core/types.h"

// Forward declarations for logging (used by handlers included below)
void sendLog(const char* msg);
void sendLog(const __FlashStringHelper* msg);

#include "command/system.h"
#include "command/fsd.h"
#include "command/vehicle.h"
#include "command/window.h"
#include "command/sentry.h"
#include "command/climate.h"
#include "command/charge.h"
#include "command/drive.h"

#if BOARD_ENABLE_BT
  #include <SoftwareSerial.h>
  static SoftwareSerial btSerial(PIN_BT_RX, PIN_BT_TX);
#endif

static char usbBuf[32];
static uint8_t usbLen = 0;
#if BOARD_ENABLE_BT
static char btBuf[32];
static uint8_t btLen = 0;
#endif
static unsigned long lastStatusMs = 0;

// ── Output Helpers ───────────────────────────────────────────────────────────
void printStr(const char* s) {
  Serial.print(s);
#if BOARD_ENABLE_BT
  btSerial.print(s);
#endif
}

void printStr(const __FlashStringHelper* s) {
  Serial.print(s);
#if BOARD_ENABLE_BT
  btSerial.print(s);
#endif
}

void printNum(long n) {
  Serial.print(n);
#if BOARD_ENABLE_BT
  btSerial.print(n);
#endif
}

void printHex(uint8_t b) {
  if (b < 0x10) printStr("0");
  Serial.print(b, HEX);
#if BOARD_ENABLE_BT
  btSerial.print(b, HEX);
#endif
}

void printLn() {
  Serial.println();
#if BOARD_ENABLE_BT
  btSerial.println();
#endif
}

// ── JSON Messages ────────────────────────────────────────────────────────────
void sendBoot(State& s) {
  Features f = s.features();
  printStr(F("{\"t\":\"boot\",\"hw\":\""));
  printStr(F(BOARD_HW_NAME));
  printStr(F("\",\"can\":\""));
  printStr(F(BOARD_CAN_NAME));
  printStr(F("\",\"drv\":\""));
  printStr(F(BOARD_DRIVER_NAME));
  printStr(F("\",\"variant\":\""));
  printStr(variantName(s.variant));
  printStr(F("\",\"cap\":\""));
#if BOARD_ENABLE_BT
  printStr(F("usb+bluetooth"));
#else
  printStr(F("usb"));
#endif
  printStr(F("\",\"ready\":\"runtime-ready\",\"btEnabled\":"));
#if BOARD_ENABLE_BT
  printStr(F("1"));
#else
  printStr(F("0"));
#endif
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
  printStr(F(",\"bus2\":"));
#if BOARD_ENABLE_MCP2515_2
  extern bool bus2Available;
  printNum(bus2Available ? 1 : 0);
#else
  printStr(F("0"));
#endif
  printStr(F(",\"canOnline\":"));
  printNum(s.canOnline ? 1 : 0);
  printStr(F(",\"standby\":"));
  printNum(s.standby ? 1 : 0);
#if BOARD_ENABLE_BT
  printStr(F(",\"bt\":\""));
  printStr(F(BOARD_BT_NAME));
  printStr(F("\""));
#endif
  printStr(F("}"));
  printLn();
}

void sendStatus(State& s, unsigned long now) {
  Features f = s.features();
  printStr(F("{\"t\":\"status\",\"hw\":\""));
  printStr(F(BOARD_HW_NAME));
  printStr(F("\",\"can\":\""));
  printStr(F(BOARD_CAN_NAME));
  printStr(F("\",\"drv\":\""));
  printStr(F(BOARD_DRIVER_NAME));
  printStr(F("\",\"variant\":\""));
  printStr(variantName(s.variant));
  printStr(F("\",\"cap\":\""));
#if BOARD_ENABLE_BT
  printStr(F("usb+bluetooth"));
#else
  printStr(F("usb"));
#endif
  printStr(F("\",\"ready\":\"runtime-ready\",\"bt\":"));
#if BOARD_ENABLE_BT
  printStr(F("1"));
#else
  printStr(F("0"));
#endif
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
  printStr(F(",\"nag\":"));
  printNum(s.nagSuppress ? 1 : 0);
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
  printStr(F(",\"bus2\":"));
#if BOARD_ENABLE_MCP2515_2
  extern bool bus2Available;
  printNum(bus2Available ? 1 : 0);
#else
  printStr(F("0"));
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
  // System commands
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
    sendLog(s.fsdEnabled ? F("FSD enabled - saved to EEPROM") : F("FSD disabled - saved to EEPROM"));
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
  
  // Try advanced commands first (don't need 0x273)
  if (execWindowCmd(cmd, s) ||
      execSentryCmd(cmd, s) ||
      execClimateCmd(cmd, s) ||
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
  
  if (executeVehicleCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(cmd);
    sendStatus(s, now);
  } else {
    sendError(F("Unknown command"));
  }
}

void handleChar(char* buf, uint8_t& len, char c, State& s) {
  if (c == '\r') return;

  if (c == '\n') {
    if (len > 0 && len < 32) {      // drop empty lines + overflowed (poisoned) buffers
      buf[len] = '\0';
      executeCommand(buf, s, millis());
    }
    len = 0;
    return;
  }

  // Allow only safe command characters
  bool valid = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
               (c >= '0' && c <= '9') || c == ':' || c == '-' || c == '_';
  if (!valid) { len = 0; return; }

  if (len < 31) {
    buf[len++] = c;
  } else {
    len = 32;  // poison the buffer; will be cleared at next '\n'
  }
}

// ── Serial API ───────────────────────────────────────────────────────────────
void serialInit(State& s) {
#if BOARD_ENABLE_BT
  btSerial.begin(BT_BAUD);
#endif
  delay(1000);
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {}
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
  
  // Bluetooth commands
#if BOARD_ENABLE_BT
  while (btSerial.available()) {
    char c = btSerial.read();
    handleChar(btBuf, btLen, c, s);
  }
#endif
}
