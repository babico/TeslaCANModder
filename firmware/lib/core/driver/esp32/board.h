#pragma once
#include <SPI.h>
#include <mcp2515.h>
#include "core/config/esp32/board.h"
#include "core/types.h"
#include "core/can/bus.h"

// ── MCP2515 Array-Driven Driver (ESP32, up to 3 buses) ─────────────────────
// All CAN buses use MCP2515 modules over SPI. Array slots are indexed by the
// BUS_* IDs from core/can/bus.h (BUS_CHASSIS=0, BUS_VEHICLE=1, BUS_BODY=2).
// The static_asserts below pin that contract.

static_assert(BUS_CHASSIS == 0, "BUS_CHASSIS must be slot 0");
static_assert(BUS_VEHICLE == 1, "BUS_VEHICLE must be slot 1");
static_assert(BUS_BODY == 2, "BUS_BODY must be slot 2");

static const uint8_t mcpCsPins[BUS_MAX] = {
	PIN_MCP2515_CHASSIS_CS,
	PIN_MCP2515_VEHICLE_CS,
	PIN_MCP2515_BODY_CS,
};
static const uint8_t mcpIntPins[BUS_MAX] = {
	PIN_MCP2515_CHASSIS_INT,
	PIN_MCP2515_VEHICLE_INT,
	PIN_MCP2515_BODY_INT,
};

static MCP2515 *mcpBus[BUS_MAX];
static volatile bool mcpFrameReady[BUS_MAX];
bool mcpAvailable[BUS_MAX];							 // extern-visible
static uint8_t mcpClockReqMHz = BOARD_CAN_CLOCK_MHZ; // 0=auto
static uint8_t mcpClockMHz = BOARD_CAN_CLOCK_MHZ;	 // active clock after fallback

// One ISR per bus slot — distinct function pointers required by attachInterrupt.
void IRAM_ATTR mcpISR_chassis() { mcpFrameReady[BUS_CHASSIS] = true; }
void IRAM_ATTR mcpISR_vehicle() { mcpFrameReady[BUS_VEHICLE] = true; }
void IRAM_ATTR mcpISR_body()    { mcpFrameReady[BUS_BODY]    = true; }

static void (*mcpISRs[BUS_MAX])() = {
	mcpISR_chassis,
	mcpISR_vehicle,
	mcpISR_body,
};

// Shared bitrate, filter, clock, and send functions
#include "core/driver/common.h"

// ── Platform-specific init ──────────────────────────────────────────────────

static bool initMcpBus(MCP2515 &mcp, uint8_t intPin, void (*isr)())
{
	mcp.reset();
	bool ok = configureBitrateWithFallback(mcp);
	if (!ok)
		return false;
	mcp.setNormalMode();
	pinMode(intPin, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(intPin), isr, FALLING);
	return true;
}

bool driverInit()
{
	SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI);
	bool anyOk = false;
	for (uint8_t i = 0; i < BUS_MAX; i++)
	{
		if (!busActive(i))
		{
			mcpAvailable[i] = false;
			continue;
		}
		mcpBus[i] = new MCP2515(mcpCsPins[i]);
		mcpFrameReady[i] = true;
		mcpAvailable[i] = initMcpBus(*mcpBus[i], mcpIntPins[i], mcpISRs[i]);
		if (mcpAvailable[i])
			anyOk = true;
	}
	return anyOk;
}

bool driverRead(Frame &f, uint8_t &bus)
{
	for (uint8_t i = 0; i < BUS_MAX; i++)
	{
		if (mcpAvailable[i] && mcpFrameReady[i])
		{
			can_frame raw;
			if (mcpBus[i]->readMessage(&raw) == MCP2515::ERROR_OK)
			{
				f.id = raw.can_id;
				f.dlc = raw.can_dlc;
				memcpy(f.data, raw.data, 8);
				bus = i;
				return true;
			}
			mcpFrameReady[i] = false;
		}
	}
	return false;
}

bool driverReinit()
{
	bool anyOk = false;
	for (uint8_t i = 0; i < BUS_MAX; i++)
	{
		if (!busActive(i))
			continue;
		mcpAvailable[i] = initMcpBus(*mcpBus[i], mcpIntPins[i], mcpISRs[i]);
		mcpFrameReady[i] = true;
		if (mcpAvailable[i])
			anyOk = true;
	}
	return anyOk;
}
