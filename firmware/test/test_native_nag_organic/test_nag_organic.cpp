/** @file firmware/test/test_native_nag_organic/test_nag_organic.cpp
 *  @brief Unit tests for the unified nag echo pipeline (organic mode + feifan +
 *         legacy/natural byte-equivalence regression).
 *  @author Tesla CAN Mod Contributors
 *  @license GPL-3.0
 *
 *  The pipeline is:
 *    nagEchoShouldEcho(state, mode) -> bool
 *    nagEchoCompute(state, mode, nowMs) -> NagTorque
 *    nagEchoApply(frame, torque) -> void
 *
 *  These tests drive the pipeline end-to-end and assert byte-level equivalence
 *  with the pre-refactor behavior for every mode, plus feifan-specific
 *  coverage (handsOnLevel left at 0, walk bounds, byte 6 counter, checksum).
 */

#include <unity.h>
#include <cstring>
#include <cstdlib>
#include <initializer_list>

class __FlashStringHelper;
#define F(s) (reinterpret_cast<const __FlashStringHelper *>(s))

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_BLE 0
#define BOARD_ENABLE_WIFI 0

#include "core/types.h"
#include "vehicle/can/ids.h"

static int saveCount = 0;
void saveSettings(const State &)
{
	saveCount++;
}
void resetHandlerLogFlags() {}
void applyFilters(State &) {}

#include "core/util/parse.h"
#include "feature/fsd/nag.h"
#include "support/helpers.h"


static State makeState(uint8_t handsOnState = 2, uint8_t apState = 4, float steerAngle = 0.0f)
{
	State s = {};
	s.nagMode = NAG_MODE_ORGANIC;

	s.dasApState = apState;
	s.dasHandsOnState = handsOnState;
	s.dasSeen = true;
	s.steeringAngle = steerAngle;
	s.nagOrganicPrevState = 0xFF;
	return s;
}

static int16_t absRaw(int16_t raw)
{
	return raw > 2048 ? raw - 2048 : 2048 - raw;
}

void setUp()
{
	saveCount = 0;
}
void tearDown() {}


// ── Gate tests (drive nagEchoShouldEcho with NAG_MODE_ORGANIC) ──────────────

void test_organic_gate_is_parameterized_by_mode_argument()
{
	// The unified gate is parameterized by the mode argument, NOT by s.nagMode.
	// Querying with NAG_MODE_ORGANIC on a state whose s.nagMode is set to a
	// different mode still applies the organic rules and accepts when they pass.
	State s = makeState();        // default state: organic conditions pass
	s.nagMode = NAG_MODE_NATURAL; // noise: caller-chosen mode is the argument, not s.nagMode
	TEST_ASSERT_TRUE(nagEchoShouldEcho(s, NAG_MODE_ORGANIC));
	// Querying with the natural mode on the same state also passes the natural gate.
	TEST_ASSERT_TRUE(nagEchoShouldEcho(s, NAG_MODE_NATURAL));
}

void test_organic_gate_rejects_when_not_enabled()
{
	State s = makeState();
	TEST_ASSERT_FALSE(nagEchoShouldEcho(s, NAG_MODE_OFF));
}

void test_organic_gate_rejects_when_das_not_seen()
{
	State s = makeState();
	s.dasSeen = false;
	TEST_ASSERT_FALSE(nagEchoShouldEcho(s, s.nagMode));
}

void test_organic_gate_rejects_when_ap_state_out_of_range()
{
	for (uint8_t apState : {(uint8_t)0, (uint8_t)1, (uint8_t)2, (uint8_t)7, (uint8_t)15})
	{
		State s = makeState(2, apState);
		TEST_ASSERT_FALSE(nagEchoShouldEcho(s, s.nagMode));
	}
}

void test_organic_gate_accepts_when_ap_state_in_range()
{
	for (uint8_t apState : {(uint8_t)3, (uint8_t)4, (uint8_t)5, (uint8_t)6})
	{
		State s = makeState(2, apState);
		TEST_ASSERT_TRUE(nagEchoShouldEcho(s, s.nagMode));
	}
}

void test_organic_gate_rejects_idle_hands_on_states()
{
	for (uint8_t st : {(uint8_t)0, (uint8_t)8, (uint8_t)15})
	{
		State s = makeState(st, 4);
		TEST_ASSERT_FALSE(nagEchoShouldEcho(s, s.nagMode));
	}
}

void test_organic_gate_rejects_when_driver_bypass_and_real_hands_on()
{
	State s = makeState();
	s.nagOrganicDriverBypass = true;
	s.nagOrganicRealHandsOn = 1;
	TEST_ASSERT_FALSE(nagEchoShouldEcho(s, s.nagMode));
}

void test_organic_gate_accepts_when_driver_bypass_but_real_hands_zero()
{
	State s = makeState();
	s.nagOrganicDriverBypass = true;
	s.nagOrganicRealHandsOn = 0;
	TEST_ASSERT_TRUE(nagEchoShouldEcho(s, s.nagMode));
}


// ── Organic state-machine helpers (preserved; tested via tick) ──────────────

void test_on_state_change_captures_state1_grace()
{
	State s = makeState(1);
	s.nagOrganicPrevState = 2;
	s.nagOrgLastRaw = 2200;
	s.nagOrgLastLevel = 2;
	nagOrganicOnStateChange(s, 10000UL);
	TEST_ASSERT_EQUAL(10000UL, s.nagOrg1EnterMs);
	TEST_ASSERT_EQUAL_INT16(2200, s.nagOrg1HoldRaw);
	TEST_ASSERT_EQUAL(2, s.nagOrg1HoldLevel);
}

void test_on_state_change_clears_state1_memory_when_leaving_state1()
{
	State s = makeState(2);
	s.nagOrganicPrevState = 1;
	s.nagOrg1EnterMs = 5000;
	s.nagOrg1HoldRaw = 2200;
	s.nagOrg1HoldLevel = 2;
	nagOrganicOnStateChange(s, 10000UL);
	TEST_ASSERT_EQUAL(0UL, s.nagOrg1EnterMs);
	TEST_ASSERT_EQUAL(0, s.nagOrg1HoldLevel);
}

void test_on_state_change_starts_state2_pause()
{
	State s = makeState(2);
	s.nagOrganicPrevState = 0;
	nagOrganicOnStateChange(s, 5000UL);
	TEST_ASSERT_EQUAL(5000UL, s.nagOrg2EnterMs);
}

void test_on_state_change_starts_strong_pause_group()
{
	State s = makeState(3);
	s.nagOrganicPrevState = 0;
	nagOrganicOnStateChange(s, 7000UL);
	TEST_ASSERT_EQUAL(7000UL, s.nagOrgStrongEnterMs);
}

void test_on_state_change_does_not_reset_strong_within_group()
{
	State s = makeState(4);
	s.nagOrganicPrevState = 3;
	s.nagOrgStrongEnterMs = 7000UL;
	nagOrganicOnStateChange(s, 9000UL);
	TEST_ASSERT_EQUAL(7000UL, s.nagOrgStrongEnterMs);
}

void test_on_state_change_clears_strong_when_leaving_group()
{
	State s = makeState(2);
	s.nagOrganicPrevState = 5;
	s.nagOrgStrongEnterMs = 7000UL;
	nagOrganicOnStateChange(s, 9000UL);
	TEST_ASSERT_EQUAL(0UL, s.nagOrgStrongEnterMs);
}


void test_tick_returns_false_when_gate_closed()
{
	State s = makeState(0);
	TEST_ASSERT_FALSE(nagOrganicTick(s, 1000UL));
}

void test_tick_state1_grace_hold_returns_true_within_500ms()
{
	State s = makeState(1);
	s.nagOrg1EnterMs = 1000UL;
	s.nagOrg1HoldRaw = 2150;
	s.nagOrg1HoldLevel = 1;
	TEST_ASSERT_TRUE(nagOrganicTick(s, 1400UL));
	TEST_ASSERT_EQUAL_INT16(2150, s.nagOrgLastRaw);
	TEST_ASSERT_EQUAL(1, s.nagOrgLastLevel);
}

void test_tick_state1_returns_false_after_500ms_grace()
{
	State s = makeState(1);
	s.nagOrg1EnterMs = 1000UL;
	TEST_ASSERT_FALSE(nagOrganicTick(s, 1500UL));
}

void test_tick_state2_returns_false_during_2s_pause()
{
	State s = makeState(2);
	s.nagOrg2EnterMs = 1000UL;
	TEST_ASSERT_FALSE(nagOrganicTick(s, 1999UL));
}

void test_tick_state2_returns_true_after_2s_with_walk_in_range()
{
	State s = makeState(2, 4, -5.0f);
	s.nagOrg2EnterMs = 1000UL;
	for (int i = 0; i < 20; i++)
	{
		bool sent = nagOrganicTick(s, 3100UL + i);
		TEST_ASSERT_TRUE(sent);
	}
	TEST_ASSERT_TRUE_MESSAGE(s.nagOrgLastRaw >= 1848 && s.nagOrgLastRaw <= 2378,
							 "walk raw escaped plausible range");
}

void test_tick_state2_walk_flips_direction_with_steering_sign()
{
	State s = makeState(2, 4, +10.0f);
	s.nagOrg2EnterMs = 1000UL;
	bool anyBelowCenter = false;
	for (int i = 0; i < 40; i++)
	{
		nagOrganicTick(s, 3100UL + i);
		if (s.nagOrgLastRaw < 2048)
			anyBelowCenter = true;
	}
	TEST_ASSERT_TRUE_MESSAGE(anyBelowCenter, "positive steering should produce negative torque");
}

void test_tick_state3_returns_false_during_1s_pause()
{
	State s = makeState(3);
	s.nagOrgStrongEnterMs = 5000UL;
	TEST_ASSERT_FALSE(nagOrganicTick(s, 5500UL));
}

void test_tick_state3_returns_true_after_pause_with_ramp()
{
	State s = makeState(3, 4, -5.0f);
	s.nagOrgStrongEnterMs = 5000UL;
	TEST_ASSERT_TRUE(nagOrganicTick(s, 6000UL));
	TEST_ASSERT_INT16_WITHIN(20, 2048, s.nagOrgLastRaw);
	TEST_ASSERT_TRUE(nagOrganicTick(s, 6250UL));
	int16_t midRaw = s.nagOrgLastRaw;
	TEST_ASSERT_TRUE_MESSAGE(midRaw > 2048, "expected positive direction");
	TEST_ASSERT_INT16_WITHIN(40, 2153, midRaw);
	TEST_ASSERT_TRUE(nagOrganicTick(s, 6600UL));
	int16_t holdRaw = s.nagOrgLastRaw;
	TEST_ASSERT_TRUE_MESSAGE(holdRaw >= 2258, "expected hold at ~2.1 Nm");
}

void test_tick_strong_group_treats_345_equivalently()
{
	for (uint8_t state : {(uint8_t)3, (uint8_t)4, (uint8_t)5})
	{
		State s = makeState(state);
		s.nagOrgStrongEnterMs = 1000UL;
		TEST_ASSERT_TRUE(nagOrganicTick(s, 2600UL));
		TEST_ASSERT_TRUE(absRaw(s.nagOrgLastRaw) >= 180);
	}
}


void test_level_from_raw_threshold_2nm()
{
	TEST_ASSERT_EQUAL(2, nagOrgLevelFromRaw(2248));
	TEST_ASSERT_EQUAL(2, nagOrgLevelFromRaw(1848));
}

void test_level_from_raw_threshold_1nm()
{
	TEST_ASSERT_EQUAL(1, nagOrgLevelFromRaw(2148));
	TEST_ASSERT_EQUAL(1, nagOrgLevelFromRaw(1948));
}

void test_level_from_raw_below_1nm()
{
	TEST_ASSERT_EQUAL(0, nagOrgLevelFromRaw(2100));
	TEST_ASSERT_EQUAL(0, nagOrgLevelFromRaw(1996));
	TEST_ASSERT_EQUAL(0, nagOrgLevelFromRaw(2048));
}


void test_grip_excursion_fires_within_225_frames()
{
	State s = makeState(2, 4, -5.0f);
	s.nagOrg2EnterMs = 1000UL;
	s.nagOrgFramesUntilExc = 1;
	s.nagOrg2WalkRaw = 2150;
	int excursionObserved = 0;
	for (int i = 0; i < 10; i++)
	{
		nagOrganicTick(s, 3100UL + i);
		if (absRaw(s.nagOrgLastRaw) >= 280)
			excursionObserved++;
	}
	TEST_ASSERT_TRUE_MESSAGE(excursionObserved >= 3, "expected 3-5 excursion frames");
}


// ── Unified nagEchoApply: organic byte-equivalence regression ───────────────

void test_apply_organic_increments_counter_byte6_lower_nibble()
{
	State s = makeState();
	s.nagOrgLastRaw = 2200;
	s.nagOrgLastLevel = 2;
	Frame f = makeFrame(CAN_ID_EPAS_TORQUE);
	f.data[6] = 0xA5;
	NagTorque t{true, s.nagOrgLastRaw, s.nagOrgLastLevel, 6, true};
	nagEchoApply(f, t);
	TEST_ASSERT_EQUAL_HEX8(0xA6, f.data[6]);
}

void test_apply_organic_counter_wraps_at_16()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_EPAS_TORQUE);
	f.data[6] = 0x8F;
	NagTorque t{true, s.nagOrgLastRaw, s.nagOrgLastLevel, 6, true};
	nagEchoApply(f, t);
	TEST_ASSERT_EQUAL_HEX8(0x80, f.data[6]);
}

void test_apply_organic_encodes_torque_motorola_19_12()
{
	State s = makeState();
	s.nagOrgLastRaw = 2248;
	Frame f = makeFrame(CAN_ID_EPAS_TORQUE);
	f.data[2] = 0xF0;
	NagTorque t{true, s.nagOrgLastRaw, s.nagOrgLastLevel, 6, true};
	nagEchoApply(f, t);
	TEST_ASSERT_EQUAL_HEX8(0xF8, f.data[2]);
	TEST_ASSERT_EQUAL_HEX8(0xC8, f.data[3]);
}

void test_apply_organic_writes_hands_on_level_bits_7_6()
{
	State s = makeState();
	s.nagOrgLastLevel = 2;
	Frame f = makeFrame(CAN_ID_EPAS_TORQUE);
	f.data[4] = 0xCF;
	NagTorque t{true, s.nagOrgLastRaw, s.nagOrgLastLevel, 6, true};
	nagEchoApply(f, t);
	TEST_ASSERT_EQUAL_HEX8(0x8F, f.data[4]);
}

void test_apply_organic_recomputes_checksum()
{
	State s = makeState();
	s.nagOrgLastRaw = 2048;
	s.nagOrgLastLevel = 1;
	Frame f = makeFrame(CAN_ID_EPAS_TORQUE);
	f.data[7] = 0xFF;
	NagTorque t{true, s.nagOrgLastRaw, s.nagOrgLastLevel, 6, true};
	nagEchoApply(f, t);
	uint8_t expected = nagChecksum(f.data);
	TEST_ASSERT_EQUAL_HEX8(expected, f.data[7]);
}

void test_apply_ignores_short_frame()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_EPAS_TORQUE, 4);
	uint8_t before[8];
	memcpy(before, f.data, 8);
	NagTorque t{true, s.nagOrgLastRaw, s.nagOrgLastLevel, 6, true};
	nagEchoApply(f, t);
	TEST_ASSERT_EQUAL_MEMORY(before, f.data, 8);
}


// ── Legacy / Safe / Natural byte-equivalence through the pipeline ───────────

void test_apply_legacy_zero_torque_byte1_counter()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_EPAS_TORQUE);
	f.data[1] = 0x37;
	f.data[4] = 0x00; // handsOnLevel will be forced to 1
	NagTorque t = nagEchoCompute(s, NAG_MODE_LEGACY, 0);
	TEST_ASSERT_TRUE(t.valid);
	TEST_ASSERT_EQUAL(1, t.handsOnLevel);
	TEST_ASSERT_EQUAL(1, t.counterByte);
	TEST_ASSERT_FALSE(t.preserveData2High);
	nagEchoApply(f, t);
	// Counter: byte 1 low nibble +1, high nibble preserved.
	TEST_ASSERT_EQUAL_HEX8(0x38, f.data[1]);
	// Torque hard zero.
	TEST_ASSERT_EQUAL_HEX8(0x00, f.data[2]);
	TEST_ASSERT_EQUAL_HEX8(0x00, f.data[3]);
	// handsOnLevel = 1 forced into byte 4 bits[7:6].
	TEST_ASSERT_EQUAL_HEX8(0x40, f.data[4]);
	// Checksum.
	TEST_ASSERT_EQUAL_HEX8(nagChecksum(f.data), f.data[7]);
}

void test_apply_natural_encodes_signed_torque_byte1_counter()
{
	State s = makeState();
	s.dasHandsOnState = 4;
	// First call: interval is ready (naturalNagLastMs=0 -> immediately ready),
	// re-rolls interval, computes a torque in [0.08, 0.18] Nm.
	NagTorque t = nagEchoCompute(s, NAG_MODE_NATURAL, 1000UL);
	TEST_ASSERT_TRUE(t.valid);
	TEST_ASSERT_EQUAL(1, t.handsOnLevel);
	TEST_ASSERT_EQUAL(1, t.counterByte);
	TEST_ASSERT_FALSE(t.preserveData2High);
	Frame f = makeFrame(CAN_ID_EPAS_TORQUE);
	f.data[1] = 0x20;
	f.data[4] = 0x00;
	nagEchoApply(f, t);
	// Counter: byte 1 low nibble +1 -> 0x21.
	TEST_ASSERT_EQUAL_HEX8(0x21, f.data[1]);
	// Torque raw is signed 16-bit in data[2:3], should be in [8, 18] (0.08..0.18 Nm * 100).
	int16_t raw = (int16_t)(((uint16_t)f.data[2] << 8) | f.data[3]);
	TEST_ASSERT_TRUE_MESSAGE(raw >= 8 && raw <= 18, "natural torque raw out of expected range");
	// handsOnLevel forced to 1.
	TEST_ASSERT_EQUAL_HEX8(0x40, f.data[4]);
	TEST_ASSERT_EQUAL_HEX8(nagChecksum(f.data), f.data[7]);
}


// ── Feifan mode (NAG_MODE_FEIFAN) ───────────────────────────────────────────

void test_feifan_gate_rejects_when_das_not_seen()
{
	State s = makeState();
	s.nagMode = NAG_MODE_FEIFAN;
	s.dasSeen = false;
	TEST_ASSERT_FALSE(nagEchoShouldEcho(s, s.nagMode));
}

void test_feifan_gate_rejects_when_das_not_requesting()
{
	State s = makeState();
	s.nagMode = NAG_MODE_FEIFAN;
	// dasHandsOnState = 2 IS in the request set, so this should pass.
	// Use idle states to ensure rejection.
	for (uint8_t st : {(uint8_t)0, (uint8_t)1, (uint8_t)8, (uint8_t)11, (uint8_t)15})
	{
		State ss = makeState(st, 4);
		ss.nagMode = NAG_MODE_FEIFAN;
		TEST_ASSERT_FALSE_MESSAGE(nagEchoShouldEcho(ss, ss.nagMode),
								 "feifan gate accepted an idle hands-on state");
	}
}

void test_feifan_gate_accepts_when_das_requesting()
{
	for (uint8_t st : {(uint8_t)2, (uint8_t)3, (uint8_t)4, (uint8_t)5, (uint8_t)6, (uint8_t)7, (uint8_t)9, (uint8_t)10})
	{
		State s = makeState(st, 4);
		s.nagMode = NAG_MODE_FEIFAN;
		TEST_ASSERT_TRUE_MESSAGE(nagEchoShouldEcho(s, s.nagMode),
								"feifan gate rejected a requesting hands-on state");
	}
}

void test_feifan_compute_returns_hands_on_zero_byte6_counter()
{
	State s = makeState();
	s.nagMode = NAG_MODE_FEIFAN;
	NagTorque t = nagEchoCompute(s, NAG_MODE_FEIFAN, 1000UL);
	TEST_ASSERT_TRUE(t.valid);
	TEST_ASSERT_EQUAL(0, t.handsOnLevel);   // CRITICAL: feifan leaves handsOnLevel at 0
	TEST_ASSERT_EQUAL(6, t.counterByte);    // counter on byte 6 (organic layout)
	TEST_ASSERT_TRUE(t.preserveData2High);  // 12-bit torque slot, preserve data[2] high nibble
	TEST_ASSERT_TRUE_MESSAGE(t.raw >= 2043 && t.raw <= 2050,
							 "feifan walk raw outside captured [-5,+2] range");
}

void test_feifan_compute_walk_stays_bounded_over_many_steps()
{
	State s = makeState();
	s.nagMode = NAG_MODE_FEIFAN;
	for (int i = 0; i < 500; i++)
	{
		NagTorque t = nagEchoCompute(s, NAG_MODE_FEIFAN, 2000UL + i);
		TEST_ASSERT_TRUE(t.valid);
		TEST_ASSERT_TRUE_MESSAGE(t.raw >= 2043 && t.raw <= 2050,
								 "feifan walk raw escaped [2043,2050]");
	}
}

void test_feifan_apply_leaves_hands_on_level_at_zero()
{
	State s = makeState();
	s.nagMode = NAG_MODE_FEIFAN;
	NagTorque t = nagEchoCompute(s, NAG_MODE_FEIFAN, 1000UL);
	TEST_ASSERT_EQUAL(0, t.handsOnLevel);
	Frame f = makeFrame(CAN_ID_EPAS_TORQUE);
	f.data[4] = 0xCF; // any pre-existing high nibble
	nagEchoApply(f, t);
	// handsOnLevel bits [7:6] must remain 0.
	TEST_ASSERT_EQUAL_HEX8(0x0F, f.data[4] & 0x3F); // bits[7:6] cleared, lower 6 preserved
	TEST_ASSERT_EQUAL_HEX8(0x0F, f.data[4]);
}

void test_feifan_apply_increments_byte6_counter()
{
	State s = makeState();
	s.nagMode = NAG_MODE_FEIFAN;
	NagTorque t = nagEchoCompute(s, NAG_MODE_FEIFAN, 1000UL);
	Frame f = makeFrame(CAN_ID_EPAS_TORQUE);
	f.data[6] = 0xA5;
	nagEchoApply(f, t);
	TEST_ASSERT_EQUAL_HEX8(0xA6, f.data[6]);
}

void test_feifan_apply_preserves_data2_high_nibble_status()
{
	State s = makeState();
	s.nagMode = NAG_MODE_FEIFAN;
	NagTorque t = nagEchoCompute(s, NAG_MODE_FEIFAN, 1000UL);
	Frame f = makeFrame(CAN_ID_EPAS_TORQUE);
	f.data[2] = 0x12; // 0x12 = status nibble + zero torque low nibble (feifan capture)
	nagEchoApply(f, t);
	// High nibble (status 0x1) must be preserved.
	TEST_ASSERT_EQUAL_HEX8(0x10, f.data[2] & 0xF0);
}

void test_feifan_apply_recomputes_checksum()
{
	State s = makeState();
	s.nagMode = NAG_MODE_FEIFAN;
	NagTorque t = nagEchoCompute(s, NAG_MODE_FEIFAN, 1000UL);
	Frame f = makeFrame(CAN_ID_EPAS_TORQUE);
	f.data[7] = 0xFF;
	nagEchoApply(f, t);
	TEST_ASSERT_EQUAL_HEX8(nagChecksum(f.data), f.data[7]);
}


// ── Command dispatch ─────────────────────────────────────────────────────────

void test_cmd_mode_organic_selects_organic()
{
	State s = {};
	s.nagMode = NAG_MODE_LEGACY;
	TEST_ASSERT_TRUE(executeNagCmd("nag:mode:organic", s));
	TEST_ASSERT_EQUAL(NAG_MODE_ORGANIC, s.nagMode);
	TEST_ASSERT_EQUAL(1, saveCount);
}

void test_cmd_mode_feifan_selects_feifan()
{
	State s = {};
	s.nagMode = NAG_MODE_OFF;
	TEST_ASSERT_TRUE(executeNagCmd("nag:mode:feifan", s));
	TEST_ASSERT_EQUAL(NAG_MODE_FEIFAN, s.nagMode);
	TEST_ASSERT_EQUAL(1, saveCount);
}

void test_cmd_bypass_on_sets_flag()
{
	State s = {};
	TEST_ASSERT_TRUE(executeNagCmd("nag:bypass:on", s));
	TEST_ASSERT_TRUE(s.nagOrganicDriverBypass);
}

void test_cmd_bypass_off_clears_flag()
{
	State s = {};
	s.nagOrganicDriverBypass = true;
	TEST_ASSERT_TRUE(executeNagCmd("nag:bypass:off", s));
	TEST_ASSERT_FALSE(s.nagOrganicDriverBypass);
}

void test_cmd_mode_name_round_trip()
{
	TEST_ASSERT_EQUAL_STRING("organic", nagModeName(NAG_MODE_ORGANIC));
	TEST_ASSERT_EQUAL_STRING("feifan", nagModeName(NAG_MODE_FEIFAN));
	NagMode m;
	TEST_ASSERT_TRUE(parseNagMode("organic", m));
	TEST_ASSERT_EQUAL(NAG_MODE_ORGANIC, m);
	TEST_ASSERT_TRUE(parseNagMode("feifan", m));
	TEST_ASSERT_EQUAL(NAG_MODE_FEIFAN, m);
	TEST_ASSERT_TRUE(nagModeUsesEpasEcho(NAG_MODE_FEIFAN));
	TEST_ASSERT_FALSE(nagModeUsesBit19(NAG_MODE_FEIFAN));
}


int main()
{
	UNITY_BEGIN();

	// Gate
	RUN_TEST(test_organic_gate_is_parameterized_by_mode_argument);
	RUN_TEST(test_organic_gate_rejects_when_not_enabled);
	RUN_TEST(test_organic_gate_rejects_when_das_not_seen);
	RUN_TEST(test_organic_gate_rejects_when_ap_state_out_of_range);
	RUN_TEST(test_organic_gate_accepts_when_ap_state_in_range);
	RUN_TEST(test_organic_gate_rejects_idle_hands_on_states);
	RUN_TEST(test_organic_gate_rejects_when_driver_bypass_and_real_hands_on);
	RUN_TEST(test_organic_gate_accepts_when_driver_bypass_but_real_hands_zero);

	// OnStateChange
	RUN_TEST(test_on_state_change_captures_state1_grace);
	RUN_TEST(test_on_state_change_clears_state1_memory_when_leaving_state1);
	RUN_TEST(test_on_state_change_starts_state2_pause);
	RUN_TEST(test_on_state_change_starts_strong_pause_group);
	RUN_TEST(test_on_state_change_does_not_reset_strong_within_group);
	RUN_TEST(test_on_state_change_clears_strong_when_leaving_group);

	// Tick
	RUN_TEST(test_tick_returns_false_when_gate_closed);
	RUN_TEST(test_tick_state1_grace_hold_returns_true_within_500ms);
	RUN_TEST(test_tick_state1_returns_false_after_500ms_grace);
	RUN_TEST(test_tick_state2_returns_false_during_2s_pause);
	RUN_TEST(test_tick_state2_returns_true_after_2s_with_walk_in_range);
	RUN_TEST(test_tick_state2_walk_flips_direction_with_steering_sign);
	RUN_TEST(test_tick_state3_returns_false_during_1s_pause);
	RUN_TEST(test_tick_state3_returns_true_after_pause_with_ramp);
	RUN_TEST(test_tick_strong_group_treats_345_equivalently);

	// Level
	RUN_TEST(test_level_from_raw_threshold_2nm);
	RUN_TEST(test_level_from_raw_threshold_1nm);
	RUN_TEST(test_level_from_raw_below_1nm);

	// Grip excursion
	RUN_TEST(test_grip_excursion_fires_within_225_frames);

	// Apply (organic byte-equivalence through the pipeline)
	RUN_TEST(test_apply_organic_increments_counter_byte6_lower_nibble);
	RUN_TEST(test_apply_organic_counter_wraps_at_16);
	RUN_TEST(test_apply_organic_encodes_torque_motorola_19_12);
	RUN_TEST(test_apply_organic_writes_hands_on_level_bits_7_6);
	RUN_TEST(test_apply_organic_recomputes_checksum);
	RUN_TEST(test_apply_ignores_short_frame);

	// Legacy / Natural byte-equivalence through the pipeline
	RUN_TEST(test_apply_legacy_zero_torque_byte1_counter);
	RUN_TEST(test_apply_natural_encodes_signed_torque_byte1_counter);

	// Feifan
	RUN_TEST(test_feifan_gate_rejects_when_das_not_seen);
	RUN_TEST(test_feifan_gate_rejects_when_das_not_requesting);
	RUN_TEST(test_feifan_gate_accepts_when_das_requesting);
	RUN_TEST(test_feifan_compute_returns_hands_on_zero_byte6_counter);
	RUN_TEST(test_feifan_compute_walk_stays_bounded_over_many_steps);
	RUN_TEST(test_feifan_apply_leaves_hands_on_level_at_zero);
	RUN_TEST(test_feifan_apply_increments_byte6_counter);
	RUN_TEST(test_feifan_apply_preserves_data2_high_nibble_status);
	RUN_TEST(test_feifan_apply_recomputes_checksum);

	// Commands
	RUN_TEST(test_cmd_mode_organic_selects_organic);
	RUN_TEST(test_cmd_mode_feifan_selects_feifan);
	RUN_TEST(test_cmd_bypass_on_sets_flag);
	RUN_TEST(test_cmd_bypass_off_clears_flag);
	RUN_TEST(test_cmd_mode_name_round_trip);

	return UNITY_END();
}
