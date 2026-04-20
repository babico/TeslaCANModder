#pragma once
#include <SPI.h>
#include <mcp2515.h>
#include "core/config/uno.h"
#include "core/types.h"
#include "infra/can.h"

// ── MCP2515 Array-Driven Driver (Arduino Uno, up to 3 buses) ────────────────
// Bus 0 = MCP2515_1, Bus 1 = MCP2515_2, Bus 2 = MCP2515_3
// Note: Uno has only 2 hardware interrupts (INT0=D2, INT1=D3).
// MCP2515_3 (bus 2) uses polling (frameReady always true).

static MCP2515 mcp2515_1(PIN_MCP2515_1_CS);
#if BUS_VEHICLE_ACTIVE
static MCP2515 mcp2515_2(PIN_MCP2515_2_CS);
#endif
#if BUS_BODY_ACTIVE
static MCP2515 mcp2515_3(PIN_MCP2515_3_CS);
#endif

static MCP2515 *mcpBus[BUS_MAX] = {&mcp2515_1
#if BUS_VEHICLE_ACTIVE
								   ,
								   &mcp2515_2
#else
								   ,
								   nullptr
#endif
#if BUS_BODY_ACTIVE
								   ,
								   &mcp2515_3
#else
								   ,
								   nullptr
#endif
};

static volatile bool mcpFrameReady[BUS_MAX];
bool mcpAvailable[BUS_MAX];							 // extern-visible
static uint8_t mcpClockReqMHz = BOARD_CAN_CLOCK_MHZ; // 0=auto
static uint8_t mcpClockMHz = BOARD_CAN_CLOCK_MHZ;	 // active clock after fallback

static const uint8_t mcpIntPins[3] = {PIN_MCP2515_1_INT, PIN_MCP2515_2_INT, PIN_MCP2515_3_INT};

void mcpISR0() { mcpFrameReady[0] = true; }
#if BUS_VEHICLE_ACTIVE
void mcpISR1() { mcpFrameReady[1] = true; }
#endif

static void (*mcpISRs[2])() = {mcpISR0
#if BUS_VEHICLE_ACTIVE
							   ,
							   mcpISR1
#endif
};

// Shared bitrate, filter, clock, and send functions
#include "core/driver/common.h"

// ── Platform-specific init ──────────────────────────────────────────────────

static bool initMcpBus(MCP2515 &mcp, uint8_t intPin, void (*isr)(), bool useInterrupt)
{
	mcp.reset();
	bool ok = configureBitrateWithFallback(mcp);
	if (!ok)
		return false;
	mcp.setNormalMode();
	if (useInterrupt)
	{
		pinMode(intPin, INPUT_PULLUP);
		attachInterrupt(digitalPinToInterrupt(intPin), isr, FALLING);
	}
	return true;
}

bool driverInit()
{
	// Bus 0: MCP2515_1 (hardware interrupt)
	mcpFrameReady[0] = true;
	mcpAvailable[0] = initMcpBus(mcp2515_1, PIN_MCP2515_1_INT, mcpISR0, true);

#if BUS_VEHICLE_ACTIVE
	// Bus 1: MCP2515_2 (hardware interrupt)
	mcpFrameReady[1] = true;
	mcpAvailable[1] = initMcpBus(mcp2515_2, PIN_MCP2515_2_INT, mcpISR1, true);
#endif

#if BUS_BODY_ACTIVE
	// Bus 2: MCP2515_3 (polled — no hardware interrupt available on Uno)
	mcpFrameReady[2] = true;
	mcpAvailable[2] = initMcpBus(mcp2515_3, PIN_MCP2515_3_INT, nullptr, false);
#endif

	return mcpAvailable[0];
}

bool driverRead(Frame &f, uint8_t &bus)
{
	for (uint8_t i = 0; i < BUS_MAX; i++)
	{
		if (!mcpAvailable[i])
			continue;
#if BUS_BODY_ACTIVE
		// Bus 2 is polled (always check)
		if (i == 2)
			mcpFrameReady[2] = true;
#endif
		if (mcpFrameReady[i])
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
	mcpFrameReady[0] = true;
	mcpAvailable[0] = initMcpBus(mcp2515_1, PIN_MCP2515_1_INT, mcpISR0, true);
#if BUS_VEHICLE_ACTIVE
	mcpFrameReady[1] = true;
	mcpAvailable[1] = initMcpBus(mcp2515_2, PIN_MCP2515_2_INT, mcpISR1, true);
#endif
#if BUS_BODY_ACTIVE
	mcpFrameReady[2] = true;
	mcpAvailable[2] = initMcpBus(mcp2515_3, PIN_MCP2515_3_INT, nullptr, false);
#endif
	return mcpAvailable[0];
}
