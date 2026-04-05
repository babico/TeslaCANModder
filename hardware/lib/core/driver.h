#pragma once
#include <SPI.h>
#include <mcp2515.h>
#include "config.h"
#include "types.h"

static MCP2515 mcp1(PIN_MCP2515_CS);
#if BOARD_ENABLE_MCP2515_2
static MCP2515 mcp2(PIN_MCP2515_2_CS);
#endif

static volatile bool frameReady1 = true;
#if BOARD_ENABLE_MCP2515_2
static volatile bool frameReady2 = true;
#endif

void canISR1() { frameReady1 = true; }
#if BOARD_ENABLE_MCP2515_2
void canISR2() { frameReady2 = true; }
#endif

static bool initBus(MCP2515& mcp, uint8_t intPin, void (*isr)()) {
  mcp.reset();
  // Try 8MHz crystal first, then 16MHz — covers common MCP2515 modules
  bool ok = mcp.setBitrate(CAN_500KBPS, MCP_8MHZ)  == MCP2515::ERROR_OK
         || mcp.setBitrate(CAN_500KBPS, MCP_16MHZ) == MCP2515::ERROR_OK;
  if (!ok) return false;  // Chip absent or wiring fault — times out safely (10ms)
  mcp.setNormalMode();
  pinMode(intPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(intPin), isr, FALLING);
  return true;
}

#if BOARD_ENABLE_MCP2515_2
static bool bus2Available = false;
#endif

bool driverInit() {
  bool ok1 = initBus(mcp1, PIN_MCP2515_INT, canISR1);
#if BOARD_ENABLE_MCP2515_2
  bus2Available = initBus(mcp2, PIN_MCP2515_2_INT, canISR2);
#endif
  return ok1;
}

static void applyFiltersTo(MCP2515& mcp, const uint32_t* ids, uint8_t count) {
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

void driverSetFilters(const uint32_t* ids, uint8_t count) {
  applyFiltersTo(mcp1, ids, count);
}

#if BOARD_ENABLE_MCP2515_2
void driverSetFilters2(const uint32_t* ids, uint8_t count) {
  if (!bus2Available) return;
  applyFiltersTo(mcp2, ids, count);
}

bool driverBus2Ready() { return bus2Available; }
#endif

bool driverRead(Frame& f, uint8_t& bus) {
  can_frame raw;
#if BOARD_ENABLE_MCP2515_2
  static bool lastReadBus1 = true;
  if (bus2Available) {
    bool tryBus1First = lastReadBus1;
    for (uint8_t attempt = 0; attempt < 2; attempt++) {
      if (tryBus1First) {
        if (frameReady1 && mcp1.readMessage(&raw) == MCP2515::ERROR_OK) {
          f.id = raw.can_id; f.dlc = raw.can_dlc;
          memcpy(f.data, raw.data, 8);
          bus = 0;
          lastReadBus1 = false;
          return true;
        }
        frameReady1 = false;
      } else {
        if (frameReady2 && mcp2.readMessage(&raw) == MCP2515::ERROR_OK) {
          f.id = raw.can_id; f.dlc = raw.can_dlc;
          memcpy(f.data, raw.data, 8);
          bus = 1;
          lastReadBus1 = true;
          return true;
        }
        frameReady2 = false;
      }
      tryBus1First = !tryBus1First;
    }
    return false;
  }
#endif
  if (frameReady1) {
    if (mcp1.readMessage(&raw) == MCP2515::ERROR_OK) {
      f.id = raw.can_id; f.dlc = raw.can_dlc;
      memcpy(f.data, raw.data, 8);
      bus = 0;
      return true;
    }
    frameReady1 = false;
  }
  return false;
}

// Re-initialize MCP2515 after standby / error recovery
bool driverReinit() {
  bool ok1 = initBus(mcp1, PIN_MCP2515_INT, canISR1);
#if BOARD_ENABLE_MCP2515_2
  bus2Available = initBus(mcp2, PIN_MCP2515_2_INT, canISR2);
#endif
  frameReady1 = true;
#if BOARD_ENABLE_MCP2515_2
  frameReady2 = true;
#endif
  return ok1;
}

void driverSend(const Frame& f, uint8_t bus = 0) {
  can_frame raw;
  raw.can_id = f.id;
  raw.can_dlc = f.dlc;
  memcpy(raw.data, f.data, 8);
#if BOARD_ENABLE_MCP2515_2
  if (bus == 1 && bus2Available) mcp2.sendMessage(&raw);
  else                            mcp1.sendMessage(&raw);
#else
  mcp1.sendMessage(&raw);
#endif
}
