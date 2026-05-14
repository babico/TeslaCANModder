/** @file firmware/test/test_native_nag_organic/test_nag_organic.cpp
 *  @brief Unit tests for organic nag suppression mode
 *  @author Tesla CAN Mod Contributors
 *  @license GPL-3.0
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


static State makeStateOrganic(uint8_t handsOnState = 2, uint8_t apState = 4, float steerAngle = 0.0f)
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


void test_organic_gate_rejects_when_mode_not_organic()
{
	State s = makeStateOrganic();
	s.nagMode = NAG_MODE_NATURAL;
	TEST_ASSERT_FALSE(nagOrganicShouldEcho(s));
}

void test_organic_gate_rejects_when_not_enabled()
{
	State s = makeStateOrganic();
	s.nagMode = NAG_MODE_OFF;
	TEST_ASSERT_FALSE(nagOrganicShouldEcho(s));
}

void test_organic_gate_rejects_when_das_not_seen()
{
	State s = makeStateOrganic();
	s.dasSeen = false;
	TEST_ASSERT_FALSE(nagOrganicShouldEcho(s));
}

void test_organic_gate_rejects_when_ap_state_out_of_range()
{
	for (uint8_t apState : {(uint8_t)0, (uint8_t)1, (uint8_t)2, (uint8_t)7, (uint8_t)15})
	{
		State s = makeStateOrganic(2, apState);
		TEST_ASSERT_FALSE(nagOrganicShouldEcho(s));
	}
}

void test_organic_gate_accepts_when_ap_state_in_range()
{
	for (uint8_t apState : {(uint8_t)3, (uint8_t)4, (uint8_t)5, (uint8_t)6})
	{
		State s = makeStateOrganic(2, apState);
		TEST_ASSERT_TRUE(nagOrganicShouldEcho(s));
	}
}

void test_organic_gate_rejects_idle_hands_on_states()
{
	for (uint8_t st : {(uint8_t)0, (uint8_t)8, (uint8_t)15})
	{
		State s = makeStateOrganic(st, 4);
		TEST_ASSERT_FALSE(nagOrganicShouldEcho(s));
	}
}

void test_organic_gate_rejects_when_driver_bypass_and_real_hands_on()
{
	State s = makeStateOrganic();
	s.nagOrganicDriverBypass = true;
	s.nagOrganicRealHandsOn = 1;
	TEST_ASSERT_FALSE(nagOrganicShouldEcho(s));
}

void test_organic_gate_accepts_when_driver_bypass_but_real_hands_zero()
{
	State s = makeStateOrganic();
	s.nagOrganicDriverBypass = true;
	s.nagOrganicRealHandsOn = 0;
	TEST_ASSERT_TRUE(nagOrganicShouldEcho(s));
}


void test_on_state_change_captures_state1_grace()
{
	State s = makeStateOrganic(1);
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
	State s = makeStateOrganic(2);
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
	State s = makeStateOrganic(2);
	s.nagOrganicPrevState = 0;
	nagOrganicOnStateChange(s, 5000UL);
	TEST_ASSERT_EQUAL(5000UL, s.nagOrg2EnterMs);
}

void test_on_state_change_starts_strong_pause_group()
{
	State s = makeStateOrganic(3);
	s.nagOrganicPrevState = 0;
	nagOrganicOnStateChange(s, 7000UL);
	TEST_ASSERT_EQUAL(7000UL, s.nagOrgStrongEnterMs);
}

void test_on_state_change_does_not_reset_strong_within_group()
{
	State s = makeStateOrganic(4);
	s.nagOrganicPrevState = 3;
	s.nagOrgStrongEnterMs = 7000UL;
	nagOrganicOnStateChange(s, 9000UL);
	TEST_ASSERT_EQUAL(7000UL, s.nagOrgStrongEnterMs);
}

void test_on_state_change_clears_strong_when_leaving_group()
{
	State s = makeStateOrganic(2);
	s.nagOrganicPrevState = 5;
	s.nagOrgStrongEnterMs = 7000UL;
	nagOrganicOnStateChange(s, 9000UL);
	TEST_ASSERT_EQUAL(0UL, s.nagOrgStrongEnterMs);
}


void test_tick_returns_false_when_gate_closed()
{
	State s = makeStateOrganic(0);
	TEST_ASSERT_FALSE(nagOrganicTick(s, 1000UL));
}

void test_tick_state1_grace_hold_returns_true_within_500ms()
{
	State s = makeStateOrganic(1);
	s.nagOrg1EnterMs = 1000UL;
	s.nagOrg1HoldRaw = 2150;
	s.nagOrg1HoldLevel = 1;
	TEST_ASSERT_TRUE(nagOrganicTick(s, 1400UL));
	TEST_ASSERT_EQUAL_INT16(2150, s.nagOrgLastRaw);
	TEST_ASSERT_EQUAL(1, s.nagOrgLastLevel);
}

void test_tick_state1_returns_false_after_500ms_grace()
{
	State s = makeStateOrganic(1);
	s.nagOrg1EnterMs = 1000UL;
	TEST_ASSERT_FALSE(nagOrganicTick(s, 1500UL));
}

void test_tick_state2_returns_false_during_2s_pause()
{
	State s = makeStateOrganic(2);
	s.nagOrg2EnterMs = 1000UL;
	TEST_ASSERT_FALSE(nagOrganicTick(s, 1999UL));
}

void test_tick_state2_returns_true_after_2s_with_walk_in_range()
{
	State s = makeStateOrganic(2, 4, -5.0f);
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
	State s = makeStateOrganic(2, 4, +10.0f);
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
	State s = makeStateOrganic(3);
	s.nagOrgStrongEnterMs = 5000UL;
	TEST_ASSERT_FALSE(nagOrganicTick(s, 5500UL));
}

void test_tick_state3_returns_true_after_pause_with_ramp()
{
	State s = makeStateOrganic(3, 4, -5.0f);
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
		State s = makeStateOrganic(state);
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
	State s = makeStateOrganic(2, 4, -5.0f);
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


void test_modify_increments_counter_byte6_lower_nibble()
{
	State s = makeStateOrganic();
	s.nagOrgLastRaw = 2200;
	s.nagOrgLastLevel = 2;
	Frame f = makeFrame(CAN_ID_EPAS_TORQUE);
	f.data[6] = 0xA5;
	nagOrganicApply(f, s);
	TEST_ASSERT_EQUAL_HEX8(0xA6, f.data[6]);
}

void test_modify_counter_wraps_at_16()
{
	State s = makeStateOrganic();
	Frame f = makeFrame(CAN_ID_EPAS_TORQUE);
	f.data[6] = 0x8F;
	nagOrganicApply(f, s);
	TEST_ASSERT_EQUAL_HEX8(0x80, f.data[6]);
}

void test_modify_encodes_torque_motorola_19_12()
{
	State s = makeStateOrganic();
	s.nagOrgLastRaw = 2248;
	Frame f = makeFrame(CAN_ID_EPAS_TORQUE);
	f.data[2] = 0xF0;
	nagOrganicApply(f, s);
	TEST_ASSERT_EQUAL_HEX8(0xF8, f.data[2]);
	TEST_ASSERT_EQUAL_HEX8(0xC8, f.data[3]);
}

void test_modify_writes_hands_on_level_bits_7_6()
{
	State s = makeStateOrganic();
	s.nagOrgLastLevel = 2;
	Frame f = makeFrame(CAN_ID_EPAS_TORQUE);
	f.data[4] = 0xCF;
	nagOrganicApply(f, s);
	TEST_ASSERT_EQUAL_HEX8(0x8F, f.data[4]);
}

void test_modify_recomputes_checksum()
{
	State s = makeStateOrganic();
	s.nagOrgLastRaw = 2048;
	s.nagOrgLastLevel = 1;
	Frame f = makeFrame(CAN_ID_EPAS_TORQUE);
	f.data[7] = 0xFF;
	nagOrganicApply(f, s);
	uint8_t expected = nagChecksum(f.data);
	TEST_ASSERT_EQUAL_HEX8(expected, f.data[7]);
}

void test_modify_ignores_short_frame()
{
	State s = makeStateOrganic();
	Frame f = makeFrame(CAN_ID_EPAS_TORQUE, 4);
	uint8_t before[8];
	memcpy(before, f.data, 8);
	nagOrganicApply(f, s);
	TEST_ASSERT_EQUAL_MEMORY(before, f.data, 8);
}


void test_cmd_mode_organic_selects_organic()
{
	State s = {};
	s.nagMode = NAG_MODE_LEGACY;
	TEST_ASSERT_TRUE(executeNagCmd("nag:mode:organic", s));
	TEST_ASSERT_EQUAL(NAG_MODE_ORGANIC, s.nagMode);
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
	NagMode m;
	TEST_ASSERT_TRUE(parseNagMode("organic", m));
	TEST_ASSERT_EQUAL(NAG_MODE_ORGANIC, m);
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_organic_gate_rejects_when_mode_not_organic);
	RUN_TEST(test_organic_gate_rejects_when_not_enabled);
	RUN_TEST(test_organic_gate_rejects_when_das_not_seen);
	RUN_TEST(test_organic_gate_rejects_when_ap_state_out_of_range);
	RUN_TEST(test_organic_gate_accepts_when_ap_state_in_range);
	RUN_TEST(test_organic_gate_rejects_idle_hands_on_states);
	RUN_TEST(test_organic_gate_rejects_when_driver_bypass_and_real_hands_on);
	RUN_TEST(test_organic_gate_accepts_when_driver_bypass_but_real_hands_zero);

	RUN_TEST(test_on_state_change_captures_state1_grace);
	RUN_TEST(test_on_state_change_clears_state1_memory_when_leaving_state1);
	RUN_TEST(test_on_state_change_starts_state2_pause);
	RUN_TEST(test_on_state_change_starts_strong_pause_group);
	RUN_TEST(test_on_state_change_does_not_reset_strong_within_group);
	RUN_TEST(test_on_state_change_clears_strong_when_leaving_group);

	RUN_TEST(test_tick_returns_false_when_gate_closed);
	RUN_TEST(test_tick_state1_grace_hold_returns_true_within_500ms);
	RUN_TEST(test_tick_state1_returns_false_after_500ms_grace);
	RUN_TEST(test_tick_state2_returns_false_during_2s_pause);
	RUN_TEST(test_tick_state2_returns_true_after_2s_with_walk_in_range);
	RUN_TEST(test_tick_state2_walk_flips_direction_with_steering_sign);
	RUN_TEST(test_tick_state3_returns_false_during_1s_pause);
	RUN_TEST(test_tick_state3_returns_true_after_pause_with_ramp);
	RUN_TEST(test_tick_strong_group_treats_345_equivalently);

	RUN_TEST(test_level_from_raw_threshold_2nm);
	RUN_TEST(test_level_from_raw_threshold_1nm);
	RUN_TEST(test_level_from_raw_below_1nm);

	RUN_TEST(test_grip_excursion_fires_within_225_frames);

	RUN_TEST(test_modify_increments_counter_byte6_lower_nibble);
	RUN_TEST(test_modify_counter_wraps_at_16);
	RUN_TEST(test_modify_encodes_torque_motorola_19_12);
	RUN_TEST(test_modify_writes_hands_on_level_bits_7_6);
	RUN_TEST(test_modify_recomputes_checksum);
	RUN_TEST(test_modify_ignores_short_frame);

	RUN_TEST(test_cmd_mode_organic_selects_organic);
	RUN_TEST(test_cmd_bypass_on_sets_flag);
	RUN_TEST(test_cmd_bypass_off_clears_flag);
	RUN_TEST(test_cmd_mode_name_round_trip);

	return UNITY_END();
}

