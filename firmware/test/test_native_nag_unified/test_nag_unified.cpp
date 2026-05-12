/** @file firmware/test/test_native_nag_unified/test_nag_unified.cpp
 *  @brief Unit tests for unified nag mode selection
 *  @author Tesla CAN Mod Contributors
 *  @license GPL-3.0
 */

#include <unity.h>
#include <cstring>
#include <cstdio>

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
#include "feature/nag.h"

static State makeState()
{
	return State{};
}

void setUp()
{
	saveCount = 0;
}
void tearDown() {}


void test_mode_names_are_stable()
{
	TEST_ASSERT_EQUAL_STRING("off", nagModeName(NAG_MODE_OFF));
	TEST_ASSERT_EQUAL_STRING("bit19", nagModeName(NAG_MODE_BIT19));
	TEST_ASSERT_EQUAL_STRING("legacy", nagModeName(NAG_MODE_LEGACY));
	TEST_ASSERT_EQUAL_STRING("safe", nagModeName(NAG_MODE_SAFE));
	TEST_ASSERT_EQUAL_STRING("natural", nagModeName(NAG_MODE_NATURAL));
	TEST_ASSERT_EQUAL_STRING("organic", nagModeName(NAG_MODE_ORGANIC));
	TEST_ASSERT_EQUAL_STRING("full", nagModeName(NAG_MODE_FULL));
}

void test_parse_accepts_every_valid_name()
{
	NagMode m;
	TEST_ASSERT_TRUE(parseNagMode("off", m));
	TEST_ASSERT_EQUAL(NAG_MODE_OFF, m);
	TEST_ASSERT_TRUE(parseNagMode("bit19", m));
	TEST_ASSERT_EQUAL(NAG_MODE_BIT19, m);
	TEST_ASSERT_TRUE(parseNagMode("legacy", m));
	TEST_ASSERT_EQUAL(NAG_MODE_LEGACY, m);
	TEST_ASSERT_TRUE(parseNagMode("safe", m));
	TEST_ASSERT_EQUAL(NAG_MODE_SAFE, m);
	TEST_ASSERT_TRUE(parseNagMode("natural", m));
	TEST_ASSERT_EQUAL(NAG_MODE_NATURAL, m);
	TEST_ASSERT_TRUE(parseNagMode("organic", m));
	TEST_ASSERT_EQUAL(NAG_MODE_ORGANIC, m);
	TEST_ASSERT_TRUE(parseNagMode("full", m));
	TEST_ASSERT_EQUAL(NAG_MODE_FULL, m);
}

void test_parse_rejects_unknown_and_case()
{
	NagMode m;
	TEST_ASSERT_FALSE(parseNagMode("turbo", m));
	TEST_ASSERT_FALSE(parseNagMode("", m));
	TEST_ASSERT_FALSE(parseNagMode("LEGACY", m));
	TEST_ASSERT_FALSE(parseNagMode("nag", m));
	TEST_ASSERT_FALSE(parseNagMode("on", m));
}


void test_uses_bit19_only_for_bit19_and_full()
{
	TEST_ASSERT_FALSE(nagModeUsesBit19(NAG_MODE_OFF));
	TEST_ASSERT_TRUE(nagModeUsesBit19(NAG_MODE_BIT19));
	TEST_ASSERT_FALSE(nagModeUsesBit19(NAG_MODE_LEGACY));
	TEST_ASSERT_FALSE(nagModeUsesBit19(NAG_MODE_SAFE));
	TEST_ASSERT_FALSE(nagModeUsesBit19(NAG_MODE_NATURAL));
	TEST_ASSERT_FALSE(nagModeUsesBit19(NAG_MODE_ORGANIC));
	TEST_ASSERT_TRUE(nagModeUsesBit19(NAG_MODE_FULL));
}

void test_uses_epas_echo_for_all_echo_modes()
{
	TEST_ASSERT_FALSE(nagModeUsesEpasEcho(NAG_MODE_OFF));
	TEST_ASSERT_FALSE(nagModeUsesEpasEcho(NAG_MODE_BIT19));
	TEST_ASSERT_TRUE(nagModeUsesEpasEcho(NAG_MODE_LEGACY));
	TEST_ASSERT_TRUE(nagModeUsesEpasEcho(NAG_MODE_SAFE));
	TEST_ASSERT_TRUE(nagModeUsesEpasEcho(NAG_MODE_NATURAL));
	TEST_ASSERT_TRUE(nagModeUsesEpasEcho(NAG_MODE_ORGANIC));
	TEST_ASSERT_TRUE(nagModeUsesEpasEcho(NAG_MODE_FULL));
}

void test_active_everything_but_off()
{
	TEST_ASSERT_FALSE(nagModeActive(NAG_MODE_OFF));
	TEST_ASSERT_TRUE(nagModeActive(NAG_MODE_BIT19));
	TEST_ASSERT_TRUE(nagModeActive(NAG_MODE_ORGANIC));
	TEST_ASSERT_TRUE(nagModeActive(NAG_MODE_FULL));
}

void test_needs_das_gate_for_safe_natural_organic_full()
{
	TEST_ASSERT_FALSE(nagModeNeedsDasGate(NAG_MODE_OFF));
	TEST_ASSERT_FALSE(nagModeNeedsDasGate(NAG_MODE_BIT19));
	TEST_ASSERT_FALSE(nagModeNeedsDasGate(NAG_MODE_LEGACY));
	TEST_ASSERT_TRUE(nagModeNeedsDasGate(NAG_MODE_SAFE));
	TEST_ASSERT_TRUE(nagModeNeedsDasGate(NAG_MODE_NATURAL));
	TEST_ASSERT_TRUE(nagModeNeedsDasGate(NAG_MODE_ORGANIC));
	TEST_ASSERT_TRUE(nagModeNeedsDasGate(NAG_MODE_FULL));
}


void test_cmd_mode_off()
{
	State s = makeState();
	s.nagMode = NAG_MODE_ORGANIC;
	TEST_ASSERT_TRUE(executeNagCmd("nag:mode:off", s));
	TEST_ASSERT_EQUAL(NAG_MODE_OFF, s.nagMode);
	TEST_ASSERT_EQUAL(1, saveCount);
}

void test_cmd_mode_every_valid_name_sets_mode()
{
	const char *names[] = {"off", "bit19", "legacy", "safe", "natural", "organic", "full"};
	const NagMode modes[] = {NAG_MODE_OFF,  NAG_MODE_BIT19,   NAG_MODE_LEGACY, NAG_MODE_SAFE,
							 NAG_MODE_NATURAL, NAG_MODE_ORGANIC, NAG_MODE_FULL};
	for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++)
	{
		State s = makeState();
		char buf[32];
		snprintf(buf, sizeof(buf), "nag:mode:%s", names[i]);
		TEST_ASSERT_TRUE(executeNagCmd(buf, s));
		TEST_ASSERT_EQUAL(modes[i], s.nagMode);
	}
}

void test_cmd_mode_rejects_unknown_name()
{
	State s = makeState();
	s.nagMode = NAG_MODE_NATURAL;
	TEST_ASSERT_FALSE(executeNagCmd("nag:mode:turbo", s));
	TEST_ASSERT_EQUAL(NAG_MODE_NATURAL, s.nagMode);
	TEST_ASSERT_EQUAL(0, saveCount);
}

void test_cmd_bypass_on()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeNagCmd("nag:bypass:on", s));
	TEST_ASSERT_TRUE(s.nagOrganicDriverBypass);
	TEST_ASSERT_EQUAL(1, saveCount);
}

void test_cmd_bypass_off()
{
	State s = makeState();
	s.nagOrganicDriverBypass = true;
	TEST_ASSERT_TRUE(executeNagCmd("nag:bypass:off", s));
	TEST_ASSERT_FALSE(s.nagOrganicDriverBypass);
}

void test_cmd_rejects_non_nag_prefix()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeNagCmd("fsd:on", s));
	TEST_ASSERT_FALSE(executeNagCmd("nagkiller:on", s));
	TEST_ASSERT_EQUAL(0, saveCount);
}

void test_cmd_rejects_legacy_nag_on()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeNagCmd("nag:on", s));
	TEST_ASSERT_FALSE(executeNagCmd("nag:off", s));
	TEST_ASSERT_FALSE(executeNagCmd("nag:killer:on", s));
	TEST_ASSERT_FALSE(executeNagCmd("nag:killer:off", s));
	TEST_ASSERT_FALSE(executeNagCmd("nag:killer:mode:natural", s));
	TEST_ASSERT_EQUAL(0, saveCount);
}


void test_default_mode_is_off()
{
	State s = makeState();
	TEST_ASSERT_EQUAL(NAG_MODE_OFF, s.nagMode);
	TEST_ASSERT_FALSE(s.nagOrganicDriverBypass);
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_mode_names_are_stable);
	RUN_TEST(test_parse_accepts_every_valid_name);
	RUN_TEST(test_parse_rejects_unknown_and_case);

	RUN_TEST(test_uses_bit19_only_for_bit19_and_full);
	RUN_TEST(test_uses_epas_echo_for_all_echo_modes);
	RUN_TEST(test_active_everything_but_off);
	RUN_TEST(test_needs_das_gate_for_safe_natural_organic_full);

	RUN_TEST(test_cmd_mode_off);
	RUN_TEST(test_cmd_mode_every_valid_name_sets_mode);
	RUN_TEST(test_cmd_mode_rejects_unknown_name);
	RUN_TEST(test_cmd_bypass_on);
	RUN_TEST(test_cmd_bypass_off);
	RUN_TEST(test_cmd_rejects_non_nag_prefix);
	RUN_TEST(test_cmd_rejects_legacy_nag_on);

	RUN_TEST(test_default_mode_is_off);

	return UNITY_END();
}

