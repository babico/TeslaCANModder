#pragma once
#include <SPI.h>
#include <mcp2515.h>
#include "core/config/esp32.h"
#include "core/types.h"

// ── MCP2515 Array-Driven Driver (ESP32, up to 3 buses) ─────────────────────
// All CAN buses use MCP2515 modules over SPI.
// Bus 0 = MCP2515_1, Bus 1 = MCP2515_2, Bus 2 = MCP2515_3

static const uint8_t mcpCsPins[3]  = { PIN_MCP2515_1_CS, PIN_MCP2515_2_CS, PIN_MCP2515_3_CS };
static const uint8_t mcpIntPins[3] = { PIN_MCP2515_1_INT, PIN_MCP2515_2_INT, PIN_MCP2515_3_INT };

static MCP2515*       mcpBus[BUS_MAX];
static volatile bool  mcpFrameReady[BUS_MAX];
bool                  mcpAvailable[BUS_MAX];  // extern-visible

// ISRs must be distinct function pointers
void IRAM_ATTR mcpISR0() { mcpFrameReady[0] = true; }
#if BUS_VEHICLE_ACTIVE
void IRAM_ATTR mcpISR1() { mcpFrameReady[1] = true; }
#endif
#if BUS_BODY_ACTIVE
void IRAM_ATTR mcpISR2() { mcpFrameReady[2] = true; }
#endif

static void (*mcpISRs[BUS_MAX])() = { mcpISR0
#if BUS_VEHICLE_ACTIVE
  , mcpISR1
#else
  , nullptr
#endif
#if BUS_BODY_ACTIVE
  , mcpISR2
#else
  , nullptr
#endif
};

// ── Init / Filter helpers ───────────────────────────────────────────────────

static bool initMcpBus(MCP2515& mcp, uint8_t intPin, void (*isr)()) {
  mcp.reset();
  bool ok = mcp.setBitrate(CAN_500KBPS, MCP_8MHZ) == MCP2515::ERROR_OK
         || mcp.setBitrate(CAN_500KBPS, MCP_16MHZ) == MCP2515::ERROR_OK;
  if (!ok) return false;
  mcp.setNormalMode();
  pinMode(intPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(intPin), isr, FALLING);
  return true;
}

static void applyMcpFilters(MCP2515& mcp, const uint32_t* ids, uint8_t count) {
  mcp.setConfigMode();
  if (count == 0 || ids == nullptr) {
    mcp.setFilterMask(MCP2515::MASK0, false, 0x000);
    mcp.setFilter(MCP2515::RXF0, false, 0x000);
    mcp.setFilter(MCP2515::RXF1, false, 0x000);
    mcp.setFilterMask(MCP2515::MASK1, false, 0x000);
    mcp.setFilter(MCP2515::RXF2, false, 0x000);
    mcp.setFilter(MCP2515::RXF3, false, 0x000);
    mcp.setFilter(MCP2515::RXF4, false, 0x000);
    mcp.setFilter(MCP2515::RXF5, false, 0x000);
  } else {
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

void driverSetFilters(const uint32_t* ids, uint8_t count) {
  if (mcpAvailable[0]) applyMcpFilters(*mcpBus[0], ids, count);
}

void driverSetBusFilters(uint8_t bus, const uint32_t* ids, uint8_t count) {
  if (bus < BUS_MAX && busActive(bus) && mcpAvailable[bus])
    applyMcpFilters(*mcpBus[bus], ids, count);
}

bool driverBusReady(uint8_t bus) {
  return bus < BUS_MAX && busActive(bus) && mcpAvailable[bus];
}

bool driverInit() {
  SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI);
  bool anyOk = false;
  for (uint8_t i = 0; i < BUS_MAX; i++) {
    if (!busActive(i)) { mcpAvailable[i] = false; continue; }
    mcpBus[i] = new MCP2515(mcpCsPins[i]);
    mcpFrameReady[i] = true;
    mcpAvailable[i] = initMcpBus(*mcpBus[i], mcpIntPins[i], mcpISRs[i]);
    if (mcpAvailable[i]) anyOk = true;
  }
  return anyOk;
}

bool driverRead(Frame& f, uint8_t& bus) {
  for (uint8_t i = 0; i < BUS_MAX; i++) {
    if (mcpAvailable[i] && mcpFrameReady[i]) {
      can_frame raw;
      if (mcpBus[i]->readMessage(&raw) == MCP2515::ERROR_OK) {
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

bool driverReinit() {
  bool anyOk = false;
  for (uint8_t i = 0; i < BUS_MAX; i++) {
    if (!busActive(i)) continue;
    mcpAvailable[i] = initMcpBus(*mcpBus[i], mcpIntPins[i], mcpISRs[i]);
    mcpFrameReady[i] = true;
    if (mcpAvailable[i]) anyOk = true;
  }
  return anyOk;
}

void driverSend(const Frame& f, uint8_t bus = 0) {
  if (bus < BUS_MAX && mcpAvailable[bus]) {
    can_frame raw;
    raw.can_id = f.id;
    raw.can_dlc = f.dlc;
    memcpy(raw.data, f.data, 8);
    mcpBus[bus]->sendMessage(&raw);
  }
}
