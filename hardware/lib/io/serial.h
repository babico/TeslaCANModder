#pragma once
#include <Arduino.h>
#include "core/config.h"
#include "core/types.h"

// Forward declarations for logging (used by handlers included below)
void sendLog(const char* msg);

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
static char btBuf[32];
static uint8_t btLen = 0;
static unsigned long lastStatusMs = 0;

// ── Output Helpers ───────────────────────────────────────────────────────────
void printStr(const char* s) {
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
  printStr("{\"t\":\"boot\",\"hw\":\"");
  printStr(BOARD_HW_NAME);
  printStr("\",\"can\":\"");
  printStr(BOARD_CAN_NAME);
  printStr("\",\"drv\":\"");
  printStr(BOARD_DRIVER_NAME);
  printStr("\",\"variant\":\"");
  printStr(variantName(s.variant));
  printStr("\",\"cap\":\"");
#if BOARD_ENABLE_BT
  printStr("usb+bluetooth");
#else
  printStr("usb");
#endif
  printStr("\",\"ready\":\"runtime-ready\",\"btEnabled\":");
#if BOARD_ENABLE_BT
  printStr("1");
#else
  printStr("0");
#endif
  printStr(",\"fsd\":");
  printNum(s.fsdEnabled ? 1 : 0);
  printStr(",\"nag\":");
  printNum(s.nagSuppress ? 1 : 0);
  printStr(",\"sp\":");
  printNum(s.speedProfile);
  printStr(",\"spPin\":");
  printNum(s.profileOverride ? 1 : 0);
  printStr(",\"offset\":");
  printNum(s.speedOffset);
  printStr(",\"offsetPin\":");
  printNum(s.offsetOverride ? 1 : 0);
  printStr(",\"isaChime\":");
  printNum(s.isaChimeSuppress ? 1 : 0);
  printStr(",\"features\":{\"fsd\":");
  printNum(f.fsd ? 1 : 0);
  printStr(",\"profile\":");
  printNum(f.profile ? 1 : 0);
  printStr(",\"nag\":");
  printNum(f.nag ? 1 : 0);
  printStr(",\"speedOffset\":");
  printNum(f.speedOffset ? 1 : 0);
  printStr(",\"isaSpeedChime\":");
  printNum(f.isaChime ? 1 : 0);
  printStr(",\"summon\":");
  printNum(f.summon ? 1 : 0);
  printStr(",\"forceFsd\":0},\"stream\":{\"on\":0,\"emitted\":0},\"rawCan\":");
  printNum(s.rawCanListen ? 1 : 0);
  printStr(",\"bus2\":");
#if BOARD_ENABLE_MCP2515_2
  extern bool bus2Available;
  printNum(bus2Available ? 1 : 0);
#else
  printStr("0");
#endif
  printStr(",\"canOnline\":");
  printNum(s.canOnline ? 1 : 0);
  printStr(",\"standby\":");
  printNum(s.standby ? 1 : 0);
#if BOARD_ENABLE_BT
  printStr(",\"bt\":\"");
  printStr(BOARD_BT_NAME);
  printStr("\"");
#endif
  printStr("}");
  printLn();
}

void sendStatus(State& s, unsigned long now) {
  Features f = s.features();
  printStr("{\"t\":\"status\",\"hw\":\"");
  printStr(BOARD_HW_NAME);
  printStr("\",\"can\":\"");
  printStr(BOARD_CAN_NAME);
  printStr("\",\"drv\":\"");
  printStr(BOARD_DRIVER_NAME);
  printStr("\",\"variant\":\"");
  printStr(variantName(s.variant));
  printStr("\",\"cap\":\"");
#if BOARD_ENABLE_BT
  printStr("usb+bluetooth");
#else
  printStr("usb");
#endif
  printStr("\",\"ready\":\"runtime-ready\",\"bt\":");
#if BOARD_ENABLE_BT
  printStr("1");
#else
  printStr("0");
#endif
  printStr(",\"fsd\":");
  printNum(s.fsdEnabled ? 1 : 0);
  printStr(",\"sp\":");
  printNum(s.speedProfile);
  printStr(",\"spPin\":");
  printNum(s.profileOverride ? 1 : 0);
  printStr(",\"offset\":");
  printNum(s.speedOffset);
  printStr(",\"offsetPin\":");
  printNum(s.offsetOverride ? 1 : 0);
  printStr(",\"isaChime\":");
  printNum(s.isaChimeSuppress ? 1 : 0);
  printStr(",\"nag\":");
  printNum(s.nagSuppress ? 1 : 0);
  printStr(",\"features\":{\"fsd\":");
  printNum(f.fsd ? 1 : 0);
  printStr(",\"profile\":");
  printNum(f.profile ? 1 : 0);
  printStr(",\"nag\":");
  printNum(f.nag ? 1 : 0);
  printStr(",\"speedOffset\":");
  printNum(f.speedOffset ? 1 : 0);
  printStr(",\"isaSpeedChime\":");
  printNum(f.isaChime ? 1 : 0);
  printStr(",\"summon\":");
  printNum(f.summon ? 1 : 0);
  printStr(",\"forceFsd\":0},\"stream\":{\"on\":");
  printNum(s.streamEnabled ? 1 : 0);
  printStr(",\"emitted\":");
  printNum(s.streamCount);
  printStr("},\"rawCan\":");
  printNum(s.rawCanListen ? 1 : 0);
  printStr(",\"bus2\":");
#if BOARD_ENABLE_MCP2515_2
  extern bool bus2Available;
  printNum(bus2Available ? 1 : 0);
#else
  printStr("0");
#endif
  printStr(",\"canOnline\":");
  printNum(s.canOnline ? 1 : 0);
  printStr(",\"standby\":");
  printNum(s.standby ? 1 : 0);
  printStr(",\"up\":");
  printNum(now);
  printStr("}");
  printLn();
}

void sendAck(const char* cmd) {
  printStr("{\"t\":\"ack\",\"cmd\":\"");
  printStr(cmd);
  printStr("\"}");
  printLn();
}

void sendError(const char* msg) {
  printStr("{\"t\":\"error\",\"msg\":\"");
  printStr(msg);
  printStr("\"}");
  printLn();
}

void sendLog(const char* msg) {
  printStr("{\"t\":\"log\",\"msg\":\"");
  printStr(msg);
  printStr("\"}");
  printLn();
}

void sendFrame(const Frame& f, const char* dir, uint8_t bus, unsigned long ms, State& s) {
  if (!s.streamEnabled) return;
  s.streamCount++;

  printStr("{\"t\":\"frame\",\"dir\":\"");
  printStr(dir);
  printStr("\",\"bus\":");
  printNum(bus);
  printStr(",\"id\":");
  printNum(f.id);
  printStr(",\"seq\":");
  printNum(s.streamCount);
  printStr(",\"ms\":");
  printNum(ms);
  printStr(",\"ext\":0,\"dlc\":");
  printNum(f.dlc);
  printStr(",\"d\":\"");
  for (uint8_t i = 0; i < f.dlc; i++) printHex(f.data[i]);
  printStr("\"}");
  printLn();
}

// ── Command Parser ───────────────────────────────────────────────────────────
void executeCommand(const char* cmd, State& s, unsigned long now) {
  // System commands
  if (strcmp(cmd, "ping") == 0) {
    printStr("{\"t\":\"pong\",\"v\":1}"); printLn();
    return;
  }
  
  if (strcmp(cmd, "status") == 0) {
    sendStatus(s, now);
    return;
  }
  
  if (executeStreamCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.streamEnabled ? "Stream started" : "Stream stopped");
    sendStatus(s, now);
    return;
  }
  
  if (executeCanRawCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.rawCanListen ? "Raw CAN mode enabled" : "Filtered CAN mode");
    sendStatus(s, now);
    return;
  }

  if (executeFsdCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.fsdEnabled ? "FSD enabled - saved to EEPROM" : "FSD disabled - saved to EEPROM");
    sendStatus(s, now);
    return;
  }
  
  if (executeNagCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.nagSuppress ? "Nag suppress ON - saved" : "Nag suppress OFF - saved");
    sendStatus(s, now);
    return;
  }
  
  if (executeProfileCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.profileOverride ? "Profile pinned - saved" : "Profile set to auto");
    sendStatus(s, now);
    return;
  }
  
  if (executeOffsetCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.offsetOverride ? "Offset pinned - saved" : "Offset set to auto");
    sendStatus(s, now);
    return;
  }
  
  if (executeIsaChimeCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.isaChimeSuppress ? "ISA chime suppressed - saved" : "ISA chime original - saved");
    sendStatus(s, now);
    return;
  }
  
  if (executeSummonCmd(cmd, s)) {
    sendAck(cmd);
    if (s.summonRemaining > 0)
      sendLog("Summon burst started (30 frames)");
    else
      sendLog("Summon stopped");
    sendStatus(s, now);
    return;
  }
  
  if (executeVariantCmd(cmd, s)) {
    sendAck(cmd);
    sendLog("Variant changed - filters updated");
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
    sendError("Waiting for 0x273 frame");
    return;
  }
  
  if (executeVehicleCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(cmd);
    sendStatus(s, now);
  } else {
    sendError("Unknown command");
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
  sendLog(BOARD_READY_MSG);
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
