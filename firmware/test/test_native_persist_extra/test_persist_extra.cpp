/** @file firmware/test/test_native_persist_extra/test_persist_extra.cpp
 *  @brief Round-trip tests for the four newly persisted settings (C1-11)
 *  @author Tesla CAN Mod Contributors
 *  @license GPL-3.0
 *
 *  Uses the real native persist backend (core/persist/native/board.h) with the
 *  in-memory Preferences fake so the test exercises shipping load/save code.
 */

#include <unity.h>
#include <cstring>

#include "core/types.h"
#include "core/persist/native/board.h"

void setUp()
{
	Preferences::clearAll();
}
void tearDown() {}

static State makeDefault()
{
	State s = {};
	s.variant = HW4;
	return s;
}

/** @brief All four new settings must survive a save/load cycle. */
void test_new_settings_roundtrip_true()
{
	State s = makeDefault();
	s.regionSpoofCode = 4;
	s.alcAutoConfirmEnabled = true;
	s.apFirstEnabled = true;
	s.rawCanListen = true;
	saveSettings(s);

	State loaded = {};
	TEST_ASSERT_TRUE(loadSettings(loaded));
	TEST_ASSERT_EQUAL_UINT8(4, loaded.regionSpoofCode);
	TEST_ASSERT_TRUE(loaded.alcAutoConfirmEnabled);
	TEST_ASSERT_TRUE(loaded.apFirstEnabled);
	TEST_ASSERT_TRUE(loaded.rawCanListen);
}

/** @brief Clearing all four settings must also survive a save/load cycle. */
void test_new_settings_roundtrip_false()
{
	State s = makeDefault();
	s.regionSpoofCode = 0;
	s.alcAutoConfirmEnabled = false;
	s.apFirstEnabled = false;
	s.rawCanListen = false;
	saveSettings(s);

	State loaded = {};
	loaded.regionSpoofCode = 9;
	loaded.alcAutoConfirmEnabled = true;
	loaded.apFirstEnabled = true;
	loaded.rawCanListen = true;
	TEST_ASSERT_TRUE(loadSettings(loaded));
	TEST_ASSERT_EQUAL_UINT8(0, loaded.regionSpoofCode);
	TEST_ASSERT_FALSE(loaded.alcAutoConfirmEnabled);
	TEST_ASSERT_FALSE(loaded.apFirstEnabled);
	TEST_ASSERT_FALSE(loaded.rawCanListen);
}

/** @brief Schema version must be bumped so existing NVS data is rewritten. */
void test_settings_version_is_bumped()
{
	State s = makeDefault();
	saveSettings(s);

	Preferences prefs;
	prefs.begin(NVS_NAMESPACE, true);
	TEST_ASSERT_EQUAL_UINT8(NVS_SETTINGS_VERSION, prefs.getUChar(NVS_KEY_VERSION, 0));
	prefs.end();
	TEST_ASSERT_EQUAL_UINT8(0x0F, NVS_SETTINGS_VERSION);
}

int main()
{
	UNITY_BEGIN();
	RUN_TEST(test_new_settings_roundtrip_true);
	RUN_TEST(test_new_settings_roundtrip_false);
	RUN_TEST(test_settings_version_is_bumped);
	return UNITY_END();
}
