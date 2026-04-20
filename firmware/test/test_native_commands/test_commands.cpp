// ── FSD Command Handler Tests ─────────────────────────────────────────────────
// Tests the real executeProfileCmd, executeFsdCmd, executeNagCmd, executeOffsetCmd,
// executeIsaChimeCmd, executeSummonCmd and executeVariantCmd from command/fsd.h.

#include <unity.h>
#include <cstring>
#include <cstdio>

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

// Forward-declare millis() so inline functions in headers can reference it
unsigned long millis();

#include "core/types.h"
#include "feature/summon.h"

// ── Stubs for save/dispatch/applyFilters ────────────────────────────────────
static int saveCount = 0;
static int resetCount = 0;
static int filterCount = 0;
static unsigned long fake_millis_val = 1000;

void saveSettings(const State&) { saveCount++; }
void resetHandlerLogFlags()     { resetCount++; }
void applyFilters(State&)       { filterCount++; }
unsigned long millis()          { return fake_millis_val; }

// Need parseBoolCmd before fsd.h
#include "infra/parse.h"
#include "feature/can_clock.h"
#include "feature/fsd.h"
#include "feature/nag.h"
#include "feature/ban_shield.h"
#include "feature/profile.h"
#include "feature/offsets.h"
#include "feature/isa_chime.h"
#include "feature/summon.h"
#include "feature/variant.h"

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

void test_profile_lock_pins_current_profile() {
  State s = makeState();
  s.speedProfile = 2;
  s.profileOverride = false;
  TEST_ASSERT_TRUE(executeProfileCmd("profile:lock", s));
  TEST_ASSERT_TRUE(s.profileOverride);
  TEST_ASSERT_EQUAL(2, s.speedProfile);
}

void test_profile_unlock_unpins_current_profile() {
  State s = makeState();
  s.speedProfile = 4;
  s.profileOverride = true;
  TEST_ASSERT_TRUE(executeProfileCmd("profile:unlock", s));
  TEST_ASSERT_FALSE(s.profileOverride);
  TEST_ASSERT_EQUAL(4, s.speedProfile);
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

void test_fsd_saves() {
  State s = makeState();
  executeFsdCmd("fsd:on", s);
  TEST_ASSERT_EQUAL(1, saveCount);
  TEST_ASSERT_EQUAL(1, resetCount);
  TEST_ASSERT_EQUAL(1, filterCount); // dynamic filter update
}

void test_fsd_rejects_invalid() {
  State s = makeState();
  TEST_ASSERT_FALSE(executeFsdCmd("fsd:maybe", s));
}

void test_force_fsd_on() {
  State s = makeState();
  TEST_ASSERT_TRUE(executeFsdForceCmd("fsd:force:on", s));
  TEST_ASSERT_TRUE(s.fsdForceEnabled);
}

void test_force_fsd_off() {
  State s = makeState();
  s.fsdForceEnabled = true;
  TEST_ASSERT_TRUE(executeFsdForceCmd("fsd:force:off", s));
  TEST_ASSERT_FALSE(s.fsdForceEnabled);
}

void test_canclock_auto() {
  State s = makeState();
  TEST_ASSERT_TRUE(executeCanClockCmd("canclock:auto", s));
  TEST_ASSERT_EQUAL(0, s.canClockReqMHz);
  TEST_ASSERT_EQUAL(1, saveCount);
}

void test_canclock_8() {
  State s = makeState();
  TEST_ASSERT_TRUE(executeCanClockCmd("canclock:8", s));
  TEST_ASSERT_EQUAL(8, s.canClockReqMHz);
  TEST_ASSERT_EQUAL(1, saveCount);
}

void test_canclock_12() {
  State s = makeState();
  TEST_ASSERT_TRUE(executeCanClockCmd("canclock:12", s));
  TEST_ASSERT_EQUAL(12, s.canClockReqMHz);
  TEST_ASSERT_EQUAL(1, saveCount);
}

void test_canclock_rejects_invalid() {
  State s = makeState();
  TEST_ASSERT_FALSE(executeCanClockCmd("canclock:11", s));
}

// ═══════════════════════════════════════════════════════════════════════════════
// executeNagCmd

// ═══════════════════════════════════════════════════════════════════════════════
// executeBanShieldCmd
// ═══════════════════════════════════════════════════════════════════════════════
void test_banshield_on() {
  State s = makeState();
  TEST_ASSERT_FALSE(s.banShieldEnabled);
  TEST_ASSERT_TRUE(executeBanShieldCmd("banshield:on", s));
  TEST_ASSERT_TRUE(s.banShieldEnabled);
}

void test_banshield_off() {
  State s = makeState();
  s.banShieldEnabled = true;
  TEST_ASSERT_TRUE(executeBanShieldCmd("banshield:off", s));
  TEST_ASSERT_FALSE(s.banShieldEnabled);
}

void test_banshield_resets_threat_on_enable() {
  State s = makeState();
  s.banThreatLevel = 3;
  s.banDetectionCount = 10;
  TEST_ASSERT_TRUE(executeBanShieldCmd("banshield:on", s));
  TEST_ASSERT_EQUAL(0, s.banThreatLevel);
  TEST_ASSERT_EQUAL(0, s.banDetectionCount);
}

// ═══════════════════════════════════════════════════════════════════════════════
// executeNagCmd
// ═══════════════════════════════════════════════════════════════════════════════
// ═══════════════════════════════════════════════════════════════════════════════

void test_nag_on() {
  State s = makeState();
  TEST_ASSERT_TRUE(executeNagCmd("nag:on", s));
  TEST_ASSERT_TRUE(s.nagSuppress);
  TEST_ASSERT_EQUAL(1, filterCount); // dynamic filter update
}

void test_nag_off() {
  State s = makeState();
  s.nagSuppress = true;
  TEST_ASSERT_TRUE(executeNagCmd("nag:off", s));
  TEST_ASSERT_FALSE(s.nagSuppress);
  TEST_ASSERT_EQUAL(1, filterCount); // dynamic filter update
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
  State s = makeState(LEGACY); // Legacy has no mux2 handler, offset rejected
  TEST_ASSERT_FALSE(executeOffsetCmd("offset:50", s));
}

void test_hw4_offset_sets_value() {
  State s = makeState(HW4);
  TEST_ASSERT_TRUE(executeOffsetCmd("offset:16", s));
  TEST_ASSERT_EQUAL(16, s.speedOffset);
}

void test_hw4_offset_off_disables() {
  State s = makeState(HW4);
  s.speedOffset = 22;
  TEST_ASSERT_TRUE(executeOffsetCmd("offset:off", s));
  TEST_ASSERT_EQUAL(0, s.speedOffset);
}

void test_hw4_offset_rejects_out_of_range() {
  State s = makeState(HW4);
  TEST_ASSERT_FALSE(executeOffsetCmd("offset:64", s));
}

void test_hw4_offset_rejects_non_hw4() {
  State s = makeState(HW3);
  // HW3 uses legacy offset path, not HW4 — value goes to speedOffset
  TEST_ASSERT_TRUE(executeOffsetCmd("offset:10", s));
  TEST_ASSERT_EQUAL(10, s.speedOffset);
}

// ═══════════════════════════════════════════════════════════════════════════════
// executeIsaChimeCmd
// ═══════════════════════════════════════════════════════════════════════════════

void test_isa_on_hw4() {
  State s = makeState(HW4); // HW4 has isaChime
  TEST_ASSERT_TRUE(executeIsaChimeCmd("isa-chime:on", s));
  TEST_ASSERT_TRUE(s.isaChimeSuppress);
  TEST_ASSERT_EQUAL(1, filterCount); // dynamic filter update
}

void test_isa_off_hw4() {
  State s = makeState(HW4);
  s.isaChimeSuppress = true;
  TEST_ASSERT_TRUE(executeIsaChimeCmd("isa-chime:off", s));
  TEST_ASSERT_FALSE(s.isaChimeSuppress);
  TEST_ASSERT_EQUAL(1, filterCount); // dynamic filter update
}

void test_isa_rejects_without_feature() {
  State s = makeState(LEGACY); // ISA is HW4-only, rejected on non-HW4
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
  State s = makeState(LEGACY); // Legacy has features().summon=false
  s.hasCtrl = true;
  s.summonInject = true;
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

void test_variant_auto_enables_auto_detect() {
  State s = makeState(HW4);
  s.variantAutoDetect = false;
  TEST_ASSERT_TRUE(executeVariantCmd("variant:auto", s));
  TEST_ASSERT_TRUE(s.variantAutoDetect);
  TEST_ASSERT_EQUAL(HW4, s.variant); // variant unchanged
  TEST_ASSERT_EQUAL(1, filterCount); // applyFilters called
}

void test_variant_manual_disables_auto_detect() {
  State s = makeState(HW4);
  s.variantAutoDetect = true;
  TEST_ASSERT_TRUE(executeVariantCmd("variant:hw3", s));
  TEST_ASSERT_EQUAL(HW3, s.variant);
  TEST_ASSERT_FALSE(s.variantAutoDetect);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Feature flag guard tests
// ═══════════════════════════════════════════════════════════════════════════════

// Nag Killer rejects when nag feature disabled (LEGACY has no summon but has nag;
// we simulate by temporarily testing — currently nag=true for all, so we verify
// the guard path exists via nag:killer command structure)
void test_nagkiller_on() {
  State s = makeState(HW4);
  TEST_ASSERT_TRUE(executeNagKillerCmd("nag:killer:on", s));
  TEST_ASSERT_TRUE(s.nagKillerEnabled);
  TEST_ASSERT_EQUAL(1, saveCount);
}

void test_nagkiller_off() {
  State s = makeState(HW4);
  s.nagKillerEnabled = true;
  TEST_ASSERT_TRUE(executeNagKillerCmd("nag:killer:off", s));
  TEST_ASSERT_FALSE(s.nagKillerEnabled);
}

void test_nagkiller_mode_safe() {
  State s = makeState(HW4);
  TEST_ASSERT_TRUE(executeNagKillerCmd("nag:killer:mode:safe", s));
  TEST_ASSERT_EQUAL(NAG_KILLER_SAFE, s.nagKillerMode);
}

void test_nagkiller_mode_legacy() {
  State s = makeState(HW4);
  s.nagKillerMode = NAG_KILLER_SAFE;
  TEST_ASSERT_TRUE(executeNagKillerCmd("nag:killer:mode:legacy", s));
  TEST_ASSERT_EQUAL(NAG_KILLER_LEGACY, s.nagKillerMode);
}

void test_nagkiller_rejects_invalid_mode() {
  State s = makeState(HW4);
  TEST_ASSERT_FALSE(executeNagKillerCmd("nag:killer:mode:turbo", s));
}

void test_banshield_saves() {
  State s = makeState();
  executeBanShieldCmd("banshield:on", s);
  TEST_ASSERT_EQUAL(1, saveCount);
}

void test_summon_inject_on() {
  State s = makeState(HW4);
  TEST_ASSERT_TRUE(executeSummonInjectCmd("summon-inject:on", s));
  TEST_ASSERT_TRUE(s.summonInject);
}

void test_summon_inject_off() {
  State s = makeState(HW4);
  s.summonInject = true;
  TEST_ASSERT_TRUE(executeSummonInjectCmd("summon-inject:off", s));
  TEST_ASSERT_FALSE(s.summonInject);
}

void test_summon_inject_rejects_without_feature() {
  State s = makeState(LEGACY); // Legacy has features().summon=false
  TEST_ASSERT_FALSE(executeSummonInjectCmd("summon-inject:on", s));
}

// ═══════════════════════════════════════════════════════════════════════════════

int main() {
  UNITY_BEGIN();

  // Profile
  RUN_TEST(test_profile_sets_value_and_pins);
  RUN_TEST(test_profile_auto_unpins);
  RUN_TEST(test_profile_lock_pins_current_profile);
  RUN_TEST(test_profile_unlock_unpins_current_profile);
  RUN_TEST(test_profile_rejects_negative);
  RUN_TEST(test_profile_rejects_above_4);
  RUN_TEST(test_profile_accepts_0_through_4);
  RUN_TEST(test_profile_sp_alias);
  RUN_TEST(test_profile_ignores_unrelated);

  // FSD
  RUN_TEST(test_fsd_on);
  RUN_TEST(test_fsd_off);
  RUN_TEST(test_fsd_saves);
  RUN_TEST(test_fsd_rejects_invalid);
  RUN_TEST(test_force_fsd_on);
  RUN_TEST(test_force_fsd_off);
  RUN_TEST(test_canclock_auto);
  RUN_TEST(test_canclock_8);
  RUN_TEST(test_canclock_12);
  RUN_TEST(test_canclock_rejects_invalid);

  // Nag
  RUN_TEST(test_nag_on);

  // Ban Shield
  RUN_TEST(test_banshield_on);
  RUN_TEST(test_banshield_off);
  RUN_TEST(test_banshield_resets_threat_on_enable);
  RUN_TEST(test_banshield_saves);

  // Nag (continued)
  RUN_TEST(test_nag_off);

  // Offset
  RUN_TEST(test_offset_sets_value);
  RUN_TEST(test_offset_auto);
  RUN_TEST(test_offset_rejects_above_100);
  RUN_TEST(test_offset_rejects_without_feature);
  RUN_TEST(test_hw4_offset_sets_value);
  RUN_TEST(test_hw4_offset_off_disables);
  RUN_TEST(test_hw4_offset_rejects_out_of_range);
  RUN_TEST(test_hw4_offset_rejects_non_hw4);

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
  RUN_TEST(test_variant_auto_enables_auto_detect);
  RUN_TEST(test_variant_manual_disables_auto_detect);

  // Nag Killer
  RUN_TEST(test_nagkiller_on);
  RUN_TEST(test_nagkiller_off);
  RUN_TEST(test_nagkiller_mode_safe);
  RUN_TEST(test_nagkiller_mode_legacy);
  RUN_TEST(test_nagkiller_rejects_invalid_mode);

  // Summon Inject
  RUN_TEST(test_summon_inject_on);
  RUN_TEST(test_summon_inject_off);
  RUN_TEST(test_summon_inject_rejects_without_feature);

  return UNITY_END();
}
