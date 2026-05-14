/**
 * @file firmware/test/test_native_commands/test_commands.cpp
 * @brief Unit tests for FSD, profile, nag, offset, ISA chime, summon, and variant commands
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <unity.h>
#include <cstring>
#include <cstdio>

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

unsigned long millis();

#include "core/types.h"
#include "feature/body/summon.h"

static int saveCount = 0;
static int resetCount = 0;
static int filterCount = 0;
static unsigned long fake_millis_val = 1000;

void saveSettings(const State &)
{
	saveCount++;
}
void resetHandlerLogFlags()
{
	resetCount++;
}
void applyFilters(State &)
{
	filterCount++;
}
unsigned long millis()
{
	return fake_millis_val;
}

#include "core/util/parse.h"
#include "feature/misc/can_clock.h"
#include "feature/fsd/fsd.h"
#include "feature/fsd/nag.h"
#include "feature/safety/ban_shield.h"
#include "feature/fsd/profile.h"
#include "feature/fsd/offsets.h"
#include "feature/fsd/isa_chime.h"
#include "feature/body/summon.h"
#include "feature/misc/variant.h"

static State makeState(Variant v = HW4)
{
	State s = {};
	s.variant = v;
	s.speedProfile = 1;
	return s;
}

/** @brief Reset call counters before each test */
void setUp()
{
	saveCount = 0;
	resetCount = 0;
	filterCount = 0;
}

/** @brief Cleanup after each test */
void tearDown() {}

/** @brief Setting a numeric profile pins it and triggers a save */
void test_profile_sets_value_and_pins()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeProfileCmd("profile:2", s));
	TEST_ASSERT_EQUAL(2, s.speedProfile);
	TEST_ASSERT_TRUE(s.profileOverride);
	TEST_ASSERT_EQUAL(1, saveCount);
}

/** @brief "profile:auto" unpins the profile without changing the current value */
void test_profile_auto_unpins()
{
	State s = makeState();
	s.profileOverride = true;
	s.speedProfile = 3;
	TEST_ASSERT_TRUE(executeProfileCmd("profile:auto", s));
	TEST_ASSERT_FALSE(s.profileOverride);
	TEST_ASSERT_EQUAL(3, s.speedProfile);
}

/** @brief "profile:lock" pins the current profile value */
void test_profile_lock_pins_current_profile()
{
	State s = makeState();
	s.speedProfile = 2;
	s.profileOverride = false;
	TEST_ASSERT_TRUE(executeProfileCmd("profile:lock", s));
	TEST_ASSERT_TRUE(s.profileOverride);
	TEST_ASSERT_EQUAL(2, s.speedProfile);
}

/** @brief "profile:unlock" unpins the current profile value */
void test_profile_unlock_unpins_current_profile()
{
	State s = makeState();
	s.speedProfile = 4;
	s.profileOverride = true;
	TEST_ASSERT_TRUE(executeProfileCmd("profile:unlock", s));
	TEST_ASSERT_FALSE(s.profileOverride);
	TEST_ASSERT_EQUAL(4, s.speedProfile);
}

/** @brief Negative profile values are rejected */
void test_profile_rejects_negative()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeProfileCmd("profile:-1", s));
	TEST_ASSERT_EQUAL(1, s.speedProfile);
}

/** @brief Profile values above 4 are rejected */
void test_profile_rejects_above_4()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeProfileCmd("profile:5", s));
	TEST_ASSERT_EQUAL(1, s.speedProfile);
}

/** @brief Profile values 0 through 4 are all accepted */
void test_profile_accepts_0_through_4()
{
	for (int i = 0; i <= 4; i++)
	{
		State s = makeState();
		char cmd[16];
		snprintf(cmd, sizeof(cmd), "profile:%d", i);
		TEST_ASSERT_TRUE(executeProfileCmd(cmd, s));
		TEST_ASSERT_EQUAL(i, s.speedProfile);
	}
}

/** @brief "sp:" prefix is accepted as an alias for "profile:" */
void test_profile_sp_alias()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeProfileCmd("sp:3", s));
	TEST_ASSERT_EQUAL(3, s.speedProfile);
	TEST_ASSERT_TRUE(s.profileOverride);
}

/** @brief Profile handler ignores commands belonging to other features */
void test_profile_ignores_unrelated()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeProfileCmd("fsd:on", s));
	TEST_ASSERT_FALSE(executeProfileCmd("nag:off", s));
}

/** @brief "fsd:on" enables FSD injection */
void test_fsd_on()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeFsdCmd("fsd:on", s));
	TEST_ASSERT_TRUE(s.fsdEnabled);
}

/** @brief "fsd:off" disables FSD injection */
void test_fsd_off()
{
	State s = makeState();
	s.fsdEnabled = true;
	TEST_ASSERT_TRUE(executeFsdCmd("fsd:off", s));
	TEST_ASSERT_FALSE(s.fsdEnabled);
}

/** @brief FSD toggle triggers save, handler reset, and filter reapply */
void test_fsd_saves()
{
	State s = makeState();
	executeFsdCmd("fsd:on", s);
	TEST_ASSERT_EQUAL(1, saveCount);
	TEST_ASSERT_EQUAL(1, resetCount);
	TEST_ASSERT_EQUAL(1, filterCount);
}

/** @brief Invalid FSD subcommands are rejected */
void test_fsd_rejects_invalid()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeFsdCmd("fsd:maybe", s));
}

/** @brief "fsd:force:on" enables forced FSD mode */
void test_force_fsd_on()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeFsdForceCmd("fsd:force:on", s));
	TEST_ASSERT_TRUE(s.fsdForceEnabled);
}

/** @brief "fsd:force:off" disables forced FSD mode */
void test_force_fsd_off()
{
	State s = makeState();
	s.fsdForceEnabled = true;
	TEST_ASSERT_TRUE(executeFsdForceCmd("fsd:force:off", s));
	TEST_ASSERT_FALSE(s.fsdForceEnabled);
}

/** @brief "canclock:auto" resets requested clock to zero (auto-detect) */
void test_canclock_auto()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeCanClockCmd("canclock:auto", s));
	TEST_ASSERT_EQUAL(0, s.canClockReqMHz);
	TEST_ASSERT_EQUAL(1, saveCount);
}

/** @brief "canclock:8" sets requested CAN clock to 8 MHz */
void test_canclock_8()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeCanClockCmd("canclock:8", s));
	TEST_ASSERT_EQUAL(8, s.canClockReqMHz);
	TEST_ASSERT_EQUAL(1, saveCount);
}

/** @brief "canclock:12" sets requested CAN clock to 12 MHz */
void test_canclock_12()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeCanClockCmd("canclock:12", s));
	TEST_ASSERT_EQUAL(12, s.canClockReqMHz);
	TEST_ASSERT_EQUAL(1, saveCount);
}

/** @brief Unsupported clock frequencies are rejected */
void test_canclock_rejects_invalid()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeCanClockCmd("canclock:11", s));
}

/** @brief "banshield:on" enables ban shield */
void test_banshield_on()
{
	State s = makeState();
	TEST_ASSERT_FALSE(s.banShieldEnabled);
	TEST_ASSERT_TRUE(executeBanShieldCmd("banshield:on", s));
	TEST_ASSERT_TRUE(s.banShieldEnabled);
}

/** @brief "banshield:off" disables ban shield */
void test_banshield_off()
{
	State s = makeState();
	s.banShieldEnabled = true;
	TEST_ASSERT_TRUE(executeBanShieldCmd("banshield:off", s));
	TEST_ASSERT_FALSE(s.banShieldEnabled);
}

/** @brief Enabling ban shield resets threat level and detection count */
void test_banshield_resets_threat_on_enable()
{
	State s = makeState();
	s.banThreatLevel = 3;
	s.banDetectionCount = 10;
	TEST_ASSERT_TRUE(executeBanShieldCmd("banshield:on", s));
	TEST_ASSERT_EQUAL(0, s.banThreatLevel);
	TEST_ASSERT_EQUAL(0, s.banDetectionCount);
}

/** @brief "nag:mode:bit19" sets nag mode to BIT19 and reapplies filters */
void test_nag_mode_bit19()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeNagCmd("nag:mode:bit19", s));
	TEST_ASSERT_EQUAL(NAG_MODE_BIT19, s.nagMode);
	TEST_ASSERT_EQUAL(1, filterCount);
}

/** @brief "nag:mode:off" clears nag mode */
void test_nag_mode_off()
{
	State s = makeState();
	s.nagMode = NAG_MODE_BIT19;
	TEST_ASSERT_TRUE(executeNagCmd("nag:mode:off", s));
	TEST_ASSERT_EQUAL(NAG_MODE_OFF, s.nagMode);
	TEST_ASSERT_EQUAL(1, filterCount);
}

/** @brief Offset command sets value and enables override on HW3 */
void test_offset_sets_value()
{
	State s = makeState(HW3);
	TEST_ASSERT_TRUE(executeOffsetCmd("offset:50", s));
	TEST_ASSERT_EQUAL(50, s.speedOffset);
	TEST_ASSERT_TRUE(s.offsetOverride);
}

/** @brief "offset:auto" disables offset override */
void test_offset_auto()
{
	State s = makeState(HW3);
	s.offsetOverride = true;
	TEST_ASSERT_TRUE(executeOffsetCmd("offset:auto", s));
	TEST_ASSERT_FALSE(s.offsetOverride);
}

/** @brief Offset values above 100 are rejected on HW3 */
void test_offset_rejects_above_100()
{
	State s = makeState(HW3);
	TEST_ASSERT_FALSE(executeOffsetCmd("offset:101", s));
}

/** @brief Offset command is rejected on LEGACY variant */
void test_offset_rejects_without_feature()
{
	State s = makeState(LEGACY);
	TEST_ASSERT_FALSE(executeOffsetCmd("offset:50", s));
}

/** @brief HW4 offset accepts values within its valid range */
void test_hw4_offset_sets_value()
{
	State s = makeState(HW4);
	TEST_ASSERT_TRUE(executeOffsetCmd("offset:16", s));
	TEST_ASSERT_EQUAL(16, s.speedOffset);
}

/** @brief "offset:off" on HW4 zeroes the speed offset */
void test_hw4_offset_off_disables()
{
	State s = makeState(HW4);
	s.speedOffset = 22;
	TEST_ASSERT_TRUE(executeOffsetCmd("offset:off", s));
	TEST_ASSERT_EQUAL(0, s.speedOffset);
}

/** @brief HW4 offset rejects values outside its valid range */
void test_hw4_offset_rejects_out_of_range()
{
	State s = makeState(HW4);
	TEST_ASSERT_FALSE(executeOffsetCmd("offset:64", s));
}

/** @brief HW3 variant still uses the HW3 offset path (accepts value 10) */
void test_hw4_offset_rejects_non_hw4()
{
	State s = makeState(HW3);
	TEST_ASSERT_TRUE(executeOffsetCmd("offset:10", s));
	TEST_ASSERT_EQUAL(10, s.speedOffset);
}

/** @brief "isa-chime:on" enables ISA chime suppression on HW4 */
void test_isa_on_hw4()
{
	State s = makeState(HW4);
	TEST_ASSERT_TRUE(executeIsaChimeCmd("isa-chime:on", s));
	TEST_ASSERT_TRUE(s.isaChimeSuppress);
	TEST_ASSERT_EQUAL(1, filterCount);
}

/** @brief "isa-chime:off" disables ISA chime suppression on HW4 */
void test_isa_off_hw4()
{
	State s = makeState(HW4);
	s.isaChimeSuppress = true;
	TEST_ASSERT_TRUE(executeIsaChimeCmd("isa-chime:off", s));
	TEST_ASSERT_FALSE(s.isaChimeSuppress);
	TEST_ASSERT_EQUAL(1, filterCount);
}

/** @brief ISA chime command is rejected on LEGACY variant */
void test_isa_rejects_without_feature()
{
	State s = makeState(LEGACY);
	TEST_ASSERT_FALSE(executeIsaChimeCmd("isa-chime:on", s));
}

/** @brief "summon:fwd" starts forward summon with 30 remaining ticks */
void test_summon_forward()
{
	State s = makeState(HW4);
	s.hasCtrl = true;
	s.summonInject = true;
	TEST_ASSERT_TRUE(executeSummonCmd("summon:fwd", s));
	TEST_ASSERT_EQUAL(SUMMON_FORWARD, s.summonDirection);
	TEST_ASSERT_EQUAL(SUMMON_START, s.summonMode);
	TEST_ASSERT_EQUAL(30, s.summonRemaining);
}

/** @brief "summon:rev" starts reverse summon */
void test_summon_reverse()
{
	State s = makeState(HW4);
	s.hasCtrl = true;
	s.summonInject = true;
	TEST_ASSERT_TRUE(executeSummonCmd("summon:rev", s));
	TEST_ASSERT_EQUAL(SUMMON_REVERSE, s.summonDirection);
}

/** @brief "summon:stop" halts summon and zeroes remaining count */
void test_summon_stop()
{
	State s = makeState(HW4);
	s.hasCtrl = true;
	s.summonRemaining = 10;
	TEST_ASSERT_TRUE(executeSummonCmd("summon:stop", s));
	TEST_ASSERT_EQUAL(SUMMON_STOP, s.summonMode);
	TEST_ASSERT_EQUAL(0, s.summonRemaining);
}

/** @brief Summon requires hasCtrl to be true */
void test_summon_requires_ctrl()
{
	State s = makeState(HW4);
	s.hasCtrl = false;
	TEST_ASSERT_FALSE(executeSummonCmd("summon:fwd", s));
}

/** @brief Summon is rejected on LEGACY variant */
void test_summon_requires_feature()
{
	State s = makeState(LEGACY);
	s.hasCtrl = true;
	s.summonInject = true;
	TEST_ASSERT_FALSE(executeSummonCmd("summon:fwd", s));
}

/** @brief "variant:hw3" switches variant and reapplies filters */
void test_variant_hw3()
{
	State s = makeState(HW4);
	TEST_ASSERT_TRUE(executeVariantCmd("variant:hw3", s));
	TEST_ASSERT_EQUAL(HW3, s.variant);
	TEST_ASSERT_EQUAL(1, filterCount);
}

/** @brief "variant:legacy" switches to LEGACY variant */
void test_variant_legacy()
{
	State s = makeState(HW4);
	TEST_ASSERT_TRUE(executeVariantCmd("variant:legacy", s));
	TEST_ASSERT_EQUAL(LEGACY, s.variant);
}

/** @brief Invalid variant names are rejected */
void test_variant_rejects_invalid()
{
	State s = makeState(HW4);
	TEST_ASSERT_FALSE(executeVariantCmd("variant:unknown", s));
}

/** @brief "variant:auto" enables auto-detection without changing current variant */
void test_variant_auto_enables_auto_detect()
{
	State s = makeState(HW4);
	s.variantAutoDetect = false;
	TEST_ASSERT_TRUE(executeVariantCmd("variant:auto", s));
	TEST_ASSERT_TRUE(s.variantAutoDetect);
	TEST_ASSERT_EQUAL(HW4, s.variant);
	TEST_ASSERT_EQUAL(1, filterCount);
}

/** @brief Manual variant selection disables auto-detection */
void test_variant_manual_disables_auto_detect()
{
	State s = makeState(HW4);
	s.variantAutoDetect = true;
	TEST_ASSERT_TRUE(executeVariantCmd("variant:hw3", s));
	TEST_ASSERT_EQUAL(HW3, s.variant);
	TEST_ASSERT_FALSE(s.variantAutoDetect);
}

/** @brief "nag:mode:legacy" sets legacy nag mode and saves */
void test_nag_mode_legacy()
{
	State s = makeState(HW4);
	TEST_ASSERT_TRUE(executeNagCmd("nag:mode:legacy", s));
	TEST_ASSERT_EQUAL(NAG_MODE_LEGACY, s.nagMode);
	TEST_ASSERT_EQUAL(1, saveCount);
}

/** @brief "nag:mode:off" clears any active nag mode */
void test_nag_mode_off_clears()
{
	State s = makeState(HW4);
	s.nagMode = NAG_MODE_ORGANIC;
	TEST_ASSERT_TRUE(executeNagCmd("nag:mode:off", s));
	TEST_ASSERT_EQUAL(NAG_MODE_OFF, s.nagMode);
}

/** @brief "nag:mode:safe" sets safe nag mode */
void test_nag_mode_safe()
{
	State s = makeState(HW4);
	TEST_ASSERT_TRUE(executeNagCmd("nag:mode:safe", s));
	TEST_ASSERT_EQUAL(NAG_MODE_SAFE, s.nagMode);
}

/** @brief "nag:mode:full" enables all nag suppression bits */
void test_nag_mode_full_sets_all_bits()
{
	State s = makeState(HW4);
	TEST_ASSERT_TRUE(executeNagCmd("nag:mode:full", s));
	TEST_ASSERT_EQUAL(NAG_MODE_FULL, s.nagMode);
	TEST_ASSERT_TRUE(nagModeUsesBit19(s.nagMode));
	TEST_ASSERT_TRUE(nagModeUsesEpasEcho(s.nagMode));
}

/** @brief Invalid nag mode names are rejected */
void test_nag_mode_rejects_invalid_name()
{
	State s = makeState(HW4);
	TEST_ASSERT_FALSE(executeNagCmd("nag:mode:turbo", s));
}

/** @brief Legacy nag command forms (nag:on, nag:off, nag:killer:*) are no longer accepted */
void test_nag_legacy_commands_rejected()
{
	State s = makeState(HW4);
	TEST_ASSERT_FALSE(executeNagCmd("nag:on", s));
	TEST_ASSERT_FALSE(executeNagCmd("nag:off", s));
	TEST_ASSERT_FALSE(executeNagCmd("nag:killer:on", s));
	TEST_ASSERT_FALSE(executeNagCmd("nag:killer:mode:safe", s));
}

/** @brief Ban shield toggle triggers a settings save */
void test_banshield_saves()
{
	State s = makeState();
	executeBanShieldCmd("banshield:on", s);
	TEST_ASSERT_EQUAL(1, saveCount);
}

/** @brief "summon-inject:on" enables summon injection */
void test_summon_inject_on()
{
	State s = makeState(HW4);
	TEST_ASSERT_TRUE(executeSummonInjectCmd("summon-inject:on", s));
	TEST_ASSERT_TRUE(s.summonInject);
}

/** @brief "summon-inject:off" disables summon injection */
void test_summon_inject_off()
{
	State s = makeState(HW4);
	s.summonInject = true;
	TEST_ASSERT_TRUE(executeSummonInjectCmd("summon-inject:off", s));
	TEST_ASSERT_FALSE(s.summonInject);
}

/** @brief Summon inject command is rejected on LEGACY variant */
void test_summon_inject_rejects_without_feature()
{
	State s = makeState(LEGACY);
	TEST_ASSERT_FALSE(executeSummonInjectCmd("summon-inject:on", s));
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_profile_sets_value_and_pins);
	RUN_TEST(test_profile_auto_unpins);
	RUN_TEST(test_profile_lock_pins_current_profile);
	RUN_TEST(test_profile_unlock_unpins_current_profile);
	RUN_TEST(test_profile_rejects_negative);
	RUN_TEST(test_profile_rejects_above_4);
	RUN_TEST(test_profile_accepts_0_through_4);
	RUN_TEST(test_profile_sp_alias);
	RUN_TEST(test_profile_ignores_unrelated);

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

	RUN_TEST(test_nag_mode_bit19);

	RUN_TEST(test_banshield_on);
	RUN_TEST(test_banshield_off);
	RUN_TEST(test_banshield_resets_threat_on_enable);
	RUN_TEST(test_banshield_saves);

	RUN_TEST(test_nag_mode_off);

	RUN_TEST(test_offset_sets_value);
	RUN_TEST(test_offset_auto);
	RUN_TEST(test_offset_rejects_above_100);
	RUN_TEST(test_offset_rejects_without_feature);
	RUN_TEST(test_hw4_offset_sets_value);
	RUN_TEST(test_hw4_offset_off_disables);
	RUN_TEST(test_hw4_offset_rejects_out_of_range);
	RUN_TEST(test_hw4_offset_rejects_non_hw4);

	RUN_TEST(test_isa_on_hw4);
	RUN_TEST(test_isa_off_hw4);
	RUN_TEST(test_isa_rejects_without_feature);

	RUN_TEST(test_summon_forward);
	RUN_TEST(test_summon_reverse);
	RUN_TEST(test_summon_stop);
	RUN_TEST(test_summon_requires_ctrl);
	RUN_TEST(test_summon_requires_feature);

	RUN_TEST(test_variant_hw3);
	RUN_TEST(test_variant_legacy);
	RUN_TEST(test_variant_rejects_invalid);
	RUN_TEST(test_variant_auto_enables_auto_detect);
	RUN_TEST(test_variant_manual_disables_auto_detect);

	RUN_TEST(test_nag_mode_legacy);
	RUN_TEST(test_nag_mode_off_clears);
	RUN_TEST(test_nag_mode_safe);
	RUN_TEST(test_nag_mode_full_sets_all_bits);
	RUN_TEST(test_nag_mode_rejects_invalid_name);
	RUN_TEST(test_nag_legacy_commands_rejected);

	RUN_TEST(test_summon_inject_on);
	RUN_TEST(test_summon_inject_off);
	RUN_TEST(test_summon_inject_rejects_without_feature);

	return UNITY_END();
}
