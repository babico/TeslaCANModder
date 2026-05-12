/** @file firmware/test/test_native_persist/test_persist.cpp
 *  @brief Unit tests for NVS settings persistence
 *  @author Tesla CAN Mod Contributors
 *  @license GPL-3.0
 */

#include <unity.h>
#include <cstring>

#include "../support/fake_preferences.h"
#define Preferences_h

#include "core/types.h"

#define NVS_NAMESPACE "tcm"
#define NVS_KEY_MAGIC "magic"
#define NVS_KEY_VERSION "ver"
#define NVS_SETTINGS_MAGIC 0xCA
#define NVS_SETTINGS_VERSION 0x0D

static Preferences prefs;

static bool loadSettings(State &s)
{
	prefs.begin(NVS_NAMESPACE, true);
	uint8_t magic = prefs.getUChar(NVS_KEY_MAGIC, 0);
	uint8_t ver = prefs.getUChar(NVS_KEY_VERSION, 0);
	if (magic != NVS_SETTINGS_MAGIC || ver != NVS_SETTINGS_VERSION)
	{
		prefs.end();
		return false;
	}
	s.variant = (Variant)prefs.getUChar("variant", 0);
	s.fsdEnabled = prefs.getUChar("fsd", 0);
	s.fsdForceEnabled = prefs.getUChar("ffsd", 0);
	s.speedProfile = prefs.getUChar("sp", 1);
	s.profileOverride = prefs.getUChar("spPin", 0);
	s.speedOffset = prefs.getUChar("offset", 0);
	s.offsetOverride = prefs.getUChar("offPin", 0);
	s.isaChimeSuppress = prefs.getUChar("isa", 0);
	s.summonInject = prefs.getUChar("sumInj", 0);
	s.nagMode = (NagMode)prefs.getUChar("nagMode", (uint8_t)NAG_MODE_OFF);
	s.nagOrganicDriverBypass = prefs.getUChar("nagOrgDB", 0);
	s.preconditionEnabled = prefs.getUChar("precond", 0);
	s.trackModeEnabled = prefs.getUChar("track", 0);
	s.variantAutoDetect = prefs.getUChar("vAuto", 1);
	s.apInjectionGateEnabled = prefs.getUChar("apGate", 0);
	s.banShieldEnabled = prefs.getUChar("banS", 0);
	s.canClockReqMHz = prefs.getUChar("clkMHz", 0);
	s.driveModeOverride = prefs.getUChar("drvM", 0);
	s.eceR79Bypass = prefs.getUChar("eceR79", 0);
	s.lhdEnabled = prefs.getUChar("lhd", 0);
	s.enhancedAutopilot = prefs.getUChar("eap", 0);
	prefs.end();
	return true;
}

static void saveSettings(const State &s)
{
	prefs.begin(NVS_NAMESPACE, false);
	prefs.putUChar(NVS_KEY_MAGIC, NVS_SETTINGS_MAGIC);
	prefs.putUChar(NVS_KEY_VERSION, NVS_SETTINGS_VERSION);
	prefs.putUChar("variant", (uint8_t)s.variant);
	prefs.putUChar("fsd", s.fsdEnabled ? 1 : 0);
	prefs.putUChar("ffsd", s.fsdForceEnabled ? 1 : 0);
	prefs.putUChar("sp", (uint8_t)s.speedProfile);
	prefs.putUChar("spPin", s.profileOverride ? 1 : 0);
	prefs.putUChar("offset", (uint8_t)s.speedOffset);
	prefs.putUChar("offPin", s.offsetOverride ? 1 : 0);
	prefs.putUChar("isa", s.isaChimeSuppress ? 1 : 0);
	prefs.putUChar("sumInj", s.summonInject ? 1 : 0);
	prefs.putUChar("nagMode", (uint8_t)s.nagMode);
	prefs.putUChar("nagOrgDB", s.nagOrganicDriverBypass ? 1 : 0);
	prefs.putUChar("precond", s.preconditionEnabled ? 1 : 0);
	prefs.putUChar("track", s.trackModeEnabled ? 1 : 0);
	prefs.putUChar("vAuto", s.variantAutoDetect ? 1 : 0);
	prefs.putUChar("apGate", s.apInjectionGateEnabled ? 1 : 0);
	prefs.putUChar("banS", s.banShieldEnabled ? 1 : 0);
	prefs.putUChar("clkMHz", s.canClockReqMHz);
	prefs.putUChar("drvM", s.driveModeOverride);
	prefs.putUChar("eceR79", s.eceR79Bypass ? 1 : 0);
	prefs.putUChar("lhd", s.lhdEnabled ? 1 : 0);
	prefs.putUChar("eap", s.enhancedAutopilot ? 1 : 0);
	prefs.end();
}

static State makeDefault()
{
	State s = {};
	s.variant = HW4;
	s.fsdEnabled = false;
	s.fsdForceEnabled = false;
	s.speedProfile = 1;
	s.profileOverride = false;
	s.speedOffset = 0;
	s.offsetOverride = false;
	s.isaChimeSuppress = false;
	s.summonInject = false;
	s.nagMode = NAG_MODE_OFF;
	s.nagOrganicDriverBypass = false;
	s.preconditionEnabled = false;
	s.trackModeEnabled = false;
	s.variantAutoDetect = true;
	s.apInjectionGateEnabled = false;
	s.banShieldEnabled = false;
	s.canClockReqMHz = 0;
	s.driveModeOverride = 0;
	s.eceR79Bypass = false;
	s.lhdEnabled = false;
	s.enhancedAutopilot = false;
	return s;
}

void setUp()
{
	Preferences::clearAll();
}

void tearDown() {}


void test_persist_save_load_roundtrip_default()
{
	State s = makeDefault();
	saveSettings(s);

	State loaded = {};
	TEST_ASSERT_TRUE(loadSettings(loaded));
	TEST_ASSERT_EQUAL(HW4, loaded.variant);
	TEST_ASSERT_FALSE(loaded.fsdEnabled);
	TEST_ASSERT_EQUAL(NAG_MODE_OFF, loaded.nagMode);
	TEST_ASSERT_EQUAL(1, loaded.speedProfile);
	TEST_ASSERT_FALSE(loaded.profileOverride);
	TEST_ASSERT_EQUAL(0, loaded.speedOffset);
	TEST_ASSERT_FALSE(loaded.offsetOverride);
	TEST_ASSERT_FALSE(loaded.isaChimeSuppress);
}

void test_persist_save_load_roundtrip_hw3_full()
{
	State s = makeDefault();
	s.variant = HW3;
	s.fsdEnabled = true;
	s.nagMode = NAG_MODE_ORGANIC;
	s.speedProfile = 5;
	s.profileOverride = true;
	s.speedOffset = 10;
	s.offsetOverride = true;
	s.isaChimeSuppress = true;
	saveSettings(s);

	State loaded = {};
	TEST_ASSERT_TRUE(loadSettings(loaded));
	TEST_ASSERT_EQUAL(HW3, loaded.variant);
	TEST_ASSERT_TRUE(loaded.fsdEnabled);
	TEST_ASSERT_EQUAL(NAG_MODE_ORGANIC, loaded.nagMode);
	TEST_ASSERT_EQUAL(5, loaded.speedProfile);
	TEST_ASSERT_TRUE(loaded.profileOverride);
	TEST_ASSERT_EQUAL(10, loaded.speedOffset);
	TEST_ASSERT_TRUE(loaded.offsetOverride);
	TEST_ASSERT_TRUE(loaded.isaChimeSuppress);
}

void test_persist_save_load_roundtrip_legacy()
{
	State s = makeDefault();
	s.variant = LEGACY;
	s.fsdEnabled = true;
	s.speedProfile = 3;
	saveSettings(s);

	State loaded = {};
	TEST_ASSERT_TRUE(loadSettings(loaded));
	TEST_ASSERT_EQUAL(LEGACY, loaded.variant);
	TEST_ASSERT_TRUE(loaded.fsdEnabled);
	TEST_ASSERT_EQUAL(3, loaded.speedProfile);
}


void test_persist_load_empty_nvs_returns_false()
{
	State s = {};
	TEST_ASSERT_FALSE(loadSettings(s));
}

void test_persist_load_wrong_magic_returns_false()
{
	Preferences p;
	p.begin("tcm", false);
	p.putUChar("magic", 0x00);
	p.putUChar("ver", NVS_SETTINGS_VERSION);
	p.end();

	State s = {};
	TEST_ASSERT_FALSE(loadSettings(s));
}

void test_persist_load_wrong_version_returns_false()
{
	Preferences p;
	p.begin("tcm", false);
	p.putUChar("magic", NVS_SETTINGS_MAGIC);
	p.putUChar("ver", 0x99);
	p.end();

	State s = {};
	TEST_ASSERT_FALSE(loadSettings(s));
}


void test_persist_save_load_roundtrip_overwrite_preserves_latest()
{
	State s1 = makeDefault();
	s1.variant = HW4;
	s1.fsdEnabled = true;
	saveSettings(s1);

	State s2 = makeDefault();
	s2.variant = HW3;
	s2.fsdEnabled = false;
	s2.speedProfile = 7;
	saveSettings(s2);

	State loaded = {};
	loadSettings(loaded);
	TEST_ASSERT_EQUAL(HW3, loaded.variant);
	TEST_ASSERT_FALSE(loaded.fsdEnabled);
	TEST_ASSERT_EQUAL(7, loaded.speedProfile);
}

void test_persist_ecer79_lhd_roundtrip()
{
	State s = makeDefault();
	s.eceR79Bypass = true;
	s.lhdEnabled = true;
	saveSettings(s);

	State loaded = {};
	TEST_ASSERT_TRUE(loadSettings(loaded));
	TEST_ASSERT_TRUE(loaded.eceR79Bypass);
	TEST_ASSERT_TRUE(loaded.lhdEnabled);
}

void test_persist_ecer79_lhd_false_roundtrip()
{
	State s = makeDefault();
	s.eceR79Bypass = false;
	s.lhdEnabled = false;
	saveSettings(s);

	State loaded = {};
	TEST_ASSERT_TRUE(loadSettings(loaded));
	TEST_ASSERT_FALSE(loaded.eceR79Bypass);
	TEST_ASSERT_FALSE(loaded.lhdEnabled);
}

void test_persist_apgate_roundtrip_true()
{
	State s = makeDefault();
	s.apInjectionGateEnabled = true;
	s.apGateApActive = true;
	s.apGateParked = false;
	s.apGateSummoning = false;
	saveSettings(s);

	State loaded = {};
	TEST_ASSERT_TRUE(loadSettings(loaded));
	TEST_ASSERT_TRUE(loaded.apInjectionGateEnabled);
	TEST_ASSERT_FALSE(loaded.apGateApActive);
	TEST_ASSERT_TRUE(loaded.apGateParked);
	TEST_ASSERT_FALSE(loaded.apGateSummoning);
}

void test_persist_apgate_roundtrip_false()
{
	State s = makeDefault();
	s.apInjectionGateEnabled = false;
	saveSettings(s);

	State loaded = {};
	TEST_ASSERT_TRUE(loadSettings(loaded));
	TEST_ASSERT_FALSE(loaded.apInjectionGateEnabled);
}

void test_persist_hidden_ap_roundtrip_true()
{
	State s = makeDefault();
	s.enhancedAutopilot = true;
	saveSettings(s);

	State loaded = {};
	TEST_ASSERT_TRUE(loadSettings(loaded));
	TEST_ASSERT_TRUE(loaded.enhancedAutopilot);
}

void test_persist_hidden_ap_roundtrip_false()
{
	State s = makeDefault();
	s.enhancedAutopilot = false;
	saveSettings(s);

	State loaded = {};
	TEST_ASSERT_TRUE(loadSettings(loaded));
	TEST_ASSERT_FALSE(loaded.enhancedAutopilot);
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_persist_save_load_roundtrip_default);
	RUN_TEST(test_persist_save_load_roundtrip_hw3_full);
	RUN_TEST(test_persist_save_load_roundtrip_legacy);
	RUN_TEST(test_persist_load_empty_nvs_returns_false);
	RUN_TEST(test_persist_load_wrong_magic_returns_false);
	RUN_TEST(test_persist_load_wrong_version_returns_false);
	RUN_TEST(test_persist_save_load_roundtrip_overwrite_preserves_latest);
	RUN_TEST(test_persist_ecer79_lhd_roundtrip);
	RUN_TEST(test_persist_ecer79_lhd_false_roundtrip);
	RUN_TEST(test_persist_apgate_roundtrip_true);
	RUN_TEST(test_persist_apgate_roundtrip_false);
	RUN_TEST(test_persist_hidden_ap_roundtrip_true);
	RUN_TEST(test_persist_hidden_ap_roundtrip_false);

	return UNITY_END();
}

