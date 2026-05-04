// ── CRC-8/OPENSAFETY Tests ───────────────────────────────────────────────────
// Validates CRC-8 polynomial 0x2F lookup table and teslaCrc8 per-ID magic.

#include <unity.h>
#include <cstring>

#include "infra/can/crc8.h"

void setUp() {}
void tearDown() {}

// ── crc8_opensafety basic ────────────────────────────────────────────────────

void test_crc8_empty_returns_zero()
{
	uint8_t data[1] = {0x00};
	TEST_ASSERT_EQUAL_UINT8(0x00, crc8_opensafety(data, 0));
}

void test_crc8_single_byte_zero()
{
	uint8_t data[] = {0x00};
	// CRC of 0x00 with poly 0x2F, init 0
	uint8_t crc = crc8_opensafety(data, 1);
	// Just ensure deterministic: re-compute manually
	// CRC(0x00) = table[0x00] = 0x00
	TEST_ASSERT_EQUAL_UINT8(0x00, crc);
}

void test_crc8_single_byte_ff()
{
	uint8_t data[] = {0xFF};
	uint8_t crc = crc8_opensafety(data, 1);
	// table[0xFF] — should be deterministic non-zero
	TEST_ASSERT_NOT_EQUAL(0x00, crc);
}

void test_crc8_known_sequence()
{
	// Standard CRC-8/OPENSAFETY check: "123456789" -> 0x3E
	// NOTE: This is the standard test vector for poly 0x2F
	uint8_t data[] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
	TEST_ASSERT_EQUAL_UINT8(0x3E, crc8_opensafety(data, 9));
}

void test_crc8_deterministic()
{
	uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF};
	uint8_t a = crc8_opensafety(data, 4);
	uint8_t b = crc8_opensafety(data, 4);
	TEST_ASSERT_EQUAL_UINT8(a, b);
}

// ── teslaCrc8 — per CAN ID magic XOR ────────────────────────────────────────

void test_teslaCrc8_0x229_deterministic()
{
	uint8_t data[8];
	memset(data, 0x55, 8);
	uint8_t a = teslaCrc8(data, 7, data[0] & 0x0F, MAGIC_0x229);
	uint8_t b = teslaCrc8(data, 7, data[0] & 0x0F, MAGIC_0x229);
	TEST_ASSERT_EQUAL_UINT8(a, b);
}

void test_teslaCrc8_0x249_deterministic()
{
	uint8_t data[8];
	memset(data, 0xAA, 8);
	uint8_t a = teslaCrc8(data, 7, data[0] & 0x0F, MAGIC_0x249);
	uint8_t b = teslaCrc8(data, 7, data[0] & 0x0F, MAGIC_0x249);
	TEST_ASSERT_EQUAL_UINT8(a, b);
}

void test_teslaCrc8_0x370_deterministic()
{
	uint8_t data[8] = {};
	data[0] = 0x01;
	uint8_t a = teslaCrc8(data, 7, data[0] & 0x0F, MAGIC_0x370);
	uint8_t b = teslaCrc8(data, 7, data[0] & 0x0F, MAGIC_0x370);
	TEST_ASSERT_EQUAL_UINT8(a, b);
}

void test_teslaCrc8_different_data_different_crc()
{
	uint8_t d1[8] = {};
	uint8_t d2[8];
	memset(d2, 0xFF, 8);
	uint8_t c1 = teslaCrc8(d1, 7, d1[0] & 0x0F, MAGIC_0x229);
	uint8_t c2 = teslaCrc8(d2, 7, d2[0] & 0x0F, MAGIC_0x229);
	TEST_ASSERT_TRUE(c1 != c2);
}

void test_teslaCrc8_zero_magic_equals_base_crc()
{
	static const uint8_t MAGIC_ZERO[16] = {};
	uint8_t data[7];
	memset(data, 0x55, 7);
	uint8_t crc = teslaCrc8(data, 7, 0, MAGIC_ZERO);
	// With zero magic, result = crc8_opensafety(data + 0x00) ^ 0x00
	uint8_t buf[8];
	memcpy(buf, data, 7);
	buf[7] = 0x00;
	TEST_ASSERT_EQUAL_UINT8(crc8_opensafety(buf, 8), crc);
}

void test_teslaCrc8_mux_counter_varies_magic()
{
	// Different counters should produce different CRCs via magic XOR
	uint8_t d1[8] = {};
	uint8_t d2[8] = {};
	d1[0] = 0x00; // counter 0
	d2[0] = 0x01; // counter 1
	uint8_t c1 = teslaCrc8(d1, 7, d1[0] & 0x0F, MAGIC_0x229);
	uint8_t c2 = teslaCrc8(d2, 7, d2[0] & 0x0F, MAGIC_0x229);
	TEST_ASSERT_TRUE(c1 != c2);
}

// ── main ─────────────────────────────────────────────────────────────────────

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_crc8_empty_returns_zero);
	RUN_TEST(test_crc8_single_byte_zero);
	RUN_TEST(test_crc8_single_byte_ff);
	RUN_TEST(test_crc8_known_sequence);
	RUN_TEST(test_crc8_deterministic);
	RUN_TEST(test_teslaCrc8_0x229_deterministic);
	RUN_TEST(test_teslaCrc8_0x249_deterministic);
	RUN_TEST(test_teslaCrc8_0x370_deterministic);
	RUN_TEST(test_teslaCrc8_different_data_different_crc);
	RUN_TEST(test_teslaCrc8_zero_magic_equals_base_crc);
	RUN_TEST(test_teslaCrc8_mux_counter_varies_magic);
	return UNITY_END();
}
