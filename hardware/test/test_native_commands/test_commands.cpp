// ── FSD Command Handler Tests ─────────────────────────────────────────────────
// Tests the real executeProfileCmd, executeFsdCmd, executeNagCmd, executeOffsetCmd,
// executeIsaChimeCmd, executeSummonCmd and executeVariantCmd from command/fsd.h.

#include <unity.h>
#include <cstring>
#include <cstdio>

#define BUS_FSD_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

#include "core/types.h"
#include "protocol/summon.h"

// ── Stubs for save/dispatch/applyFilters ────────────────────────────────────
static int saveCount = 0;
static int resetCount = 0;
static int filterCount = 0;

void saveSettings(const State&) { saveCount++; }
void resetHandlerLogFlags()     { resetCount++; }
void applyFilters(State&)       { filterCount++; }

// Need parseBoolCmd before fsd.h
#include "command/system.h"
#include "command/fsd_toggle.h"
#include "command/nag.h"
#include "command/profile.h"
#include "command/offset.h"
#include "command/isa_chime.h"
#include "command/summon_inject.h"
#include "command/summon_cmd.h"
#include "command/variant.h"

static State makeState(Variant v = HW4) {
  State s = {};
  s.variant = v;
  s.speedProfile = 1;
  return s;
}

void setUp() {
  saveCount = 0;
  resetCount = 0;
  filterCount = 0;
}
void tearDown() {}

// ═══════════════════════════════════════════════════════════════════════════════
// executeProfileCmd
// ═══════════════════════════════════════════════════════════════════════════════

void test_profile_sets_value_and_pins() {
  State s = makeState();
  TEST_ASSERT_TRUE(executeProfileCmd("profile:2", s));
  TEST_ASSERT_EQUAL(2, s.speedProfile);
  TEST_ASSERT_TRUE(s.profileOverride);
  TEST_ASSERT_EQUAL(1, saveCount);
}

void test_profile_auto_unpins() {
  State s = makeState();
  s.profileOverride = true;
  s.speedProfile = 3;
  TEST_ASSERT_TRUE(executeProfileCmd("profile:auto", s));
  TEST_ASSERT_FALSE(s.profileOverride);
  TEST_ASSERT_EQUAL(3, s.speedProfile); // value unchanged
}

void test_profile_rejects_negative() {
  State s = makeState();
  TEST_ASSERT_FALSE(executeProfileCmd("profile:-1", s));
  TEST_ASSERT_EQUAL(1, s.speedProfile); // unchanged
}

void test_profile_rejects_above_4() {
  State s = makeState();
  TEST_ASSERT_FALSE(executeProfileCmd("profile:5", s));
  TEST_ASSERT_EQUAL(1, s.speedProfile); // unchanged
}

void test_profile_accepts_0_through_4() {
  for (int i = 0; i <= 4; i++) {
    State s = makeState();
    char cmd[16];
    snprintf(cmd, sizeof(cmd), "profile:%d", i);
    TEST_ASSERT_TRUE(executeProfileCmd(cmd, s));
    TEST_ASSERT_EQUAL(i, s.speedProfile);
  }
}

void test_profile_sp_alias() {
  State s = makeState();
  TEST_ASSERT_TRUE(executeProfileCmd("sp:3", s));
  TEST_ASSERT_EQUAL(3, s.speedProfile);
  TEST_ASSERT_TRUE(s.profileOverride);
}

void test_profile_ignores_unrelated() {
  State s = makeState();
  TEST_ASSERT_FALSE(executeProfileCmd("fsd:on", s));
  TEST_ASSERT_FALSE(executeProfileCmd("nag:off", s));
}

// ═══════════════════════════════════════════════════════════════════════════════
// executeFsdCmd
// ═══════════════════════════════════════════════════════════════════════════════

void test_fsd_on() {
  State s = makeState();
  TEST_ASSERT_TRUE(executeFsdCmd("fsd:on", s));
  TEST_ASSERT_TRUE(s.fsdEnabled);
}

void test_fsd_off() {
  State s = makeState();
  s.fsdEnabled = true;
  TEST_ASSERT_TRUE(executeFsdCmd("fsd:off", s));
  TEST_ASSERT_FALSE(s.fsdEnabled);
}

void test_fsd_toggle() {
  State s = makeState();
  s.fsdEnabled = false;
  TEST_ASSERT_TRUE(executeFsdCmd("fsd:toggle", s));
  TEST_ASSERT_TRUE(s.fsdEnabled);
  setUp();
  TEST_ASSERT_TRUE(executeFsdCmd("fsd:toggle", s));
  TEST_ASSERT_FALSE(s.fsdEnabled);
}

void test_fsd_saves() {
  State s = makeState();
  executeFsdCmd("fsd:on", s);
  TEST_ASSERT_EQUAL(1, saveCount);
  TEST_ASSERT_EQUAL(1, resetCount);
}

void test_fsd_rejects_invalid() {
  State s = makeState();
  TEST_ASSERT_FALSE(executeFsdCmd("fsd:maybe", s));
}

// ═══════════════════════════════════════════════════════════════════════════════
// executeNagCmd
// ═══════════════════════════════════════════════════════════════════════════════

void test_nag_on() {
  State s = makeState();
  TEST_ASSERT_TRUE(executeNagCmd("nag:on", s));
  TEST_ASSERT_TRUE(s.nagSuppress);
}

void test_nag_off() {
  State s = makeState();
  s.nagSuppress = true;
  TEST_ASSERT_TRUE(executeNagCmd("nag:off", s));
  TEST_ASSERT_FALSE(s.nagSuppress);
}

// ═══════════════════════════════════════════════════════════════════════════════
// executeOffsetCmd
// ═══════════════════════════════════════════════════════════════════════════════

void test_offset_sets_value() {
  State s = makeState(HW3); // HW3 has speedOffset feature
  TEST_ASSERT_TRUE(executeOffsetCmd("offset:50", s));
  TEST_ASSERT_EQUAL(50, s.speedOffset);
  TEST_ASSERT_TRUE(s.offsetOverride);
}

void test_offset_auto() {
  State s = makeState(HW3);
  s.offsetOverride = true;
  TEST_ASSERT_TRUE(executeOffsetCmd("offset:auto", s));
  TEST_ASSERT_FALSE(s.offsetOverride);
}

void test_offset_rejects_above_100() {
  State s = makeState(HW3);
  TEST_ASSERT_FALSE(executeOffsetCmd("offset:101", s));
}

void test_offset_rejects_without_feature() {
  State s = makeState(LEGACY); // LEGACY doesn't have speedOffset
  TEST_ASSERT_FALSE(executeOffsetCmd("offset:50", s));
}

// ═══════════════════════════════════════════════════════════════════════════════
// executeIsaChimeCmd
// ═══════════════════════════════════════════════════════════════════════════════

void test_isa_on_hw4() {
  State s = makeState(HW4); // HW4 has isaChime
  TEST_ASSERT_TRUE(executeIsaChimeCmd("isa-chime:on", s));
  TEST_ASSERT_TRUE(s.isaChimeSuppress);
}

void test_isa_off_hw4() {
  State s = makeState(HW4);
  s.isaChimeSuppress = true;
  TEST_ASSERT_TRUE(executeIsaChimeCmd("isa-chime:off", s));
  TEST_ASSERT_FALSE(s.isaChimeSuppress);
}

void test_isa_rejects_without_feature() {
  State s = makeState(LEGACY); // LEGACY doesn't have isaChime
  TEST_ASSERT_FALSE(executeIsaChimeCmd("isa-chime:on", s));
}

// ═══════════════════════════════════════════════════════════════════════════════
// executeSummonCmd
// ═══════════════════════════════════════════════════════════════════════════════

void test_summon_forward() {
  State s = makeState(HW4);
  s.hasCtrl = true;
  s.summonInject = true;
  TEST_ASSERT_TRUE(executeSummonCmd("summon:fwd", s));
  TEST_ASSERT_EQUAL(SUMMON_FORWARD, s.summonDirection);
  TEST_ASSERT_EQUAL(SUMMON_START, s.summonMode);
  TEST_ASSERT_EQUAL(30, s.summonRemaining);
}

void test_summon_reverse() {
  State s = makeState(HW4);
  s.hasCtrl = true;
  s.summonInject = true;
  TEST_ASSERT_TRUE(executeSummonCmd("summon:rev", s));
  TEST_ASSERT_EQUAL(SUMMON_REVERSE, s.summonDirection);
}

void test_summon_stop() {
  State s = makeState(HW4);
  s.hasCtrl = true;
  s.summonRemaining = 10;
  TEST_ASSERT_TRUE(executeSummonCmd("summon:stop", s));
  TEST_ASSERT_EQUAL(SUMMON_STOP, s.summonMode);
  TEST_ASSERT_EQUAL(0, s.summonRemaining);
}

void test_summon_requires_ctrl() {
  State s = makeState(HW4);
  s.hasCtrl = false;
  TEST_ASSERT_FALSE(executeSummonCmd("summon:fwd", s));
}

void test_summon_requires_feature() {
  State s = makeState(LEGACY); // no summon
  s.hasCtrl = true;
  TEST_ASSERT_FALSE(executeSummonCmd("summon:fwd", s));
}

// ═══════════════════════════════════════════════════════════════════════════════
// executeVariantCmd
// ═══════════════════════════════════════════════════════════════════════════════

void test_variant_hw3() {
  State s = makeState(HW4);
  TEST_ASSERT_TRUE(executeVariantCmd("variant:hw3", s));
  TEST_ASSERT_EQUAL(HW3, s.variant);
  TEST_ASSERT_EQUAL(1, filterCount);
}

void test_variant_legacy() {
  State s = makeState(HW4);
  TEST_ASSERT_TRUE(executeVariantCmd("variant:legacy", s));
  TEST_ASSERT_EQUAL(LEGACY, s.variant);
}

void test_variant_rejects_invalid() {
  State s = makeState(HW4);
  TEST_ASSERT_FALSE(executeVariantCmd("variant:unknown", s));
}

// ═══════════════════════════════════════════════════════════════════════════════

int main() {
  UNITY_BEGIN();

  // Profile
  RUN_TEST(test_profile_sets_value_and_pins);
  RUN_TEST(test_profile_auto_unpins);
  RUN_TEST(test_profile_rejects_negative);
  RUN_TEST(test_profile_rejects_above_4);
  RUN_TEST(test_profile_accepts_0_through_4);
  RUN_TEST(test_profile_sp_alias);
  RUN_TEST(test_profile_ignores_unrelated);

  // FSD
  RUN_TEST(test_fsd_on);
  RUN_TEST(test_fsd_off);
  RUN_TEST(test_fsd_toggle);
  RUN_TEST(test_fsd_saves);
  RUN_TEST(test_fsd_rejects_invalid);

  // Nag
  RUN_TEST(test_nag_on);
  RUN_TEST(test_nag_off);

  // Offset
  RUN_TEST(test_offset_sets_value);
  RUN_TEST(test_offset_auto);
  RUN_TEST(test_offset_rejects_above_100);
  RUN_TEST(test_offset_rejects_without_feature);

  // ISA Chime
  RUN_TEST(test_isa_on_hw4);
  RUN_TEST(test_isa_off_hw4);
  RUN_TEST(test_isa_rejects_without_feature);

  // Summon
  RUN_TEST(test_summon_forward);
  RUN_TEST(test_summon_reverse);
  RUN_TEST(test_summon_stop);
  RUN_TEST(test_summon_requires_ctrl);
  RUN_TEST(test_summon_requires_feature);

  // Variant
  RUN_TEST(test_variant_hw3);
  RUN_TEST(test_variant_legacy);
  RUN_TEST(test_variant_rejects_invalid);

  return UNITY_END();
}
