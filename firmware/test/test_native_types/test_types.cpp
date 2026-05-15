/**
 * @file firmware/test/test_native_types/test_types.cpp
 * @brief Unit tests for core types and helpers in types.h
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <unity.h>
#include <cstring>

#include "core/types.h"
#include "support/helpers.h"

void setUp() {}
void tearDown() {}

/* ── setBit ──────────────────────────────────────────────────────────────── */

void test_set_bit_sets_bit_0()
{
	Frame f = makeFrame(0x100, 8);
	memset(f.data, 0, 8);
	setBit(f, 0, true);
	TEST_ASSERT_EQUAL_HEX8(0x01, f.data[0]);
}

void test_set_bit_sets_bit_7()
{
	Frame f = makeFrame(0x100, 8);
	memset(f.data, 0, 8);
	setBit(f, 7, true);
	TEST_ASSERT_EQUAL_HEX8(0x80, f.data[0]);
}

void test_set_bit_sets_bit_19()
{
	Frame f = makeFrame(0x100, 8);
	memset(f.data, 0, 8);
	setBit(f, 19, true);
	TEST_ASSERT_EQUAL_HEX8(0x08, f.data[2]);
}

void test_set_bit_clears_bit()
{
	Frame f = makeFrame(0x100, 8);
	memset(f.data, 0xFF, 8);
	setBit(f, 3, false);
	TEST_ASSERT_EQUAL_HEX8(0xF7, f.data[0]);
}

void test_set_bit_does_not_modify_other_bits()
{
	Frame f = makeFrame(0x100, 8);
	f.data[0] = 0xAC;
	setBit(f, 1, true);
	TEST_ASSERT_EQUAL_HEX8(0xAE, f.data[0]);
}

void test_set_bit_ignores_bit_beyond_dlc()
{
	Frame f = makeFrame(0x100, 4);
	memset(f.data, 0, 8);
	setBit(f, 40, true);
	TEST_ASSERT_EQUAL_HEX8(0x00, f.data[5]);
}

void test_set_bit_exact_at_dlc_boundary()
{
	Frame f = makeFrame(0x100, 3);
	memset(f.data, 0, 8);
	setBit(f, 23, true);
	TEST_ASSERT_EQUAL_HEX8(0x80, f.data[2]);
}

void test_set_bit_just_past_dlc_boundary()
{
	Frame f = makeFrame(0x100, 3);
	memset(f.data, 0, 8);
	setBit(f, 24, true);
	TEST_ASSERT_EQUAL_HEX8(0x00, f.data[3]);
}

/* ── readMuxID ───────────────────────────────────────────────────────────── */

void test_read_mux_id_zero()
{
	Frame f = makeFrame(0x100, 8);
	f.data[0] = 0x00;
	TEST_ASSERT_EQUAL_UINT8(0, readMuxID(f));
}

void test_read_mux_id_seven()
{
	Frame f = makeFrame(0x100, 8);
	f.data[0] = 0x07;
	TEST_ASSERT_EQUAL_UINT8(7, readMuxID(f));
}

void test_read_mux_id_masks_upper_bits()
{
	Frame f = makeFrame(0x100, 8);
	f.data[0] = 0xFF;
	TEST_ASSERT_EQUAL_UINT8(7, readMuxID(f));
}

void test_read_mux_id_returns_zero_when_dlc_zero()
{
	Frame f = makeFrame(0x100, 0);
	f.data[0] = 0x05;
	TEST_ASSERT_EQUAL_UINT8(0, readMuxID(f));
}

void test_read_mux_id_various_values()
{
	Frame f = makeFrame(0x100, 8);
	for (uint8_t i = 0; i < 8; i++)
	{
		f.data[0] = i;
		TEST_ASSERT_EQUAL_UINT8(i, readMuxID(f));
	}
}

/* ── variantName ─────────────────────────────────────────────────────────── */

void test_variant_name_hw4()
{
	TEST_ASSERT_EQUAL_STRING("hw4", variantName(HW4));
}

void test_variant_name_hw3()
{
	TEST_ASSERT_EQUAL_STRING("hw3", variantName(HW3));
}

void test_variant_name_legacy()
{
	TEST_ASSERT_EQUAL_STRING("legacy", variantName(LEGACY));
}

/* ── parseVariant ────────────────────────────────────────────────────────── */

void test_parse_variant_hw4()
{
	Variant v;
	TEST_ASSERT_TRUE(parseVariant("hw4", v));
	TEST_ASSERT_EQUAL(HW4, v);
}

void test_parse_variant_hw3()
{
	Variant v;
	TEST_ASSERT_TRUE(parseVariant("hw3", v));
	TEST_ASSERT_EQUAL(HW3, v);
}

void test_parse_variant_legacy()
{
	Variant v;
	TEST_ASSERT_TRUE(parseVariant("legacy", v));
	TEST_ASSERT_EQUAL(LEGACY, v);
}

void test_parse_variant_rejects_unknown()
{
	Variant v = HW4;
	TEST_ASSERT_FALSE(parseVariant("unknown", v));
}

void test_parse_variant_rejects_empty()
{
	Variant v = HW4;
	TEST_ASSERT_FALSE(parseVariant("", v));
}

/* ── getFeatures ─────────────────────────────────────────────────────────── */

void test_features_hw4_all_true()
{
	Features f = getFeatures(HW4);
	TEST_ASSERT_TRUE(f.fsd);
	TEST_ASSERT_TRUE(f.fsdForce);
	TEST_ASSERT_TRUE(f.offset);
	TEST_ASSERT_TRUE(f.profile);
	TEST_ASSERT_TRUE(f.nag);
	TEST_ASSERT_TRUE(f.isaChime);
	TEST_ASSERT_TRUE(f.summon);
}

void test_features_hw3_no_isa_chime()
{
	Features f = getFeatures(HW3);
	TEST_ASSERT_TRUE(f.fsd);
	TEST_ASSERT_TRUE(f.offset);
	TEST_ASSERT_FALSE(f.isaChime);
	TEST_ASSERT_TRUE(f.summon);
}

void test_features_legacy_no_offset_no_isa_no_summon()
{
	Features f = getFeatures(LEGACY);
	TEST_ASSERT_TRUE(f.fsd);
	TEST_ASSERT_FALSE(f.offset);
	TEST_ASSERT_FALSE(f.isaChime);
	TEST_ASSERT_FALSE(f.summon);
}

/* ── NagMode predicates ──────────────────────────────────────────────────── */

void test_nag_mode_name_all_values()
{
	TEST_ASSERT_EQUAL_STRING("off", nagModeName(NAG_MODE_OFF));
	TEST_ASSERT_EQUAL_STRING("bit19", nagModeName(NAG_MODE_BIT19));
	TEST_ASSERT_EQUAL_STRING("legacy", nagModeName(NAG_MODE_LEGACY));
	TEST_ASSERT_EQUAL_STRING("safe", nagModeName(NAG_MODE_SAFE));
	TEST_ASSERT_EQUAL_STRING("natural", nagModeName(NAG_MODE_NATURAL));
	TEST_ASSERT_EQUAL_STRING("organic", nagModeName(NAG_MODE_ORGANIC));
	TEST_ASSERT_EQUAL_STRING("full", nagModeName(NAG_MODE_FULL));
}

void test_parse_nag_mode_all_values()
{
	NagMode m;
	TEST_ASSERT_TRUE(parseNagMode("off", m));
	TEST_ASSERT_EQUAL(NAG_MODE_OFF, m);
	TEST_ASSERT_TRUE(parseNagMode("bit19", m));
	TEST_ASSERT_EQUAL(NAG_MODE_BIT19, m);
	TEST_ASSERT_TRUE(parseNagMode("full", m));
	TEST_ASSERT_EQUAL(NAG_MODE_FULL, m);
}

void test_parse_nag_mode_rejects_unknown()
{
	NagMode m = NAG_MODE_OFF;
	TEST_ASSERT_FALSE(parseNagMode("turbo", m));
}

void test_nag_mode_uses_epas_echo()
{
	TEST_ASSERT_FALSE(nagModeUsesEpasEcho(NAG_MODE_OFF));
	TEST_ASSERT_FALSE(nagModeUsesEpasEcho(NAG_MODE_BIT19));
	TEST_ASSERT_TRUE(nagModeUsesEpasEcho(NAG_MODE_LEGACY));
	TEST_ASSERT_TRUE(nagModeUsesEpasEcho(NAG_MODE_SAFE));
	TEST_ASSERT_TRUE(nagModeUsesEpasEcho(NAG_MODE_NATURAL));
	TEST_ASSERT_TRUE(nagModeUsesEpasEcho(NAG_MODE_ORGANIC));
	TEST_ASSERT_TRUE(nagModeUsesEpasEcho(NAG_MODE_FULL));
}

void test_nag_mode_uses_bit19()
{
	TEST_ASSERT_TRUE(nagModeUsesBit19(NAG_MODE_BIT19));
	TEST_ASSERT_TRUE(nagModeUsesBit19(NAG_MODE_FULL));
	TEST_ASSERT_FALSE(nagModeUsesBit19(NAG_MODE_OFF));
	TEST_ASSERT_FALSE(nagModeUsesBit19(NAG_MODE_ORGANIC));
}

void test_nag_mode_active()
{
	TEST_ASSERT_FALSE(nagModeActive(NAG_MODE_OFF));
	for (int i = 1; i <= 6; i++)
	{
		TEST_ASSERT_TRUE(nagModeActive((NagMode)i));
	}
}

void test_nag_mode_needs_das_gate()
{
	TEST_ASSERT_FALSE(nagModeNeedsDasGate(NAG_MODE_OFF));
	TEST_ASSERT_FALSE(nagModeNeedsDasGate(NAG_MODE_BIT19));
	TEST_ASSERT_FALSE(nagModeNeedsDasGate(NAG_MODE_LEGACY));
	TEST_ASSERT_TRUE(nagModeNeedsDasGate(NAG_MODE_SAFE));
	TEST_ASSERT_TRUE(nagModeNeedsDasGate(NAG_MODE_NATURAL));
	TEST_ASSERT_TRUE(nagModeNeedsDasGate(NAG_MODE_ORGANIC));
	TEST_ASSERT_TRUE(nagModeNeedsDasGate(NAG_MODE_FULL));
}

/* ── State::apGateOpen ───────────────────────────────────────────────────── */

void test_ap_gate_open_when_disabled()
{
	State s = {};
	s.apInjectionGateEnabled = false;
	TEST_ASSERT_TRUE(s.apGateOpen());
}

void test_ap_gate_open_when_ap_active()
{
	State s = {};
	s.apInjectionGateEnabled = true;
	s.apGateApActive = true;
	TEST_ASSERT_TRUE(s.apGateOpen());
}

void test_ap_gate_open_when_parked()
{
	State s = {};
	s.apInjectionGateEnabled = true;
	s.apGateParked = true;
	TEST_ASSERT_TRUE(s.apGateOpen());
}

void test_ap_gate_open_when_summoning()
{
	State s = {};
	s.apInjectionGateEnabled = true;
	s.apGateSummoning = true;
	TEST_ASSERT_TRUE(s.apGateOpen());
}

void test_ap_gate_closed_when_all_conditions_false()
{
	State s = {};
	s.apInjectionGateEnabled = true;
	s.apGateApActive = false;
	s.apGateParked = false;
	s.apGateSummoning = false;
	TEST_ASSERT_FALSE(s.apGateOpen());
}

/* ── State::features ─────────────────────────────────────────────────────── */

void test_state_features_delegates_to_get_features()
{
	State s = {};
	s.variant = HW3;
	Features f = s.features();
	TEST_ASSERT_FALSE(f.isaChime);
}

/* ── CanBusStat defaults ─────────────────────────────────────────────────── */

void test_can_bus_stat_defaults()
{
	CanBusStat stat;
	TEST_ASSERT_EQUAL_UINT32(0, stat.frames);
	TEST_ASSERT_EQUAL_UINT16(0, stat.hz);
	TEST_ASSERT_EQUAL_UINT16(0xFFFF, stat.hzMin);
	TEST_ASSERT_EQUAL_UINT16(0, stat.hzMax);
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_set_bit_sets_bit_0);
	RUN_TEST(test_set_bit_sets_bit_7);
	RUN_TEST(test_set_bit_sets_bit_19);
	RUN_TEST(test_set_bit_clears_bit);
	RUN_TEST(test_set_bit_does_not_modify_other_bits);
	RUN_TEST(test_set_bit_ignores_bit_beyond_dlc);
	RUN_TEST(test_set_bit_exact_at_dlc_boundary);
	RUN_TEST(test_set_bit_just_past_dlc_boundary);

	RUN_TEST(test_read_mux_id_zero);
	RUN_TEST(test_read_mux_id_seven);
	RUN_TEST(test_read_mux_id_masks_upper_bits);
	RUN_TEST(test_read_mux_id_returns_zero_when_dlc_zero);
	RUN_TEST(test_read_mux_id_various_values);

	RUN_TEST(test_variant_name_hw4);
	RUN_TEST(test_variant_name_hw3);
	RUN_TEST(test_variant_name_legacy);

	RUN_TEST(test_parse_variant_hw4);
	RUN_TEST(test_parse_variant_hw3);
	RUN_TEST(test_parse_variant_legacy);
	RUN_TEST(test_parse_variant_rejects_unknown);
	RUN_TEST(test_parse_variant_rejects_empty);

	RUN_TEST(test_features_hw4_all_true);
	RUN_TEST(test_features_hw3_no_isa_chime);
	RUN_TEST(test_features_legacy_no_offset_no_isa_no_summon);

	RUN_TEST(test_nag_mode_name_all_values);
	RUN_TEST(test_parse_nag_mode_all_values);
	RUN_TEST(test_parse_nag_mode_rejects_unknown);
	RUN_TEST(test_nag_mode_uses_epas_echo);
	RUN_TEST(test_nag_mode_uses_bit19);
	RUN_TEST(test_nag_mode_active);
	RUN_TEST(test_nag_mode_needs_das_gate);

	RUN_TEST(test_ap_gate_open_when_disabled);
	RUN_TEST(test_ap_gate_open_when_ap_active);
	RUN_TEST(test_ap_gate_open_when_parked);
	RUN_TEST(test_ap_gate_open_when_summoning);
	RUN_TEST(test_ap_gate_closed_when_all_conditions_false);

	RUN_TEST(test_state_features_delegates_to_get_features);

	RUN_TEST(test_can_bus_stat_defaults);

	return UNITY_END();
}
