// ── Variant Feature Gate Tests ────────────────────────────────────────────────
// Tests getFeatures() per variant, verifies all features default OFF,
// and checks manual variant selection + auto-detect toggling.

#include <unity.h>
#include <cstring>

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

// Forward-declare millis() so inline functions in headers can reference it
unsigned long millis();

#include "core/types.h"

#include "core/util/parse.h"
#include "feature/fsd.h"
#include "feature/nag.h"
#include "feature/offsets.h"
#include "feature/isa_chime.h"
#include "feature/summon.h"
#include "feature/variant.h"
#include "feature/ban_shield.h"
#include "feature/profile.h"
#include "feature/can_clock.h"

// ── Stubs ────────────────────────────────────────────────────────────────────
static int saveCount = 0;
static int filterCount = 0;
void saveSettings(const State &)
{
	saveCount++;
}
void resetHandlerLogFlags() {}
void applyFilters(State &)
{
	filterCount++;
}
unsigned long millis()
{
	return 1000;
}

void setUp()
{
	saveCount = 0;
	filterCount = 0;
}
void tearDown() {}

// ═══════════════════════════════════════════════════════════════════════════════
// getFeatures() per variant
// ═══════════════════════════════════════════════════════════════════════════════

// ── HW4 ──────────────────────────────────────────────────────────────────────

void test_hw4_fsd_enabled()
{
	TEST_ASSERT_TRUE(getFeatures(HW4).fsd);
}
void test_hw4_fsdForce_enabled()
{
	TEST_ASSERT_TRUE(getFeatures(HW4).fsdForce);
}
void test_hw4_offset_enabled()
{
	TEST_ASSERT_TRUE(getFeatures(HW4).offset);
}
void test_hw4_profile_enabled()
{
	TEST_ASSERT_TRUE(getFeatures(HW4).profile);
}
void test_hw4_nag_enabled()
{
	TEST_ASSERT_TRUE(getFeatures(HW4).nag);
}
void test_hw4_isaChime_enabled()
{
	TEST_ASSERT_TRUE(getFeatures(HW4).isaChime);
}
void test_hw4_summon_enabled()
{
	TEST_ASSERT_TRUE(getFeatures(HW4).summon);
}

// ── HW3 ──────────────────────────────────────────────────────────────────────

void test_hw3_fsd_enabled()
{
	TEST_ASSERT_TRUE(getFeatures(HW3).fsd);
}
void test_hw3_fsdForce_enabled()
{
	TEST_ASSERT_TRUE(getFeatures(HW3).fsdForce);
}
void test_hw3_offset_enabled()
{
	TEST_ASSERT_TRUE(getFeatures(HW3).offset);
}
void test_hw3_profile_enabled()
{
	TEST_ASSERT_TRUE(getFeatures(HW3).profile);
}
void test_hw3_nag_enabled()
{
	TEST_ASSERT_TRUE(getFeatures(HW3).nag);
}
void test_hw3_isaChime_disabled()
{
	TEST_ASSERT_FALSE(getFeatures(HW3).isaChime);
}
void test_hw3_summon_enabled()
{
	TEST_ASSERT_TRUE(getFeatures(HW3).summon);
}

// ── Legacy ───────────────────────────────────────────────────────────────────

void test_legacy_fsd_enabled()
{
	TEST_ASSERT_TRUE(getFeatures(LEGACY).fsd);
}
void test_legacy_fsdForce_enabled()
{
	TEST_ASSERT_TRUE(getFeatures(LEGACY).fsdForce);
}
void test_legacy_offset_disabled()
{
	TEST_ASSERT_FALSE(getFeatures(LEGACY).offset);
}
void test_legacy_profile_enabled()
{
	TEST_ASSERT_TRUE(getFeatures(LEGACY).profile);
}
void test_legacy_nag_enabled()
{
	TEST_ASSERT_TRUE(getFeatures(LEGACY).nag);
}
void test_legacy_isaChime_disabled()
{
	TEST_ASSERT_FALSE(getFeatures(LEGACY).isaChime);
}
void test_legacy_summon_disabled()
{
	TEST_ASSERT_FALSE(getFeatures(LEGACY).summon);
}

// ═══════════════════════════════════════════════════════════════════════════════
// All features default OFF
// ═══════════════════════════════════════════════════════════════════════════════

void test_default_fsd_off()
{
	State s = {};
	TEST_ASSERT_FALSE(s.fsdEnabled);
}

void test_default_fsdForce_off()
{
	State s = {};
	TEST_ASSERT_FALSE(s.fsdForceEnabled);
}

void test_default_nag_off()
{
	State s = {};
	TEST_ASSERT_FALSE(s.nagSuppress);
}

void test_default_isaChime_off()
{
	State s = {};
	TEST_ASSERT_FALSE(s.isaChimeSuppress);
}

void test_default_summonInject_off()
{
	State s = {};
	TEST_ASSERT_FALSE(s.summonInject);
}

void test_default_nagKiller_off()
{
	State s = {};
	TEST_ASSERT_FALSE(s.nagKillerEnabled);
}

void test_default_nagKillerMode_legacy()
{
	State s = {};
	TEST_ASSERT_EQUAL(NAG_KILLER_LEGACY, s.nagKillerMode);
}

void test_default_precondition_off()
{
	State s = {};
	TEST_ASSERT_FALSE(s.preconditionEnabled);
}

void test_default_trackMode_off()
{
	State s = {};
	TEST_ASSERT_FALSE(s.trackModeEnabled);
}

void test_default_banShield_off()
{
	State s = {};
	TEST_ASSERT_FALSE(s.banShieldEnabled);
}

void test_default_stream_off()
{
	State s = {};
	TEST_ASSERT_FALSE(s.streamEnabled);
}

void test_default_rawCan_off()
{
	State s = {};
	TEST_ASSERT_FALSE(s.rawCanListen);
}

void test_default_profileOverride_off()
{
	State s = {};
	TEST_ASSERT_FALSE(s.profileOverride);
}

void test_default_offsetOverride_off()
{
	State s = {};
	TEST_ASSERT_FALSE(s.offsetOverride);
}

void test_default_speedOffset_zero()
{
	State s = {};
	TEST_ASSERT_EQUAL(0, s.speedOffset);
}

void test_default_speedProfile_1()
{
	State s = {};
	TEST_ASSERT_EQUAL(1, s.speedProfile);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Variant selection: manual override disables auto-detect
// ═══════════════════════════════════════════════════════════════════════════════

void test_variant_hw4_manual()
{
	State s = {};
	s.variant = HW3;
	s.variantAutoDetect = true;
	TEST_ASSERT_TRUE(executeVariantCmd("variant:hw4", s));
	TEST_ASSERT_EQUAL(HW4, s.variant);
	TEST_ASSERT_FALSE(s.variantAutoDetect);
	TEST_ASSERT_EQUAL(1, saveCount);
	TEST_ASSERT_EQUAL(1, filterCount);
}

void test_variant_hw3_manual()
{
	State s = {};
	s.variant = HW4;
	s.variantAutoDetect = true;
	TEST_ASSERT_TRUE(executeVariantCmd("variant:hw3", s));
	TEST_ASSERT_EQUAL(HW3, s.variant);
	TEST_ASSERT_FALSE(s.variantAutoDetect);
}

void test_variant_legacy_manual()
{
	State s = {};
	s.variant = HW4;
	s.variantAutoDetect = true;
	TEST_ASSERT_TRUE(executeVariantCmd("variant:legacy", s));
	TEST_ASSERT_EQUAL(LEGACY, s.variant);
	TEST_ASSERT_FALSE(s.variantAutoDetect);
}

void test_variant_auto_enables_autodetect()
{
	State s = {};
	s.variant = HW3;
	s.variantAutoDetect = false;
	TEST_ASSERT_TRUE(executeVariantCmd("variant:auto", s));
	TEST_ASSERT_TRUE(s.variantAutoDetect);
	TEST_ASSERT_EQUAL(HW3, s.variant); // variant unchanged
}

void test_variant_rejects_invalid()
{
	State s = {};
	TEST_ASSERT_FALSE(executeVariantCmd("variant:hw5", s));
	TEST_ASSERT_FALSE(executeVariantCmd("variant:", s));
	TEST_ASSERT_FALSE(executeVariantCmd("variant:HW4", s));
}

void test_variant_manual_then_auto_roundtrip()
{
	State s = {};
	s.variant = HW4;
	s.variantAutoDetect = true;

	// Manual override
	TEST_ASSERT_TRUE(executeVariantCmd("variant:hw3", s));
	TEST_ASSERT_EQUAL(HW3, s.variant);
	TEST_ASSERT_FALSE(s.variantAutoDetect);

	// Re-enable auto
	TEST_ASSERT_TRUE(executeVariantCmd("variant:auto", s));
	TEST_ASSERT_TRUE(s.variantAutoDetect);
	TEST_ASSERT_EQUAL(HW3, s.variant); // still HW3 until CAN detects

	// Manual override again
	TEST_ASSERT_TRUE(executeVariantCmd("variant:legacy", s));
	TEST_ASSERT_EQUAL(LEGACY, s.variant);
	TEST_ASSERT_FALSE(s.variantAutoDetect);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Command guards per variant: rejected when feature unavailable
// ═══════════════════════════════════════════════════════════════════════════════

void test_hw3_rejects_isa_chime()
{
	State s = {};
	s.variant = HW3;
	TEST_ASSERT_FALSE(executeIsaChimeCmd("isa-chime:on", s));
}

void test_legacy_rejects_isa_chime()
{
	State s = {};
	s.variant = LEGACY;
	TEST_ASSERT_FALSE(executeIsaChimeCmd("isa-chime:on", s));
}

void test_legacy_rejects_summon()
{
	State s = {};
	s.variant = LEGACY;
	s.hasCtrl = true;
	s.summonInject = true;
	TEST_ASSERT_FALSE(executeSummonCmd("summon:forward", s));
}

void test_legacy_rejects_offset()
{
	State s = {};
	s.variant = LEGACY;
	TEST_ASSERT_FALSE(executeOffsetCmd("offset:10", s));
}

void test_hw4_accepts_offset_routes_hw4()
{
	State s = {};
	s.variant = HW4;
	TEST_ASSERT_TRUE(executeOffsetCmd("offset:16", s));
	TEST_ASSERT_EQUAL(16, s.speedOffset);
}

void test_hw3_accepts_offset_routes_legacy()
{
	State s = {};
	s.variant = HW3;
	TEST_ASSERT_TRUE(executeOffsetCmd("offset:50", s));
	TEST_ASSERT_EQUAL(50, s.speedOffset);
	TEST_ASSERT_TRUE(s.offsetOverride);
}

void test_hw4_rejects_offset_above_63()
{
	State s = {};
	s.variant = HW4;
	TEST_ASSERT_FALSE(executeOffsetCmd("offset:64", s));
}

void test_hw3_rejects_offset_above_100()
{
	State s = {};
	s.variant = HW3;
	TEST_ASSERT_FALSE(executeOffsetCmd("offset:101", s));
}

// ═══════════════════════════════════════════════════════════════════════════════

int main()
{
	UNITY_BEGIN();

	// getFeatures() — HW4
	RUN_TEST(test_hw4_fsd_enabled);
	RUN_TEST(test_hw4_fsdForce_enabled);
	RUN_TEST(test_hw4_offset_enabled);
	RUN_TEST(test_hw4_profile_enabled);
	RUN_TEST(test_hw4_nag_enabled);
	RUN_TEST(test_hw4_isaChime_enabled);
	RUN_TEST(test_hw4_summon_enabled);

	// getFeatures() — HW3
	RUN_TEST(test_hw3_fsd_enabled);
	RUN_TEST(test_hw3_fsdForce_enabled);
	RUN_TEST(test_hw3_offset_enabled);
	RUN_TEST(test_hw3_profile_enabled);
	RUN_TEST(test_hw3_nag_enabled);
	RUN_TEST(test_hw3_isaChime_disabled);
	RUN_TEST(test_hw3_summon_enabled);

	// getFeatures() — Legacy
	RUN_TEST(test_legacy_fsd_enabled);
	RUN_TEST(test_legacy_fsdForce_enabled);
	RUN_TEST(test_legacy_offset_disabled);
	RUN_TEST(test_legacy_profile_enabled);
	RUN_TEST(test_legacy_nag_enabled);
	RUN_TEST(test_legacy_isaChime_disabled);
	RUN_TEST(test_legacy_summon_disabled);

	// Default state — all features OFF
	RUN_TEST(test_default_fsd_off);
	RUN_TEST(test_default_fsdForce_off);
	RUN_TEST(test_default_nag_off);
	RUN_TEST(test_default_isaChime_off);
	RUN_TEST(test_default_summonInject_off);
	RUN_TEST(test_default_nagKiller_off);
	RUN_TEST(test_default_nagKillerMode_legacy);
	RUN_TEST(test_default_precondition_off);
	RUN_TEST(test_default_trackMode_off);
	RUN_TEST(test_default_banShield_off);
	RUN_TEST(test_default_stream_off);
	RUN_TEST(test_default_rawCan_off);
	RUN_TEST(test_default_profileOverride_off);
	RUN_TEST(test_default_offsetOverride_off);
	RUN_TEST(test_default_speedOffset_zero);
	RUN_TEST(test_default_speedProfile_1);

	// Variant selection — manual override
	RUN_TEST(test_variant_hw4_manual);
	RUN_TEST(test_variant_hw3_manual);
	RUN_TEST(test_variant_legacy_manual);
	RUN_TEST(test_variant_auto_enables_autodetect);
	RUN_TEST(test_variant_rejects_invalid);
	RUN_TEST(test_variant_manual_then_auto_roundtrip);

	// Command guards per variant
	RUN_TEST(test_hw3_rejects_isa_chime);
	RUN_TEST(test_legacy_rejects_isa_chime);
	RUN_TEST(test_legacy_rejects_summon);
	RUN_TEST(test_legacy_rejects_offset);
	RUN_TEST(test_hw4_accepts_offset_routes_hw4);
	RUN_TEST(test_hw3_accepts_offset_routes_legacy);
	RUN_TEST(test_hw4_rejects_offset_above_63);
	RUN_TEST(test_hw3_rejects_offset_above_100);

	return UNITY_END();
}
