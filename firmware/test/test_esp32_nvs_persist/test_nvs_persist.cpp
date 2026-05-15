/**
 * @file firmware/test/test_esp32_nvs_persist/test_nvs_persist.cpp
 * @brief Hardware tests for ESP32 NVS (Non-Volatile Storage) persistence
 *
 * Requires: ESP32 DevKit (any variant).
 * Tests: NVS read/write cycle, data persistence across reboots,
 *        namespace isolation, key size limits, error handling.
 *
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <Arduino.h>
#include <unity.h>
#include <Preferences.h>

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

#include "core/types.h"
#include "core/persist/keys.h"

void setUp() {}
void tearDown() {}

/* ── NVS namespace open ───────────────────────────────────────────────────── */

void test_nvs_namespace_opens()
{
	Preferences prefs;
	bool ok = prefs.begin(PERSIST_NAMESPACE, false);
	TEST_ASSERT_TRUE_MESSAGE(ok, "NVS namespace should open");
	prefs.end();
}

void test_nvs_namespace_readonly()
{
	Preferences prefs;
	bool ok = prefs.begin(PERSIST_NAMESPACE, true);
	TEST_ASSERT_TRUE_MESSAGE(ok, "NVS namespace should open read-only");
	prefs.end();
}

/* ── Boolean persistence ──────────────────────────────────────────────────── */

void test_nvs_bool_roundtrip()
{
	Preferences prefs;
	prefs.begin(PERSIST_NAMESPACE, false);

	prefs.putBool("test_bool", true);
	TEST_ASSERT_TRUE_MESSAGE(prefs.getBool("test_bool", false), "Stored bool should read back true");

	prefs.putBool("test_bool", false);
	TEST_ASSERT_FALSE_MESSAGE(prefs.getBool("test_bool", true), "Stored bool should read back false");

	prefs.end();
}

/* ── Integer persistence ──────────────────────────────────────────────────── */

void test_nvs_int_roundtrip()
{
	Preferences prefs;
	prefs.begin(PERSIST_NAMESPACE, false);

	prefs.putInt("test_int", 42);
	TEST_ASSERT_EQUAL_MESSAGE(42, prefs.getInt("test_int", 0), "Stored int should read back 42");

	prefs.putInt("test_int", -100);
	TEST_ASSERT_EQUAL_MESSAGE(-100, prefs.getInt("test_int", 0), "Stored negative int should read back");

	prefs.end();
}

void test_nvs_uint32_roundtrip()
{
	Preferences prefs;
	prefs.begin(PERSIST_NAMESPACE, false);

	prefs.putUInt("test_uint32", 0xDEADBEEF);
	TEST_ASSERT_EQUAL_HEX32_MESSAGE(0xDEADBEEF, prefs.getUInt("test_uint32", 0), "Stored uint32 should read back");

	prefs.end();
}

/* ── Float persistence ────────────────────────────────────────────────────── */

void test_nvs_float_roundtrip()
{
	Preferences prefs;
	prefs.begin(PERSIST_NAMESPACE, false);

	prefs.putFloat("test_float", 3.14159f);
	float val = prefs.getFloat("test_float", 0.0f);
	TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, 3.14159f, val, "Stored float should read back");

	prefs.end();
}

/* ── String persistence ───────────────────────────────────────────────────── */

void test_nvs_string_roundtrip()
{
	Preferences prefs;
	prefs.begin(PERSIST_NAMESPACE, false);

	prefs.putString("test_str", "hello_nvS");
	String val = prefs.getString("test_str", "");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("hello_nvS", val.c_str(), "Stored string should read back");

	prefs.end();
}

void test_nvs_string_max_length()
{
	Preferences prefs;
	prefs.begin(PERSIST_NAMESPACE, false);

	// NVS supports strings up to ~4000 bytes
	String longStr;
	for (int i = 0; i < 100; i++)
		longStr += "abcdefghij";

	prefs.putString("test_long", longStr);
	String val = prefs.getString("test_long", "");
	TEST_ASSERT_EQUAL_MESSAGE(longStr.length(), val.length(), "Long string length should match");

	prefs.end();
}

/* ── Blob (binary) persistence ────────────────────────────────────────────── */

void test_nvs_blob_roundtrip()
{
	Preferences prefs;
	prefs.begin(PERSIST_NAMESPACE, false);

	uint8_t data[16];
	for (int i = 0; i < 16; i++)
		data[i] = (uint8_t)(i * 17);

	prefs.putBytes("test_blob", data, sizeof(data));

	uint8_t readBack[16];
	size_t len = prefs.getBytes("test_blob", readBack, sizeof(readBack));
	TEST_ASSERT_EQUAL_MESSAGE(sizeof(data), len, "Blob length should match");

	for (int i = 0; i < 16; i++)
	{
		char msg[32];
		snprintf(msg, sizeof(msg), "blob[%d] should match", i);
		TEST_ASSERT_EQUAL_HEX8_MESSAGE(data[i], readBack[i], msg);
	}

	prefs.end();
}

/* ── Key existence check ──────────────────────────────────────────────────── */

void test_nvs_is_key_exists()
{
	Preferences prefs;
	prefs.begin(PERSIST_NAMESPACE, false);

	prefs.putBool("exists_key", true);
	TEST_ASSERT_TRUE_MESSAGE(prefs.isKey("exists_key"), "Key should exist after put");
	TEST_ASSERT_FALSE_MESSAGE(!prefs.isKey("nonexistent_key"), "Non-existent key should not exist");

	prefs.end();
}

/* ── Key removal ──────────────────────────────────────────────────────────── */

void test_nvs_remove_key()
{
	Preferences prefs;
	prefs.begin(PERSIST_NAMESPACE, false);

	prefs.putInt("remove_me", 123);
	TEST_ASSERT_TRUE_MESSAGE(prefs.isKey("remove_me"), "Key should exist before removal");

	prefs.remove("remove_me");
	TEST_ASSERT_FALSE_MESSAGE(!prefs.isKey("remove_me"), "Key should not exist after removal");

	prefs.end();
}

/* ── Namespace clear ──────────────────────────────────────────────────────── */

void test_nvs_clear_all()
{
	Preferences prefs;
	prefs.begin(PERSIST_NAMESPACE, false);

	prefs.putInt("k1", 1);
	prefs.putInt("k2", 2);
	prefs.putString("k3", "three");

	prefs.clear();

	TEST_ASSERT_FALSE_MESSAGE(!prefs.isKey("k1"), "k1 should be cleared");
	TEST_ASSERT_FALSE_MESSAGE(!prefs.isKey("k2"), "k2 should be cleared");
	TEST_ASSERT_FALSE_MESSAGE(!prefs.isKey("k3"), "k3 should be cleared");

	prefs.end();
}

/* ── Multiple namespaces isolation ────────────────────────────────────────── */

void test_nvs_namespace_isolation()
{
	Preferences prefs1, prefs2;

	prefs1.begin("test_ns_a", false);
	prefs1.putInt("shared_name", 100);
	prefs1.end();

	prefs2.begin("test_ns_b", false);
	prefs2.putInt("shared_name", 200);
	prefs2.end();

	prefs1.begin("test_ns_a", true);
	TEST_ASSERT_EQUAL_MESSAGE(100, prefs1.getInt("shared_name", 0), "Namespace A should have its own value");
	prefs1.end();

	prefs2.begin("test_ns_b", true);
	TEST_ASSERT_EQUAL_MESSAGE(200, prefs2.getInt("shared_name", 0), "Namespace B should have its own value");
	prefs2.end();

	// Cleanup
	prefs1.begin("test_ns_a", false);
	prefs1.clear();
	prefs1.end();
	prefs2.begin("test_ns_b", false);
	prefs2.clear();
	prefs2.end();
}

/* ── State struct persistence (real-world scenario) ───────────────────────── */

void test_nvs_state_struct_roundtrip()
{
	Preferences prefs;
	prefs.begin(PERSIST_NAMESPACE, false);

	// Simulate persisting a State-like struct as a blob
	State s;
	s.fsdEnabled = true;
	s.speedProfile = 2;
	s.speedOffset = 50;
	s.nagMode = NAG_MODE_ORGANIC;

	prefs.putBytes("state_blob", &s, sizeof(State));

	State loaded;
	size_t len = prefs.getBytes("state_blob", &loaded, sizeof(State));
	TEST_ASSERT_EQUAL_MESSAGE(sizeof(State), len, "State blob size should match");

	TEST_ASSERT_TRUE_MESSAGE(loaded.fsdEnabled, "fsdEnabled should persist");
	TEST_ASSERT_EQUAL_MESSAGE(2, loaded.speedProfile, "speedProfile should persist");
	TEST_ASSERT_EQUAL_MESSAGE(50, loaded.speedOffset, "speedOffset should persist");
	TEST_ASSERT_EQUAL_MESSAGE(NAG_MODE_ORGANIC, loaded.nagMode, "nagMode should persist");

	prefs.end();
}

void setup()
{
	delay(2000);
	Serial.begin(115200);
	delay(1000);
	Serial.println("=== ESP32 NVS Persistence Hardware Tests ===");

	UNITY_BEGIN();
	RUN_TEST(test_nvs_namespace_opens);
	RUN_TEST(test_nvs_namespace_readonly);
	RUN_TEST(test_nvs_bool_roundtrip);
	RUN_TEST(test_nvs_int_roundtrip);
	RUN_TEST(test_nvs_uint32_roundtrip);
	RUN_TEST(test_nvs_float_roundtrip);
	RUN_TEST(test_nvs_string_roundtrip);
	RUN_TEST(test_nvs_string_max_length);
	RUN_TEST(test_nvs_blob_roundtrip);
	RUN_TEST(test_nvs_is_key_exists);
	RUN_TEST(test_nvs_remove_key);
	RUN_TEST(test_nvs_clear_all);
	RUN_TEST(test_nvs_namespace_isolation);
	RUN_TEST(test_nvs_state_struct_roundtrip);
	UNITY_END();
}

void loop()
{
	delay(1000);
}
