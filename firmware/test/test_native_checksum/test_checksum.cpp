/**
 * @file firmware/test/test_native_checksum/test_checksum.cpp
 * @brief Unit tests for CAN frame checksum helpers
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <unity.h>
#include <cstring>

#include "vehicle/can/checksum.h"

void setUp() {}
void tearDown() {}

/* ── dasChecksum ─────────────────────────────────────────────────────────── */

void test_dasChecksum_basic()
{
	uint8_t data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
	uint8_t crc = dasChecksum(0x03FD, data, 8, 7);
	uint8_t expected = 0;
	expected += (0x03FD & 0xFF);
	expected += ((0x03FD >> 8) & 0xFF);
	for (int i = 0; i < 8; i++)
		if (i != 7)
			expected += data[i];
	TEST_ASSERT_EQUAL_UINT8(expected, crc);
}

void test_dasChecksum_excludes_checksum_byte()
{
	uint8_t data[4] = {0x10, 0x20, 0x30, 0x40};
	uint8_t crc = dasChecksum(0x0100, data, 4, 2);
	uint8_t expected = (0x00 + 0x01) + 0x10 + 0x20 + 0x40;
	TEST_ASSERT_EQUAL_UINT8(expected, crc);
}

void test_dasChecksum_zero_can_id()
{
	uint8_t data[4] = {0x01, 0x02, 0x03, 0x00};
	uint8_t crc = dasChecksum(0x0000, data, 4, 3);
	uint8_t expected = 0x01 + 0x02 + 0x03;
	TEST_ASSERT_EQUAL_UINT8(expected, crc);
}

void test_dasChecksum_single_byte()
{
	uint8_t data[1] = {0x42};
	uint8_t crc = dasChecksum(0x0000, data, 1, 0);
	TEST_ASSERT_EQUAL_UINT8(0, crc);
}

/* ── computeHW4IsaChecksum ───────────────────────────────────────────────── */

void test_hw4_isa_checksum_basic()
{
	Frame f = {};
	f.id = 0x0399;
	f.dlc = 8;
	for (int i = 0; i < 8; i++)
		f.data[i] = (uint8_t)(i + 1);
	uint8_t crc = computeHW4IsaChecksum(f);
	uint8_t expected = 0;
	for (int i = 0; i < 7; i++)
		expected += f.data[i];
	expected += (f.id & 0xFF) + (f.id >> 8);
	TEST_ASSERT_EQUAL_UINT8(expected, crc);
}

void test_hw4_isa_checksum_returns_zero_when_dlc_less_than_8()
{
	Frame f = {};
	f.id = 0x0399;
	f.dlc = 7;
	memset(f.data, 0x01, 8);
	TEST_ASSERT_EQUAL_UINT8(0, computeHW4IsaChecksum(f));
}

void test_hw4_isa_checksum_all_zeros()
{
	Frame f = {};
	f.id = 0x0399;
	f.dlc = 8;
	memset(f.data, 0, 8);
	uint8_t expected = (0x99 + 0x03);
	TEST_ASSERT_EQUAL_UINT8(expected, computeHW4IsaChecksum(f));
}

/* ── nagChecksum ─────────────────────────────────────────────────────────── */

void test_nag_checksum_basic()
{
	uint8_t data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
	uint8_t crc = nagChecksum(data);
	uint16_t expected = 0;
	for (int i = 0; i < 7; i++)
		expected += data[i];
	expected += (0x370 & 0xFF);
	expected += ((0x370 >> 8) & 0xFF);
	TEST_ASSERT_EQUAL_UINT8(expected & 0xFF, crc);
}

void test_nag_checksum_all_zeros()
{
	uint8_t data[8] = {};
	uint8_t crc = nagChecksum(data);
	uint8_t expected = (0x70 + 0x03) & 0xFF;
	TEST_ASSERT_EQUAL_UINT8(expected, crc);
}

void test_nag_checksum_overflow_wraps()
{
	uint8_t data[8];
	memset(data, 0xFF, 8);
	uint8_t crc = nagChecksum(data);
	uint16_t sum = 0;
	for (int i = 0; i < 7; i++)
		sum += data[i];
	sum += 0x70 + 0x03;
	TEST_ASSERT_EQUAL_UINT8(sum & 0xFF, crc);
}

/* ── driveChecksum ───────────────────────────────────────────────────────── */

void test_drive_checksum_basic()
{
	uint8_t data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
	uint8_t crc = driveChecksum(data, 8);
	uint8_t expected = 0x01 + 0x02 + 0x03 + 0x04 + 0x05 + 0x06 + 0x07;
	TEST_ASSERT_EQUAL_UINT8(expected, crc);
}

void test_drive_checksum_three_bytes()
{
	uint8_t data[3] = {0x10, 0x20, 0x30};
	uint8_t crc = driveChecksum(data, 3);
	TEST_ASSERT_EQUAL_UINT8(0x30, crc);
}

void test_drive_checksum_all_zeros()
{
	uint8_t data[8] = {};
	TEST_ASSERT_EQUAL_UINT8(0, driveChecksum(data, 8));
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_dasChecksum_basic);
	RUN_TEST(test_dasChecksum_excludes_checksum_byte);
	RUN_TEST(test_dasChecksum_zero_can_id);
	RUN_TEST(test_dasChecksum_single_byte);

	RUN_TEST(test_hw4_isa_checksum_basic);
	RUN_TEST(test_hw4_isa_checksum_returns_zero_when_dlc_less_than_8);
	RUN_TEST(test_hw4_isa_checksum_all_zeros);

	RUN_TEST(test_nag_checksum_basic);
	RUN_TEST(test_nag_checksum_all_zeros);
	RUN_TEST(test_nag_checksum_overflow_wraps);

	RUN_TEST(test_drive_checksum_basic);
	RUN_TEST(test_drive_checksum_three_bytes);
	RUN_TEST(test_drive_checksum_all_zeros);

	return UNITY_END();
}
