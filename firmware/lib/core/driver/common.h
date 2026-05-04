#pragma once
// ── Shared MCP2515 driver functions ─────────────────────────────────────────
// Include AFTER platform-specific state is defined:
//   mcpBus[], mcpFrameReady[], mcpAvailable[], mcpClockReqMHz, mcpClockMHz
// Requires: <SPI.h>, <mcp2515.h>, "core/types.h", "infra/can.h"

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

static bool configureBitrateWithFallback(MCP2515 &mcp)
{
	const uint8_t tryAuto[] = {8, 16, 20};
	const uint8_t try8[] = {8, 16, 20};
	const uint8_t try16[] = {16, 8, 20};
	const uint8_t try20[] = {20, 16, 8};

	// 12 MHz is accepted as a compatibility request, but there is no distinct
	// 12 MHz MCP2515 clock setting. Intentionally use the default auto fallback
	// order so the driver probes the supported clocks in its normal sequence.
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

static void applyMcpFilters(MCP2515 &mcp, const uint32_t *ids, uint8_t count)
{
	mcp.setConfigMode();
	if (count == 0 || ids == nullptr)
	{
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

void driverSetFilters(const uint32_t *ids, uint8_t count)
{
	if (mcpAvailable[0])
		applyMcpFilters(*mcpBus[0], ids, count);
}

void driverSetBusFilters(uint8_t bus, const uint32_t *ids, uint8_t count)
{
	if (bus < BUS_MAX && busActive(bus) && mcpAvailable[bus])
		applyMcpFilters(*mcpBus[bus], ids, count);
}

bool driverBusReady(uint8_t bus)
{
	return bus < BUS_MAX && busActive(bus) && mcpAvailable[bus];
}

void driverSetClockMHz(uint8_t mhz)
{
	mcpClockReqMHz = mhz;
}

uint8_t driverGetClockReqMHz()
{
	return mcpClockReqMHz;
}

uint8_t driverGetClockMHz()
{
	return mcpClockMHz;
}

// Module-level TX/bus-off diagnostic counters (polled from main loop)
static uint32_t _driverTxFailCount = 0;
static uint32_t _driverBusOffCount = 0;

uint32_t driverGetAndResetTxFails()
{
	uint32_t v = _driverTxFailCount;
	_driverTxFailCount = 0;
	return v;
}

uint32_t driverGetAndResetBusOffEvents()
{
	uint32_t v = _driverBusOffCount;
	_driverBusOffCount = 0;
	return v;
}

// Poll MCP2515 EFLG register for bus-off state on each active bus.
// Increments internal counter and auto-recovers (resets to Normal mode).
void driverPollBusErrors()
{
	for (uint8_t i = 0; i < BUS_MAX; i++)
	{
		if (!busActive(i) || !mcpAvailable[i])
			continue;
		uint8_t eflg = mcpBus[i]->getErrorFlags();
		if (eflg & MCP2515::EFLG_TXBO)
		{
			// Bus-off: increment counter and recover by re-entering Normal mode
			_driverBusOffCount++;
			mcpBus[i]->reset();
			configureBitrateWithFallback(*mcpBus[i]);
			mcpBus[i]->setNormalMode();
		}
	}
}

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
