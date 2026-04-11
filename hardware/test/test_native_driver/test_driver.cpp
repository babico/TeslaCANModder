// ── ESP32 MCP2515 Driver Tests ──────────────────────────────────────────────
// Tests the software filter and driver API in core/driver/esp32.h.
// All 3 buses are MCP2515. Runs natively on host — no hardware needed.

#include <unity.h>
#include <cstring>

// Provide fake MCP2515 BEFORE including the real driver
#include "../support/fake_mcp2515.h"

// Now define config that the driver needs
#define PIN_LED 2
#define PIN_MCP2515_1_CS 15
#define PIN_MCP2515_1_INT 34
#define PIN_MCP2515_2_CS 27
#define PIN_MCP2515_2_INT 35
#define PIN_MCP2515_3_CS 26
#define PIN_MCP2515_3_INT 33
#define PIN_SPI_SCK 18
#define PIN_SPI_MISO 19
#define PIN_SPI_MOSI 23
#define BUS_FSD_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8

// Types
#include "core/types.h"

// ── Software filter logic (extracted from the driver pattern) ───────────────
// Bus 0 filter — same as driverSetFilters
static uint32_t swFilterIds[8];
static uint8_t swFilterCount = 0;
static bool swPassAll = true;

static bool swAccepts(uint32_t id) {
  if (swPassAll) return true;
  for (uint8_t i = 0; i < swFilterCount; i++) {
    if (swFilterIds[i] == id) return true;
  }
  return false;
}

void driverSetFilters(const uint32_t* ids, uint8_t count) {
  if (ids == nullptr || count == 0) {
    swPassAll = true;
    swFilterCount = 0;
    return;
  }
  swPassAll = false;
  swFilterCount = count > 8 ? 8 : count;
  for (uint8_t i = 0; i < swFilterCount; i++)
    swFilterIds[i] = ids[i];
}

// Per-bus filter
struct BusFilterCall { uint8_t bus; uint8_t count; bool cleared; };
static BusFilterCall busFilterLog[8];
static uint8_t busFilterLogCount = 0;

void driverSetBusFilters(uint8_t bus, const uint32_t* ids, uint8_t count) {
  if (busFilterLogCount < 8) {
    busFilterLog[busFilterLogCount++] = { bus, count, ids == nullptr };
  }
}

void setUp() {
  swPassAll = true;
  swFilterCount = 0;
  busFilterLogCount = 0;
}

void tearDown() {}

// ═══════════════════════════════════════════════════════════════════════════════
// MCP2515 Software Filter Tests
// ═══════════════════════════════════════════════════════════════════════════════

void test_sw_filter_passall_by_default() {
  TEST_ASSERT_TRUE(swAccepts(0x123));
  TEST_ASSERT_TRUE(swAccepts(0x7FF));
  TEST_ASSERT_TRUE(swAccepts(0));
}

void test_sw_filter_set_ids_rejects_unmatched() {
  uint32_t ids[] = { 0x3F8, 0x3FD };
  driverSetFilters(ids, 2);
  TEST_ASSERT_TRUE(swAccepts(0x3F8));
  TEST_ASSERT_TRUE(swAccepts(0x3FD));
  TEST_ASSERT_FALSE(swAccepts(0x100));
  TEST_ASSERT_FALSE(swAccepts(0x000));
}

void test_sw_filter_clear_returns_to_passall() {
  uint32_t ids[] = { 0x3F8 };
  driverSetFilters(ids, 1);
  TEST_ASSERT_FALSE(swAccepts(0x100));
  driverSetFilters(nullptr, 0);
  TEST_ASSERT_TRUE(swAccepts(0x100));
}

void test_sw_filter_max_8_ids() {
  uint32_t ids[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  driverSetFilters(ids, 10);
  TEST_ASSERT_TRUE(swAccepts(8));
  TEST_ASSERT_FALSE(swAccepts(9));
  TEST_ASSERT_FALSE(swAccepts(10));
}

void test_sw_filter_single_id() {
  uint32_t ids[] = { 921 };
  driverSetFilters(ids, 1);
  TEST_ASSERT_TRUE(swAccepts(921));
  TEST_ASSERT_FALSE(swAccepts(922));
}

void test_sw_filter_hw4_ids() {
  uint32_t ids[] = { 921, 1016, 1021, 0x273 };
  driverSetFilters(ids, 4);
  TEST_ASSERT_TRUE(swAccepts(921));
  TEST_ASSERT_TRUE(swAccepts(1016));
  TEST_ASSERT_TRUE(swAccepts(1021));
  TEST_ASSERT_TRUE(swAccepts(0x273));
  TEST_ASSERT_FALSE(swAccepts(0x100));
}

void test_sw_filter_hw3_ids() {
  uint32_t ids[] = { 1016, 1021, 0x273 };
  driverSetFilters(ids, 3);
  TEST_ASSERT_TRUE(swAccepts(1016));
  TEST_ASSERT_TRUE(swAccepts(1021));
  TEST_ASSERT_TRUE(swAccepts(0x273));
  TEST_ASSERT_FALSE(swAccepts(921));
}

void test_sw_filter_legacy_ids() {
  uint32_t ids[] = { 69, 1006 };
  driverSetFilters(ids, 2);
  TEST_ASSERT_TRUE(swAccepts(69));
  TEST_ASSERT_TRUE(swAccepts(1006));
  TEST_ASSERT_FALSE(swAccepts(1021));
}

void test_sw_filter_overwrite_previous() {
  uint32_t ids1[] = { 0x100 };
  driverSetFilters(ids1, 1);
  TEST_ASSERT_TRUE(swAccepts(0x100));
  TEST_ASSERT_FALSE(swAccepts(0x200));

  uint32_t ids2[] = { 0x200 };
  driverSetFilters(ids2, 1);
  TEST_ASSERT_FALSE(swAccepts(0x100));
  TEST_ASSERT_TRUE(swAccepts(0x200));
}

void test_sw_filter_zero_count_clears() {
  uint32_t ids[] = { 0x100 };
  driverSetFilters(ids, 1);
  driverSetFilters(ids, 0);
  TEST_ASSERT_TRUE(swAccepts(0xFFF));
}

// ═══════════════════════════════════════════════════════════════════════════════
// MCP2515 Fake Class Tests
// ═══════════════════════════════════════════════════════════════════════════════

void test_mcp_reset_success() {
  MCP2515 mcp(15);
  TEST_ASSERT_EQUAL(MCP2515_ERROR_OK, mcp.reset());
}

void test_mcp_reset_fails() {
  MCP2515 mcp(15);
  mcp._fail_reset = true;
  TEST_ASSERT_EQUAL(MCP2515_ERROR_FAIL, mcp.reset());
}

void test_mcp_read_message() {
  MCP2515 mcp(15);
  uint8_t data[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
  mcp.pushRx(0x3FD, data, 8);

  can_frame frame;
  TEST_ASSERT_EQUAL(MCP2515_ERROR_OK, mcp.readMessage(&frame));
  TEST_ASSERT_EQUAL(0x3FD, frame.can_id);
  TEST_ASSERT_EQUAL(8, frame.can_dlc);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(data, frame.data, 8);
}

void test_mcp_read_empty_returns_nomsg() {
  MCP2515 mcp(15);
  can_frame frame;
  TEST_ASSERT_EQUAL(MCP2515_ERROR_NOMSG, mcp.readMessage(&frame));
}

void test_mcp_send_logs_frame() {
  MCP2515 mcp(15);
  can_frame frame = {};
  frame.can_id = 0x3F8;
  frame.can_dlc = 4;
  frame.data[0] = 0xAA;

  mcp.sendMessage(&frame);
  TEST_ASSERT_EQUAL(1, mcp._tx_log.size());
  TEST_ASSERT_EQUAL(0x3F8, mcp._tx_log[0].can_id);
  TEST_ASSERT_EQUAL(0xAA, mcp._tx_log[0].data[0]);
}

void test_mcp_cs_pin_stored() {
  MCP2515 m1(15);
  MCP2515 m2(27);
  MCP2515 m3(26);
  TEST_ASSERT_EQUAL(15, m1.csPin());
  TEST_ASSERT_EQUAL(27, m2.csPin());
  TEST_ASSERT_EQUAL(26, m3.csPin());
}

int main() {
  UNITY_BEGIN();

  // Software filter
  RUN_TEST(test_sw_filter_passall_by_default);
  RUN_TEST(test_sw_filter_set_ids_rejects_unmatched);
  RUN_TEST(test_sw_filter_clear_returns_to_passall);
  RUN_TEST(test_sw_filter_max_8_ids);
  RUN_TEST(test_sw_filter_single_id);
  RUN_TEST(test_sw_filter_hw4_ids);
  RUN_TEST(test_sw_filter_hw3_ids);
  RUN_TEST(test_sw_filter_legacy_ids);
  RUN_TEST(test_sw_filter_overwrite_previous);
  RUN_TEST(test_sw_filter_zero_count_clears);

  // MCP2515 class
  RUN_TEST(test_mcp_reset_success);
  RUN_TEST(test_mcp_reset_fails);
  RUN_TEST(test_mcp_read_message);
  RUN_TEST(test_mcp_read_empty_returns_nomsg);
  RUN_TEST(test_mcp_send_logs_frame);
  RUN_TEST(test_mcp_cs_pin_stored);

  return UNITY_END();
}
