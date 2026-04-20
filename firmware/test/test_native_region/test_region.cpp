// ── Region / ECE R79 Tests ───────────────────────────────────────────────────
// Tests region detection, Chinese market check, and ECE R79 bypass application.

#include <unity.h>
#include <cstring>

class __FlashStringHelper;
#define F(s) (reinterpret_cast<const __FlashStringHelper*>(s))

#include "core/types.h"
#include "infra/can.h"

void sendLog(const char*) {}
void sendLog(const __FlashStringHelper*) {}
void resetHandlerLogFlags() {}
void saveSettings(const State&) {}

#include "feature/region.h"

void setUp() {}
void tearDown() {}

// ── decodeRegionCode ─────────────────────────────────────────────────────────

void test_decode_region_from_0x398() {
  Frame f = {};
  f.id = 0x398;
  f.dlc = 8;
  f.data[2] = 0x20; // Region code 2 in byte[2] high nibble
  State s = {};
  decodeRegionCode(f, s);
  TEST_ASSERT_TRUE(s.hasRegion);
  TEST_ASSERT_EQUAL_UINT8(0x02, s.regionCode);
}

void test_decode_region_sets_hasRegion() {
  Frame f = {};
  f.id = 0x398;
  f.dlc = 8;
  f.data[2] = 0x00;
  State s = {};
  s.hasRegion = false;
  decodeRegionCode(f, s);
  TEST_ASSERT_TRUE(s.hasRegion);
}

void test_decode_region_extracts_high_nibble() {
  Frame f = {};
  f.id = 0x398;
  f.dlc = 8;
  f.data[2] = 0x3F; // High nibble = 3
  State s = {};
  decodeRegionCode(f, s);
  TEST_ASSERT_EQUAL_UINT8(0x03, s.regionCode);
}

// ── isChineseMarket ──────────────────────────────────────────────────────────

void test_chinese_market_code_3() {
  TEST_ASSERT_TRUE(isChineseMarket(3));
}

void test_not_chinese_market_code_0() {
  TEST_ASSERT_FALSE(isChineseMarket(0));
}

void test_not_chinese_market_code_2() {
  TEST_ASSERT_FALSE(isChineseMarket(2));
}

// ── isEuropeanMarket ─────────────────────────────────────────────────────────

void test_european_market_code_2() {
  TEST_ASSERT_TRUE(isEuropeanMarket(2));
}

void test_not_european_market_code_1() {
  TEST_ASSERT_FALSE(isEuropeanMarket(1));
}

// ── applyEceR79Bypass ────────────────────────────────────────────────────────

void test_ece_r79_clears_bit20() {
  Frame f = {};
  f.dlc = 8;
  // Set bit 20: byte 2, bit 4
  f.data[2] = 0xFF;
  applyEceR79Bypass(f);
  // Bit 20 (byte 2, bit 4) should be cleared
  TEST_ASSERT_BITS_LOW(0x10, f.data[2]);
}

void test_ece_r79_preserves_other_bits() {
  Frame f = {};
  f.dlc = 8;
  f.data[0] = 0xFF;
  f.data[1] = 0xFF;
  f.data[3] = 0xFF;
  applyEceR79Bypass(f);
  TEST_ASSERT_EQUAL_HEX8(0xFF, f.data[0]);
  TEST_ASSERT_EQUAL_HEX8(0xFF, f.data[1]);
  TEST_ASSERT_EQUAL_HEX8(0xFF, f.data[3]);
}

void test_ece_r79_already_clear_is_noop() {
  Frame f = {};
  f.dlc = 8;
  f.data[2] = 0x00;
  applyEceR79Bypass(f);
  TEST_ASSERT_EQUAL_HEX8(0x00, f.data[2]);
}

// ── main ─────────────────────────────────────────────────────────────────────

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_decode_region_from_0x398);
  RUN_TEST(test_decode_region_sets_hasRegion);
  RUN_TEST(test_decode_region_extracts_high_nibble);
  RUN_TEST(test_chinese_market_code_3);
  RUN_TEST(test_not_chinese_market_code_0);
  RUN_TEST(test_not_chinese_market_code_2);
  RUN_TEST(test_european_market_code_2);
  RUN_TEST(test_not_european_market_code_1);
  RUN_TEST(test_ece_r79_clears_bit20);
  RUN_TEST(test_ece_r79_preserves_other_bits);
  RUN_TEST(test_ece_r79_already_clear_is_noop);
  return UNITY_END();
}
