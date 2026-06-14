#pragma once

/**
 * @file firmware/lib/core/driver/esp32/board.h
 * @brief ESP32 MCP2515 array-driven CAN driver — init, read, reinit, and ISR wiring
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <SPI.h>
#include <mcp2515.h>
#include "core/config/esp32/board.h"
#include "core/types.h"
#include "core/can/bus.h"

// Array slots are indexed by BUS_* IDs from core/can/bus.h.
// Static asserts pin the expected slot assignments at compile time.
static_assert(BUS_CHASSIS == 0, "BUS_CHASSIS must be slot 0");
static_assert(BUS_VEHICLE == 1, "BUS_VEHICLE must be slot 1");
static_assert(BUS_BODY == 2, "BUS_BODY must be slot 2");

/** @brief Chip-select GPIO for each MCP2515 module, indexed by bus slot */
inline constexpr uint8_t mcpCsPins[BUS_MAX] = {
	PIN_MCP2515_CHASSIS_CS,
	PIN_MCP2515_VEHICLE_CS,
	PIN_MCP2515_BODY_CS,
};

/** @brief Interrupt GPIO for each MCP2515 module, indexed by bus slot */
inline constexpr uint8_t mcpIntPins[BUS_MAX] = {
	PIN_MCP2515_CHASSIS_INT,
	PIN_MCP2515_VEHICLE_INT,
	PIN_MCP2515_BODY_INT,
};

static inline MCP2515 *mcpBus[BUS_MAX];      // MCP2515 instance pointers per bus
static inline volatile bool mcpFrameReady[BUS_MAX]; // ISR-set flag: frame pending in RX buffer
inline bool mcpAvailable[BUS_MAX];           // extern-visible: bus initialized successfully
static uint8_t mcpClockReqMHz = BOARD_CAN_CLOCK_MHZ; // Requested oscillator (0 = auto)
static uint8_t mcpClockMHz = BOARD_CAN_CLOCK_MHZ;    // Active clock after fallback probing

// One ISR per bus slot — attachInterrupt requires distinct function pointers.
inline void IRAM_ATTR mcpISR_chassis() { mcpFrameReady[BUS_CHASSIS] = true; }
inline void IRAM_ATTR mcpISR_vehicle() { mcpFrameReady[BUS_VEHICLE] = true; }
inline void IRAM_ATTR mcpISR_body()    { mcpFrameReady[BUS_BODY]    = true; }

/** @brief ISR function pointer table indexed by bus slot */
static inline void (*mcpISRs[BUS_MAX])() = {
	mcpISR_chassis,
	mcpISR_vehicle,
	mcpISR_body,
};

// Shared bitrate, filter, clock, and send functions
#include "core/driver/common.h"

// ── Platform-specific initialization ────────────────────────────────────────

/**
 * @brief Initialize a single MCP2515 bus: reset, configure bitrate, attach interrupt
 * @param mcp Reference to the MCP2515 instance
 * @param intPin GPIO pin connected to the MCP2515 INT output
 * @param isr ISR function to attach for frame-ready notification
 * @return true if bitrate configuration succeeded
 */
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

/**
 * @brief Initialize SPI and all active MCP2515 buses
 * @return true if at least one bus initialized successfully
 */
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
		mcpFrameReady[i] = true; // Prime flag so first poll reads any buffered frame
		mcpAvailable[i] = initMcpBus(*mcpBus[i], mcpIntPins[i], mcpISRs[i]);
		if (mcpAvailable[i])
			anyOk = true;
	}
	return anyOk;
}

/**
 * @brief Read one pending CAN frame from any active bus (round-robin poll)
 * @param f Output frame populated on success
 * @param bus Output bus index the frame was received on
 * @return true if a frame was read
 */
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
			mcpFrameReady[i] = false; // No more frames buffered on this bus
		}
	}
	return false;
}

/**
 * @brief Re-initialize all active buses without reallocating MCP2515 instances
 * @return true if at least one bus re-initialized successfully
 */
bool driverReinit()
{
	bool anyOk = false;
	for (uint8_t i = 0; i < BUS_MAX; i++)
	{
		if (!busActive(i))
			continue;
		mcpAvailable[i] = initMcpBus(*mcpBus[i], mcpIntPins[i], mcpISRs[i]);
		mcpFrameReady[i] = true; // Prime flag for immediate poll
		if (mcpAvailable[i])
			anyOk = true;
	}
	return anyOk;
}
