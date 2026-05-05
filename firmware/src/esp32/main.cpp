/*
 * TeslaCANModder - ESP32-S DevKit Firmware
 * Main entry point - setup() and loop()
 *
 * Supports up to 3 CAN buses (3x MCP2515 via SPI),
 * optional WiFi REST API, and optional BLE (Bluetooth Low Energy).
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
#include "io/wifi/client/api/board.h"
#endif

static State state;
static VehiclePlatform platform;
static bool driverReady = false;
static bool settingsLoaded = false;

void setup()
{
	pinMode(PIN_LED, OUTPUT);
	digitalWrite(PIN_LED, HIGH);

	// Load saved settings from NVS
	settingsLoaded = loadSettings(state);

	// Initialize serial bridge (USB + optional BLE)
	serialInit(state);

	// Initialize CAN driver(s)
	driverReady = driverInit();
	if (!driverReady)
	{
		sendLog(F("ERROR: CAN init failed. Check wiring."));
	}

	state.canClockReqMHz = driverGetClockReqMHz();
	state.canClockMHz = driverGetClockMHz();

	// Apply CAN filters for current variant
	if (driverReady)
	{
		applyFilters(state);
	}

	sendLog(settingsLoaded ? F("Settings loaded from NVS") : F("NVS empty - using defaults"));

	// Restore single-shot TX mode from saved settings
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

	// Initialize WiFi REST API
#if BOARD_ENABLE_WIFI
	wifiInit(state);
#endif

#if BOARD_ENABLE_BLE
	sendLog(F("BLE active"));
#endif

	// Resolve initial platform identity from loaded settings
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

void loop()
{
	// Process incoming commands from USB/Bluetooth
	serialTick(state);

	// Handle WiFi REST API requests
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

	// ── CAN Timeout Detection ───────────────────────────────────────────────
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

	// ── Standby Mode ───────────────────────────────────────────────────────
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

	// ── Normal Operation ───────────────────────────────────────────────────
	summonTick(state);
	preconditionTick(state);
	burstTick(state);
	driveModeTick_dispatch(state);
	seatbeltEmulationTick(state);
	canSimTick(state);

	// ── CAN Diagnostic Counter Poll ────────────────────────────────────────
	// Poll MCP2515 error registers and accumulate TX/bus-off counts to State.
	// driverPollBusErrors() also auto-recovers from bus-off by resetting the chip.
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
