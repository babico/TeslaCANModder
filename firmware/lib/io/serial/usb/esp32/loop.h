#pragma once

/**
 * @file firmware/lib/io/serial/usb/esp32/loop.h
 * @brief Serial transport initialization and main tick loop for USB and BLE input
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "client/command/dispatch.h"

/**
 * @brief Initialize the serial transport, BLE (if enabled), and DAS subsystem
 * @param s Transport state to populate with initial boot message
 */
void serialInit(State& s) {
	Serial.begin(115200);
	while (!Serial && millis() < 2000) {} // Wait up to 2s for USB serial connection

#if BOARD_ENABLE_BLE
	bleInit();
#endif

	dasInit();

	sendLog(F(BOARD_READY_MSG));
	sendBoot(s);
}

/**
 * @brief Main serial tick — sends periodic status and processes incoming commands
 * @param s Transport state containing buffers, timing, and stream flags
 */
void serialTick(State& s) {
	unsigned long now = millis();

	// Periodic status broadcast at normal or live interval
	const unsigned long statusInterval = statusLiveEnabled ? STATUS_LIVE_INTERVAL_MS : STATUS_INTERVAL_MS;
	if (now - lastStatusMs >= statusInterval) {
		lastStatusMs = now;
		sendStatus(s, now);
	}

	// Process incoming USB serial characters
	while (Serial.available()) {
		char c = Serial.read();
		handleChar(usbBuf, usbLen, c, s);
	}

#if BOARD_ENABLE_BLE
	// Update BLE key distance estimate from peer RSSI
	bleDistanceTick(s);
	// Process incoming BLE serial characters
	while (bleAvailable()) {
		char c = bleRead();
		handleChar(bleBuf, bleLen, c, s);
	}
	// Flush queued gamepad button press events
	gamepadFlushEvents(s, now);
	// Map gamepad analog axes to DAS drive control
	gamepadDriveTick(s, now);
#endif

	// DAS autopilot CAN injection — rate-limited frame sender
	dasTick(now, s);
}
