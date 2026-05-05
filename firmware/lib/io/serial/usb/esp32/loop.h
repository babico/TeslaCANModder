#pragma once
#include "client/command/dispatch.h"

// ── Serial API ───────────────────────────────────────────────────────────────
void serialInit(State& s) {
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {}

#if BOARD_ENABLE_BLE
  bleInit();
#endif

  dasInit();

  sendLog(F(BOARD_READY_MSG));
  sendBoot(s);
}

void serialTick(State& s) {
  unsigned long now = millis();

  // Periodic status
  const unsigned long statusInterval = statusLiveEnabled ? STATUS_LIVE_INTERVAL_MS : STATUS_INTERVAL_MS;
  if (now - lastStatusMs >= statusInterval) {
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
  // Flush queued gamepad button press events
  gamepadFlushEvents(s, now);
  // Map gamepad analog axes to DAS drive control
  gamepadDriveTick(s, now);
#endif

  // DAS autopilot CAN injection — rate-limited frame sender (always runs when bus active)
  dasTick(now, s);
}
