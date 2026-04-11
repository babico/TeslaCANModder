// ── ESP32 NVS Persistence Tests ──────────────────────────────────────────────
// Tests loadSettings/saveSettings roundtrip using fake Preferences API.

#include <unity.h>
#include <cstring>

// Provide fake Preferences before including persist
#include "../support/fake_preferences.h"
// Provide Preferences.h guard so persist/esp32.h doesn't try to include real one
#define Preferences_h

#include "core/types.h"

// ── Inline the persist logic (matches persist/esp32.h) ──────────────────────
#define NVS_NAMESPACE "tcm"
#define NVS_KEY_MAGIC "magic"
#define NVS_KEY_VERSION "ver"
#define NVS_SETTINGS_MAGIC 0xCA
#define NVS_SETTINGS_VERSION 0x02

static Preferences prefs;

static bool loadSettings(State& s) {
  prefs.begin(NVS_NAMESPACE, true);
  uint8_t magic = prefs.getUChar(NVS_KEY_MAGIC, 0);
  uint8_t ver   = prefs.getUChar(NVS_KEY_VERSION, 0);
  if (magic != NVS_SETTINGS_MAGIC || ver != NVS_SETTINGS_VERSION) {
    prefs.end();
    return false;
  }
  s.variant          = (Variant)prefs.getUChar("variant", 0);
  s.fsdEnabled       = prefs.getUChar("fsd", 0);
  s.nagSuppress      = prefs.getUChar("nag", 0);
  s.speedProfile     = prefs.getUChar("sp", 1);
  s.profileOverride  = prefs.getUChar("spPin", 0);
  s.speedOffset      = prefs.getUChar("offset", 0);
  s.offsetOverride   = prefs.getUChar("offPin", 0);
  s.isaChimeSuppress = prefs.getUChar("isa", 0);
  prefs.end();
  return true;
}

static void saveSettings(const State& s) {
  prefs.begin(NVS_NAMESPACE, false);
  prefs.putUChar(NVS_KEY_MAGIC, NVS_SETTINGS_MAGIC);
  prefs.putUChar(NVS_KEY_VERSION, NVS_SETTINGS_VERSION);
  prefs.putUChar("variant", (uint8_t)s.variant);
  prefs.putUChar("fsd", s.fsdEnabled ? 1 : 0);
  prefs.putUChar("nag", s.nagSuppress ? 1 : 0);
  prefs.putUChar("sp", (uint8_t)s.speedProfile);
  prefs.putUChar("spPin", s.profileOverride ? 1 : 0);
  prefs.putUChar("offset", (uint8_t)s.speedOffset);
  prefs.putUChar("offPin", s.offsetOverride ? 1 : 0);
  prefs.putUChar("isa", s.isaChimeSuppress ? 1 : 0);
  prefs.end();
}

static State makeDefault() {
  State s = {};
  s.variant = HW4;
  s.fsdEnabled = false;
  s.nagSuppress = false;
  s.speedProfile = 1;
  s.profileOverride = false;
  s.speedOffset = 0;
  s.offsetOverride = false;
  s.isaChimeSuppress = false;
  return s;
}

void setUp() {
  Preferences::clearAll();
}

void tearDown() {}

// ═══════════════════════════════════════════════════════════════════════════════
// Save/Load Roundtrip
// ═══════════════════════════════════════════════════════════════════════════════

void test_persist_save_load_roundtrip_default() {
  State s = makeDefault();
  saveSettings(s);

  State loaded = {};
  TEST_ASSERT_TRUE(loadSettings(loaded));
  TEST_ASSERT_EQUAL(HW4, loaded.variant);
  TEST_ASSERT_FALSE(loaded.fsdEnabled);
  TEST_ASSERT_FALSE(loaded.nagSuppress);
  TEST_ASSERT_EQUAL(1, loaded.speedProfile);
  TEST_ASSERT_FALSE(loaded.profileOverride);
  TEST_ASSERT_EQUAL(0, loaded.speedOffset);
  TEST_ASSERT_FALSE(loaded.offsetOverride);
  TEST_ASSERT_FALSE(loaded.isaChimeSuppress);
}

void test_persist_save_load_roundtrip_hw3_full() {
  State s = makeDefault();
  s.variant = HW3;
  s.fsdEnabled = true;
  s.nagSuppress = true;
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
  TEST_ASSERT_TRUE(loaded.nagSuppress);
  TEST_ASSERT_EQUAL(5, loaded.speedProfile);
  TEST_ASSERT_TRUE(loaded.profileOverride);
  TEST_ASSERT_EQUAL(10, loaded.speedOffset);
  TEST_ASSERT_TRUE(loaded.offsetOverride);
  TEST_ASSERT_TRUE(loaded.isaChimeSuppress);
}

void test_persist_save_load_roundtrip_legacy() {
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

// ═══════════════════════════════════════════════════════════════════════════════
// Corrupt/Missing NVS
// ═══════════════════════════════════════════════════════════════════════════════

void test_persist_load_empty_nvs_returns_false() {
  State s = {};
  TEST_ASSERT_FALSE(loadSettings(s));
}

void test_persist_load_wrong_magic_returns_false() {
  Preferences p;
  p.begin("tcm", false);
  p.putUChar("magic", 0x00);
  p.putUChar("ver", NVS_SETTINGS_VERSION);
  p.end();

  State s = {};
  TEST_ASSERT_FALSE(loadSettings(s));
}

void test_persist_load_wrong_version_returns_false() {
  Preferences p;
  p.begin("tcm", false);
  p.putUChar("magic", NVS_SETTINGS_MAGIC);
  p.putUChar("ver", 0x99);
  p.end();

  State s = {};
  TEST_ASSERT_FALSE(loadSettings(s));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Overwrite
// ═══════════════════════════════════════════════════════════════════════════════

void test_persist_overwrite_preserves_latest() {
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

int main() {
  UNITY_BEGIN();

  RUN_TEST(test_persist_save_load_roundtrip_default);
  RUN_TEST(test_persist_save_load_roundtrip_hw3_full);
  RUN_TEST(test_persist_save_load_roundtrip_legacy);
  RUN_TEST(test_persist_load_empty_nvs_returns_false);
  RUN_TEST(test_persist_load_wrong_magic_returns_false);
  RUN_TEST(test_persist_load_wrong_version_returns_false);
  RUN_TEST(test_persist_overwrite_preserves_latest);

  return UNITY_END();
}
