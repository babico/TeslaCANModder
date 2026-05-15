#pragma once

/**
 * @file firmware/lib/transport/can/driver.h
 * @brief Shared MCP2515 driver functions for bitrate, filters, TX, and error recovery
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

// Include AFTER platform-specific state is defined:
//   mcpBus[], mcpFrameReady[], mcpAvailable[], mcpClockReqMHz, mcpClockMHz
// Requires: <SPI.h>, <mcp2515.h>, "core/types.h", "transport/can/bus.h"

/**
 * @brief Set the MCP2515 bitrate to 500 kbps for a given oscillator frequency
 * @param mcp Reference to the MCP2515 instance
 * @param mhz Oscillator frequency in MHz (8, 16, or 20)
 * @return true if bitrate was set successfully
 */
static bool setBitrateForClock(MCP2515 &mcp, uint8_t mhz)
{
	CAN_CLOCK clk;
	if (mhz == 8)
		clk = MCP_8MHZ;
	else if (mhz == 16)
		clk = MCP_16MHZ;
	else if (mhz == 20)
		clk = MCP_20MHZ;
	else
		return false;
	return mcp.setBitrate(CAN_500KBPS, clk) == MCP2515::ERROR_OK;
}

/**
 * @brief Attempt bitrate configuration using the requested clock, falling back to alternatives
 * @param mcp Reference to the MCP2515 instance
 * @return true if any clock frequency succeeded
 */
static bool configureBitrateWithFallback(MCP2515 &mcp)
{
	const uint8_t tryAuto[] = {8, 16, 20};
	const uint8_t try8[] = {8, 16, 20};
	const uint8_t try16[] = {16, 8, 20};
	const uint8_t try20[] = {20, 16, 8};

	// 12 MHz is accepted as a compatibility alias — no distinct MCP2515 clock
	// setting exists for it, so the default auto probe order is used instead.
	const uint8_t *seq = tryAuto;
	if (mcpClockReqMHz == 8)
		seq = try8;
	else if (mcpClockReqMHz == 16)
		seq = try16;
	else if (mcpClockReqMHz == 20)
		seq = try20;

	for (uint8_t i = 0; i < 3; i++)
	{
		if (setBitrateForClock(mcp, seq[i]))
		{
			mcpClockMHz = seq[i];
			return true;
		}
	}
	return false;
}

/**
 * @brief Program hardware RX filters on an MCP2515 instance
 * @param mcp Reference to the MCP2515 instance
 * @param ids Array of 11-bit CAN IDs to accept (up to 6)
 * @param count Number of IDs in the array (0 = accept all)
 */
static void applyMcpFilters(MCP2515 &mcp, const uint32_t *ids, uint8_t count)
{
	mcp.setConfigMode();
	if (count == 0 || ids == nullptr)
	{
		// Accept all frames — mask bits zeroed so any ID passes
		mcp.setFilterMask(MCP2515::MASK0, false, 0x000);
		mcp.setFilter(MCP2515::RXF0, false, 0x000);
		mcp.setFilter(MCP2515::RXF1, false, 0x000);
		mcp.setFilterMask(MCP2515::MASK1, false, 0x000);
		mcp.setFilter(MCP2515::RXF2, false, 0x000);
		mcp.setFilter(MCP2515::RXF3, false, 0x000);
		mcp.setFilter(MCP2515::RXF4, false, 0x000);
		mcp.setFilter(MCP2515::RXF5, false, 0x000);
	}
	else
	{
		// Exact-match filtering — full 11-bit mask, unused slots duplicate ids[0]
		mcp.setFilterMask(MCP2515::MASK0, false, 0x7FF);
		mcp.setFilter(MCP2515::RXF0, false, ids[0]);
		mcp.setFilter(MCP2515::RXF1, false, count > 1 ? ids[1] : ids[0]);
		mcp.setFilterMask(MCP2515::MASK1, false, 0x7FF);
		mcp.setFilter(MCP2515::RXF2, false, count > 2 ? ids[2] : ids[0]);
		mcp.setFilter(MCP2515::RXF3, false, count > 3 ? ids[3] : ids[0]);
		mcp.setFilter(MCP2515::RXF4, false, count > 4 ? ids[4] : ids[0]);
		mcp.setFilter(MCP2515::RXF5, false, count > 5 ? ids[5] : ids[0]);
	}
	mcp.setNormalMode();
}

// ── Public API ──────────────────────────────────────────────────────────────

/**
 * @brief Apply hardware RX filters to a specific bus
 * @param bus Bus index (BUS_CHASSIS, BUS_VEHICLE, or BUS_BODY)
 * @param ids Array of 11-bit CAN IDs to accept
 * @param count Number of IDs (0 = accept all)
 */
void driverSetBusFilters(uint8_t bus, const uint32_t *ids, uint8_t count)
{
	if (bus < BUS_MAX && busActive(bus) && mcpAvailable[bus])
		applyMcpFilters(*mcpBus[bus], ids, count);
}

/**
 * @brief Apply hardware RX filters to the chassis bus only (legacy convenience)
 * @param ids Array of 11-bit CAN IDs to accept
 * @param count Number of IDs (0 = accept all)
 */
void driverSetFilters(const uint32_t *ids, uint8_t count)
{
	driverSetBusFilters(BUS_CHASSIS, ids, count);
}

/**
 * @brief Check whether a bus is initialized and ready for communication
 * @param bus Bus index to check
 * @return true if the bus is active and its MCP2515 responded during init
 */
bool driverBusReady(uint8_t bus)
{
	return bus < BUS_MAX && busActive(bus) && mcpAvailable[bus];
}

/**
 * @brief Set the requested MCP2515 oscillator clock for next init/reinit
 * @param mhz Desired clock in MHz (8, 16, or 20; 0 = auto-detect)
 */
void driverSetClockMHz(uint8_t mhz)
{
	mcpClockReqMHz = mhz;
}

/**
 * @brief Get the currently requested oscillator clock
 * @return Requested clock in MHz (0 = auto)
 */
uint8_t driverGetClockReqMHz()
{
	return mcpClockReqMHz;
}

/**
 * @brief Get the active oscillator clock determined after fallback probing
 * @return Active clock in MHz
 */
uint8_t driverGetClockMHz()
{
	return mcpClockMHz;
}

static uint32_t _driverTxFailCount = 0;  // Cumulative TX send failures since last reset
static uint32_t _driverBusOffCount = 0;  // Cumulative bus-off events since last reset

/**
 * @brief Read and reset the TX failure counter (atomic swap pattern)
 * @return Number of TX failures since the last call
 */
uint32_t driverGetAndResetTxFails()
{
	uint32_t v = _driverTxFailCount;
	_driverTxFailCount = 0;
	return v;
}

/**
 * @brief Read and reset the bus-off event counter (atomic swap pattern)
 * @return Number of bus-off events since the last call
 */
uint32_t driverGetAndResetBusOffEvents()
{
	uint32_t v = _driverBusOffCount;
	_driverBusOffCount = 0;
	return v;
}

/**
 * @brief Poll MCP2515 EFLG register on each active bus and auto-recover from bus-off
 */
void driverPollBusErrors()
{
	for (uint8_t i = 0; i < BUS_MAX; i++)
	{
		if (!busActive(i) || !mcpAvailable[i])
			continue;
		uint8_t eflg = mcpBus[i]->getErrorFlags();
		if (eflg & MCP2515::EFLG_TXBO)
		{
			// Bus-off detected: reset controller and re-establish bitrate
			_driverBusOffCount++;
			mcpBus[i]->reset();
			configureBitrateWithFallback(*mcpBus[i]);
			mcpBus[i]->setNormalMode();
		}
	}
}

/**
 * @brief Transmit a CAN frame on the specified bus
 * @param f Frame to send (id, dlc, data)
 * @param bus Target bus index
 */
void driverSend(const Frame &f, uint8_t bus)
{
	if (bus < BUS_MAX && mcpAvailable[bus])
	{
		can_frame raw;
		raw.can_id = f.id;
		raw.can_dlc = f.dlc;
		memcpy(raw.data, f.data, 8);
		if (mcpBus[bus]->sendMessage(&raw) != MCP2515::ERROR_OK)
			_driverTxFailCount++;
	}
}

/**
 * @brief Enable or disable one-shot TX mode on all available buses
 * @param enabled true to enable one-shot (no automatic retransmit), false for normal mode
 */
void driverSetSingleShot(bool enabled)
{
	for (uint8_t i = 0; i < BUS_MAX; i++)
	{
		if (mcpAvailable[i] && mcpBus[i])
		{
			if (enabled)
			{
				mcpBus[i]->setNormalOneShotMode();
			}
			else
			{
				mcpBus[i]->setNormalMode();
			}
		}
	}
}
