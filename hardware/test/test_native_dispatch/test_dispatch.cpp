// ── ESP32 Dispatch Tests ─────────────────────────────────────────────────────
// Tests handleMessage() routing, applyFilters(), summonTick(), and frame caching.
// Uses stubs to avoid hardware dependencies.

#include <unity.h>
#include <cstring>

// Native build: Arduino's __FlashStringHelper doesn't exist
class __FlashStringHelper;

// Build flags for 3-bus ESP32
#define BUS_FSD_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

#include "core/types.h"
#include "protocol/can.h"
#include "protocol/summon.h"

// ── Stubs for driver + handler functions ────────────────────────────────────
// We stub out the actual driver and handler calls, tracking what was invoked.

struct McpFilterCall {
  uint8_t idx;
  uint8_t count;
  bool cleared;  // true if ids==nullptr
};
static McpFilterCall stub_mcp_calls[8];
static uint8_t stub_mcp_call_count = 0;

bool mcpAvailable[BUS_MAX] = { true, true, true };

void driverSetBusFilters(uint8_t bus, const uint32_t* ids, uint8_t count) {
  if (stub_mcp_call_count < 8) {
    stub_mcp_calls[stub_mcp_call_count++] = { bus, count, ids == nullptr };
  }
}

struct SendCall { Frame f; uint8_t bus; };
static SendCall stub_sends[16];
static uint8_t stub_send_count = 0;

void driverSend(const Frame& f, uint8_t bus) {
  if (stub_send_count < 16) {
    stub_sends[stub_send_count].f = f;
    stub_sends[stub_send_count].bus = bus;
    stub_send_count++;
  }
}

// Stub sendLog
void sendLog(const char*) {}
void sendLog(const __FlashStringHelper*) {}

// Stub handler functions — just track calls
static int hw4_call_count = 0;
static int hw3_call_count = 0;
static int legacy_call_count = 0;

void handleHW4(Frame&, State&) { hw4_call_count++; }
void handleHW3(Frame&, State&) { hw3_call_count++; }
void handleLegacy(Frame&, State&) { legacy_call_count++; }
void resetHW4LogFlags() {}
void resetHW3LogFlags() {}
void resetLegacyLogFlags() {}

// Fake millis
static unsigned long fake_millis = 0;
unsigned long millis() { return fake_millis; }

// ── Include dispatch logic (inlined, since it's header-only) ────────────────
// We can't include dispatch/esp32.h directly because it includes handler/*.h
// and driver/esp32.h. Instead we replicate the key functions:

void applyFilters(State& s) {
  // Bus 0 (FSD): variant-specific
  if (s.rawCanListen) {
    driverSetBusFilters(0, nullptr, 0);
  } else {
    switch(s.variant) {
      case HW4: {
        static const uint32_t ids[] = {CAN_ID_ISA_SPEED, CAN_ID_FOLLOW_DIST, CAN_ID_FSD_MUX};
        driverSetBusFilters(BUS_FSD, ids, 3);
        break;
      }
      case HW3: {
        static const uint32_t ids[] = {CAN_ID_FOLLOW_DIST, CAN_ID_FSD_MUX};
        driverSetBusFilters(BUS_FSD, ids, 2);
        break;
      }
      case LEGACY: {
        static const uint32_t ids[] = {CAN_ID_LEGACY_STALK, CAN_ID_LEGACY_FSD_MUX};
        driverSetBusFilters(BUS_FSD, ids, 2);
        break;
      }
    }
  }

#if BUS_VEHICLE_ACTIVE
  // Bus 1 (Vehicle): vehicle control frames
  if (s.rawCanListen) {
    driverSetBusFilters(BUS_VEHICLE, nullptr, 0);
  } else {
    static const uint32_t vehIds[] = {CAN_ID_UI_VEHICLE_CTRL, CAN_ID_CLIMATE, CAN_ID_CHARGE, CAN_ID_DRIVE_CONFIG};
    driverSetBusFilters(BUS_VEHICLE, vehIds, 4);
  }

  // Bus 2 (Body): body control frames
  if (s.rawCanListen) {
    driverSetBusFilters(BUS_BODY, nullptr, 0);
  } else {
    static const uint32_t bodyIds[] = {CAN_ID_WINDOW_VENT, CAN_ID_SENTRY, CAN_ID_TRUNK_CTRL};
    driverSetBusFilters(BUS_BODY, bodyIds, 3);
  }
#endif
}

void handleMessage(Frame& f, uint8_t bus, State& s) {
  // Bus 0 (FSD): variant-specific handler
  if (bus == BUS_FSD) {
    switch(s.variant) {
      case HW4: handleHW4(f, s); break;
      case HW3: handleHW3(f, s); break;
      case LEGACY: handleLegacy(f, s); break;
    }
    return;
  }

#if BUS_VEHICLE_ACTIVE
  // Bus 1 (Vehicle): cache control frames
  if (bus == BUS_VEHICLE) {
    if (f.id == CAN_ID_UI_VEHICLE_CTRL && f.dlc >= 8) {
      memcpy(s.lastCtrl, f.data, 8);
      s.hasCtrl = true;
      return;
    }
    if (f.id == CAN_ID_CLIMATE && f.dlc >= 5) {
      memcpy(s.lastClimate, f.data, 5);
      s.hasClimate = true;
      return;
    }
    if (f.id == CAN_ID_CHARGE && f.dlc >= 5) {
      memcpy(s.lastCharge, f.data, 5);
      s.hasCharge = true;
      return;
    }
    if (f.id == CAN_ID_DRIVE_CONFIG && f.dlc >= 8) {
      memcpy(s.lastDrive, f.data, 8);
      s.hasDrive = true;
      return;
    }
  }
  // Bus 2 (Body): no caching needed
#endif
}

void summonTick(State& s) {
#if !BUS_VEHICLE_ACTIVE
  (void)s;
  return;
#else
  if (s.summonRemaining == 0 || !s.hasCtrl) return;
  unsigned long now = millis();
  if (now - s.summonLastMs < 20) return;
  s.summonLastMs = now;

  Frame f;
  f.id = CAN_ID_UI_VEHICLE_CTRL;
  f.dlc = 8;
  memcpy(f.data, s.lastCtrl, 8);
  setSummonActive(f, true);
  setSummonDirection(f, s.summonDirection);
  setSummonMode(f, s.summonMode);
  driverSend(f, BUS_VEHICLE);
  s.summonRemaining--;
  if (s.summonRemaining == 0) sendLog("Summon burst complete");
#endif
}

// ── Helpers ─────────────────────────────────────────────────────────────────

static State makeState(Variant v = HW4) {
  State s = {};
  s.variant = v;
  s.speedProfile = 1;
  return s;
}

static Frame makeFrame(uint32_t id, uint8_t dlc = 8) {
  Frame f = {};
  f.id = id;
  f.dlc = dlc;
  return f;
}

void setUp() {
  stub_mcp_call_count = 0;
  stub_send_count = 0;
  hw4_call_count = 0;
  hw3_call_count = 0;
  legacy_call_count = 0;
  fake_millis = 0;
}

void tearDown() {}

// ═══════════════════════════════════════════════════════════════════════════════
// handleMessage — Frame Caching
// ═══════════════════════════════════════════════════════════════════════════════

void test_dispatch_caches_ctrl_frame() {
  State s = makeState();
  Frame f = makeFrame(CAN_ID_UI_VEHICLE_CTRL);
  f.data[0] = 0xAA;
  handleMessage(f, BUS_VEHICLE, s);
  TEST_ASSERT_TRUE(s.hasCtrl);
  TEST_ASSERT_EQUAL(0xAA, s.lastCtrl[0]);
}

void test_dispatch_caches_climate_frame() {
  State s = makeState();
  Frame f = makeFrame(CAN_ID_CLIMATE, 5);
  f.data[0] = 0xBB;
  handleMessage(f, BUS_VEHICLE, s);
  TEST_ASSERT_TRUE(s.hasClimate);
  TEST_ASSERT_EQUAL(0xBB, s.lastClimate[0]);
}

void test_dispatch_caches_charge_frame() {
  State s = makeState();
  Frame f = makeFrame(CAN_ID_CHARGE, 5);
  f.data[2] = 0xCC;
  handleMessage(f, BUS_VEHICLE, s);
  TEST_ASSERT_TRUE(s.hasCharge);
  TEST_ASSERT_EQUAL(0xCC, s.lastCharge[2]);
}

void test_dispatch_caches_drive_frame() {
  State s = makeState();
  Frame f = makeFrame(CAN_ID_DRIVE_CONFIG);
  f.data[7] = 0xDD;
  handleMessage(f, BUS_VEHICLE, s);
  TEST_ASSERT_TRUE(s.hasDrive);
  TEST_ASSERT_EQUAL(0xDD, s.lastDrive[7]);
}

// ═══════════════════════════════════════════════════════════════════════════════
// handleMessage — Handler Dispatch by Variant
// ═══════════════════════════════════════════════════════════════════════════════

void test_dispatch_hw4_routes_to_hw4_handler() {
  State s = makeState(HW4);
  Frame f = makeFrame(CAN_ID_FSD_MUX);
  handleMessage(f, 0, s);
  TEST_ASSERT_EQUAL(1, hw4_call_count);
  TEST_ASSERT_EQUAL(0, hw3_call_count);
  TEST_ASSERT_EQUAL(0, legacy_call_count);
}

void test_dispatch_hw3_routes_to_hw3_handler() {
  State s = makeState(HW3);
  Frame f = makeFrame(CAN_ID_FSD_MUX);
  handleMessage(f, 0, s);
  TEST_ASSERT_EQUAL(0, hw4_call_count);
  TEST_ASSERT_EQUAL(1, hw3_call_count);
}

void test_dispatch_legacy_routes_to_legacy_handler() {
  State s = makeState(LEGACY);
  Frame f = makeFrame(CAN_ID_LEGACY_FSD_MUX);
  handleMessage(f, 0, s);
  TEST_ASSERT_EQUAL(1, legacy_call_count);
}

void test_dispatch_ignores_handler_frames_from_bus1() {
  State s = makeState(HW4);
  Frame f = makeFrame(CAN_ID_FSD_MUX);
  handleMessage(f, 1, s);  // bus 1, not bus 0
  TEST_ASSERT_EQUAL(0, hw4_call_count);
}

void test_dispatch_ctrl_short_frame_ignored() {
  State s = makeState();
  Frame f = makeFrame(CAN_ID_UI_VEHICLE_CTRL, 4);  // dlc < 8
  handleMessage(f, BUS_VEHICLE, s);
  TEST_ASSERT_FALSE(s.hasCtrl);
}

// ═══════════════════════════════════════════════════════════════════════════════
// applyFilters
// ═══════════════════════════════════════════════════════════════════════════════

void test_apply_filters_hw4_sets_fsd_3_ids() {
  State s = makeState(HW4);
  applyFilters(s);
  // Bus 0 (FSD): 3 IDs for HW4
  TEST_ASSERT_TRUE(stub_mcp_call_count >= 1);
  TEST_ASSERT_EQUAL(BUS_FSD, stub_mcp_calls[0].idx);
  TEST_ASSERT_EQUAL(3, stub_mcp_calls[0].count);
}

void test_apply_filters_hw3_sets_fsd_2_ids() {
  State s = makeState(HW3);
  applyFilters(s);
  TEST_ASSERT_TRUE(stub_mcp_call_count >= 1);
  TEST_ASSERT_EQUAL(BUS_FSD, stub_mcp_calls[0].idx);
  TEST_ASSERT_EQUAL(2, stub_mcp_calls[0].count);
}

void test_apply_filters_legacy_sets_fsd_2_ids() {
  State s = makeState(LEGACY);
  applyFilters(s);
  TEST_ASSERT_TRUE(stub_mcp_call_count >= 1);
  TEST_ASSERT_EQUAL(BUS_FSD, stub_mcp_calls[0].idx);
  TEST_ASSERT_EQUAL(2, stub_mcp_calls[0].count);
}

void test_apply_filters_raw_can_clears_all() {
  State s = makeState(HW4);
  s.rawCanListen = true;
  applyFilters(s);
  // All 3 buses should be cleared
  for (uint8_t i = 0; i < stub_mcp_call_count; i++) {
    TEST_ASSERT_TRUE(stub_mcp_calls[i].cleared);
  }
}

void test_apply_filters_sets_vehicle_and_body_buses() {
  State s = makeState(HW4);
  applyFilters(s);
  // 3-bus config: bus 0 (FSD) + bus 1 (vehicle, 4 IDs) + bus 2 (body, 3 IDs)
  TEST_ASSERT_EQUAL(3, stub_mcp_call_count);
  TEST_ASSERT_EQUAL(BUS_VEHICLE, stub_mcp_calls[1].idx);
  TEST_ASSERT_EQUAL(4, stub_mcp_calls[1].count);
  TEST_ASSERT_EQUAL(BUS_BODY, stub_mcp_calls[2].idx);
  TEST_ASSERT_EQUAL(3, stub_mcp_calls[2].count);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Summon Tick
// ═══════════════════════════════════════════════════════════════════════════════

void test_summon_tick_does_nothing_when_remaining_zero() {
  State s = makeState();
  s.summonRemaining = 0;
  summonTick(s);
  TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_summon_tick_does_nothing_without_ctrl() {
  State s = makeState();
  s.summonRemaining = 5;
  s.hasCtrl = false;
  summonTick(s);
  TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_summon_tick_sends_burst_frame() {
  State s = makeState();
  s.summonRemaining = 3;
  s.hasCtrl = true;
  s.summonDirection = SUMMON_FORWARD;
  s.summonMode = SUMMON_START;
  memset(s.lastCtrl, 0, 8);
  fake_millis = 100;
  s.summonLastMs = 0;

  summonTick(s);
  TEST_ASSERT_EQUAL(1, stub_send_count);
  TEST_ASSERT_EQUAL(2, s.summonRemaining);
  TEST_ASSERT_EQUAL(CAN_ID_UI_VEHICLE_CTRL, stub_sends[0].f.id);
  TEST_ASSERT_EQUAL(BUS_VEHICLE, stub_sends[0].bus);
  // Check summon bits: active (bit4) + start (bit0)
  TEST_ASSERT_BITS(0x11, 0x11, stub_sends[0].f.data[0]);
}

void test_summon_tick_respects_20ms_interval() {
  State s = makeState();
  s.summonRemaining = 5;
  s.hasCtrl = true;
  memset(s.lastCtrl, 0, 8);
  s.summonLastMs = 90;
  fake_millis = 100;  // only 10ms since last — too soon

  summonTick(s);
  TEST_ASSERT_EQUAL(0, stub_send_count);
  TEST_ASSERT_EQUAL(5, s.summonRemaining);
}

void test_summon_tick_decrements_to_zero() {
  State s = makeState();
  s.summonRemaining = 1;
  s.hasCtrl = true;
  memset(s.lastCtrl, 0, 8);
  s.summonLastMs = 0;
  fake_millis = 100;

  summonTick(s);
  TEST_ASSERT_EQUAL(0, s.summonRemaining);
}

void test_summon_tick_reverse_direction() {
  State s = makeState();
  s.summonRemaining = 1;
  s.hasCtrl = true;
  s.summonDirection = SUMMON_REVERSE;
  s.summonMode = SUMMON_START;
  memset(s.lastCtrl, 0, 8);
  s.summonLastMs = 0;
  fake_millis = 100;

  summonTick(s);
  // Check reverse bit (bit5) set
  TEST_ASSERT_BITS(0x20, 0x20, stub_sends[0].f.data[0]);
}

int main() {
  UNITY_BEGIN();

  // Frame caching
  RUN_TEST(test_dispatch_caches_ctrl_frame);
  RUN_TEST(test_dispatch_caches_climate_frame);
  RUN_TEST(test_dispatch_caches_charge_frame);
  RUN_TEST(test_dispatch_caches_drive_frame);
  RUN_TEST(test_dispatch_ctrl_short_frame_ignored);

  // Handler routing
  RUN_TEST(test_dispatch_hw4_routes_to_hw4_handler);
  RUN_TEST(test_dispatch_hw3_routes_to_hw3_handler);
  RUN_TEST(test_dispatch_legacy_routes_to_legacy_handler);
  RUN_TEST(test_dispatch_ignores_handler_frames_from_bus1);

  // Filters
  RUN_TEST(test_apply_filters_hw4_sets_fsd_3_ids);
  RUN_TEST(test_apply_filters_hw3_sets_fsd_2_ids);
  RUN_TEST(test_apply_filters_legacy_sets_fsd_2_ids);
  RUN_TEST(test_apply_filters_raw_can_clears_all);
  RUN_TEST(test_apply_filters_sets_vehicle_and_body_buses);

  // Summon
  RUN_TEST(test_summon_tick_does_nothing_when_remaining_zero);
  RUN_TEST(test_summon_tick_does_nothing_without_ctrl);
  RUN_TEST(test_summon_tick_sends_burst_frame);
  RUN_TEST(test_summon_tick_respects_20ms_interval);
  RUN_TEST(test_summon_tick_decrements_to_zero);
  RUN_TEST(test_summon_tick_reverse_direction);

  return UNITY_END();
}
