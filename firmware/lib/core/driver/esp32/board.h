#pragma once
#include <SPI.h>
#include <mcp2515.h>
#include "core/config/esp32/board.h"
#include "core/types.h"
#include "infra/can.h"

// ── MCP2515 Array-Driven Driver (ESP32, up to 3 buses) ─────────────────────
// All CAN buses use MCP2515 modules over SPI.
// Bus 0 = MCP2515_1, Bus 1 = MCP2515_2, Bus 2 = MCP2515_3

static const uint8_t mcpCsPins[3] = {PIN_MCP2515_1_CS, PIN_MCP2515_2_CS, PIN_MCP2515_3_CS};
static const uint8_t mcpIntPins[3] = {PIN_MCP2515_1_INT, PIN_MCP2515_2_INT, PIN_MCP2515_3_INT};

static MCP2515 *mcpBus[BUS_MAX];
static volatile bool mcpFrameReady[BUS_MAX];
bool mcpAvailable[BUS_MAX];							 // extern-visible
static uint8_t mcpClockReqMHz = BOARD_CAN_CLOCK_MHZ; // 0=auto
static uint8_t mcpClockMHz = BOARD_CAN_CLOCK_MHZ;	 // active clock after fallback

// ISRs must be distinct function pointers
void IRAM_ATTR mcpISR0()
{
	mcpFrameReady[0] = true;
}
#if BUS_VEHICLE_ACTIVE
void IRAM_ATTR mcpISR1()
{
	mcpFrameReady[1] = true;
}
#endif
#if BUS_BODY_ACTIVE
void IRAM_ATTR mcpISR2()
{
	mcpFrameReady[2] = true;
}
#endif

static void (*mcpISRs[BUS_MAX])() = {mcpISR0
#if BUS_VEHICLE_ACTIVE
									 ,
									 mcpISR1
#else
									 ,
									 nullptr
#endif
#if BUS_BODY_ACTIVE
									 ,
									 mcpISR2
#else
									 ,
									 nullptr
#endif
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
