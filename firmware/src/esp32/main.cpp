/**
 * @file firmware/src/esp32/main.cpp
 * @brief ESP32 firmware entry point — setup() and loop() for Tesla CAN Mod
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <Arduino.h>
#include "core/config/esp32/board.h"
#include "core/types.h"
#include "core/platform.h"
#include "core/persist/esp32/board.h"
#include "core/driver/esp32/board.h"
#include "handler/dispatch.h"
#include "io/serial/usb/esp32/board.h"
#include "core/can/ring_buffer.h"
#include "core/can/health.h"

#if BOARD_ENABLE_WIFI
#include "io/wifi/esp32/board.h"
#endif

static State state;
static VehiclePlatform platform;
static bool driverReady = false;
static bool settingsLoaded = false;

/**
 * @brief Arduino setup — initializes peripherals, CAN buses, and optional transports
 */
void setup()
{
	pinMode(PIN_LED, OUTPUT);
	digitalWrite(PIN_LED, HIGH);

	// Restore persisted settings from ESP32 NVS flash
	settingsLoaded = loadSettings(state);

	// Bring up USB serial and optional BLE serial bridge
	serialInit(state);

	// Attempt MCP2515 initialization on all configured SPI buses
	driverReady = driverInit();
	if (!driverReady)
	{
		sendLog(F("ERROR: CAN init failed. Check wiring."));
	}

	state.canClockReqMHz = driverGetClockReqMHz();
	state.canClockMHz = driverGetClockMHz();

	// Program MCP2515 acceptance filters for the active Tesla variant
	if (driverReady)
	{
		applyFilters(state);
	}

	sendLog(settingsLoaded ? F("Settings loaded from NVS") : F("NVS empty - using defaults"));

	// Re-enable single-shot TX mode if it was saved in NVS
	if (state.singleShotTx)
	{
		driverSetSingleShot(true);
		sendLog(F("Single-shot TX restored from NVS"));
	}

	extern bool mcpAvailable[];
	CanHealthReport health = checkCanHealth(mcpAvailable, state.chassisOnline);
	for (uint8_t i = 0; i < BUS_MAX; i++)
	{
		if (!busActive(i))
			continue;
		char msg[48];
		if (health.bus[i].detected)
			snprintf(msg, sizeof(msg), "MCP2515_%d ready (Bus %d: %s)", i + 1, i, busIndexName(i));
		else
			snprintf(msg, sizeof(msg), "MCP2515_%d NOT DETECTED (Bus %d: %s)", i + 1, i, busIndexName(i));
		sendLog(msg);
	}

	if (!health.allDetected)
	{
		char warn[60];
		snprintf(warn, sizeof(warn), "WARNING: %d/%d CAN buses missing MCP2515",
				 health.configuredCount - health.detectedCount, health.configuredCount);
		sendLog(warn);
	}

	char busMsg[48];
	snprintf(busMsg, sizeof(busMsg), "Buses: Chassis=%d Veh=%d Body=%d", BUS_CHASSIS_ACTIVE, BUS_VEHICLE_ACTIVE,
			 BUS_BODY_ACTIVE);
	sendLog(busMsg);

	// Start WiFi AP and HTTP server if compiled in
#if BOARD_ENABLE_WIFI
	wifiInit(state);
#endif

#if BOARD_ENABLE_BLE
	sendLog(F("BLE active"));
#endif

	// Determine Tesla model/generation/software from persisted state fields
	platform.resolveFromState(state);
	syncPlatformToState(platform, state);
	if (platform.resolved)
	{
		char pMsg[80];
		snprintf(pMsg, sizeof(pMsg), "Platform: %s / %s / %u.%u.%u", teslaModelName(platform.model),
				 hwGenerationName(platform.hwGen), platform.software.year, platform.software.week,
				 platform.software.release);
		sendLog(pMsg);
	}
}

/**
 * @brief Arduino loop — processes serial commands, WiFi, and CAN frame dispatch
 */
void loop()
{
	// Drain incoming serial/BLE command queue
	serialTick(state);

	// Service pending WiFi HTTP requests
#if BOARD_ENABLE_WIFI
	wifiTick();
#endif

	if (!driverReady)
	{
		delay(10);
		return;
	}

	unsigned long now = millis();
	state.apGateSummoning = state.summonRemaining > 0;

	// Detect CAN bus silence and transition to standby
	if (state.chassisOnline && state.lastFrameMs > 0 && (now - state.lastFrameMs) >= CAN_TIMEOUT_MS)
	{
		state.chassisOnline = false;
		state.standby = true;
		state.hasCtrl = false;
		state.hasClimate = false;
		state.hasCharge = false;
		state.hasDrive = false;
		state.summonRemaining = 0;
		state.apGateSummoning = false;
		state.apGateParked = true;
		sendLog(F("CAN bus silent - entering standby"));
	}

	// Standby: blink LED, periodically reinit MCP2515, wait for first frame
	if (state.standby)
	{
		digitalWrite(PIN_LED, (now / (LED_STANDBY_INTERVAL / 2)) % 2 ? HIGH : LOW);

		if (now - state.lastReinitMs >= CAN_REINIT_INTERVAL)
		{
			state.lastReinitMs = now;
			driverReinit();
			applyFilters(state);
			state.canClockReqMHz = driverGetClockReqMHz();
			state.canClockMHz = driverGetClockMHz();
		}

		Frame frame;
		uint8_t bus;
		if (driverRead(frame, bus))
		{
			state.standby = false;
			state.chassisOnline = true;
			state.lastFrameMs = now;
			sendLog(F("CAN bus active - resuming operation"));
			sendFrame(frame, "rx", bus, now, state);
			handleMessage(frame, bus, state);
		}
		return;
	}

	// Normal operation: run periodic feature ticks
	summonTick(state);
	preconditionTick(state);
	burstTick(state);
	driveModeTick_dispatch(state);
	seatbeltEmulationTick(state);
	canSimTick(state);

	// Poll MCP2515 error registers every 250 ms; auto-recover from bus-off
	static unsigned long _lastErrPollMs = 0;
	if (now - _lastErrPollMs >= 250)
	{
		_lastErrPollMs = now;
		driverPollBusErrors();
		state.canDiag.txFailCount += driverGetAndResetTxFails();
		state.canDiag.busOffCount += driverGetAndResetBusOffEvents();
	}

	Frame frame;
	uint8_t bus;
	while (driverRead(frame, bus))
	{
		state.lastFrameMs = now;
		if (!state.chassisOnline)
		{
			state.chassisOnline = true;
			sendLog(F("CAN bus online"));
		}
		digitalWrite(PIN_LED, LOW);
		ringPush(frame, bus, now);
		sendFrame(frame, "rx", bus, now, state);
		handleMessage(frame, bus, state);
	}

	digitalWrite(PIN_LED, HIGH);
}
