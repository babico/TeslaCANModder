/**
 * @file firmware/test/test_native_ids/test_ids.cpp
 * @brief Unit tests for CAN ID helpers in ids.h
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <unity.h>
#include <cstring>

#include "vehicle/can/ids.h"
#include "support/helpers.h"

void setUp() {}
void tearDown() {}

/* ── readFollowDistance ──────────────────────────────────────────────────── */

void test_read_follow_distance_value_1()
{
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	f.data[5] = 0b00100000;
	TEST_ASSERT_EQUAL_UINT8(1, readFollowDistance(f));
}

void test_read_follow_distance_value_2()
{
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	f.data[5] = 0b01000000;
	TEST_ASSERT_EQUAL_UINT8(2, readFollowDistance(f));
}

void test_read_follow_distance_value_3()
{
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	f.data[5] = 0b01100000;
	TEST_ASSERT_EQUAL_UINT8(3, readFollowDistance(f));
}

void test_read_follow_distance_value_4()
{
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	f.data[5] = 0b10000000;
	TEST_ASSERT_EQUAL_UINT8(4, readFollowDistance(f));
}

void test_read_follow_distance_value_0()
{
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	f.data[5] = 0x00;
	TEST_ASSERT_EQUAL_UINT8(0, readFollowDistance(f));
}

void test_read_follow_distance_max_value()
{
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	f.data[5] = 0xFF;
	TEST_ASSERT_EQUAL_UINT8(7, readFollowDistance(f));
}

void test_read_follow_distance_returns_zero_when_dlc_too_short()
{
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST, 5);
	f.data[5] = 0b01000000;
	TEST_ASSERT_EQUAL_UINT8(0, readFollowDistance(f));
}

/* ── isFSDSelectedInUI ───────────────────────────────────────────────────── */

void test_is_fsd_selected_bit_set()
{
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[4] = 0x40;
	TEST_ASSERT_TRUE(isFSDSelectedInUI(f));
}

void test_is_fsd_selected_bit_clear()
{
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[4] = 0x00;
	TEST_ASSERT_FALSE(isFSDSelectedInUI(f));
}

void test_is_fsd_selected_other_bits_set()
{
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[4] = 0x7F;
	TEST_ASSERT_TRUE(isFSDSelectedInUI(f));
}

void test_is_fsd_selected_returns_false_when_dlc_too_short()
{
	Frame f = makeFrame(CAN_ID_FSD_MUX, 4);
	f.data[4] = 0x40;
	TEST_ASSERT_FALSE(isFSDSelectedInUI(f));
}

void test_is_fsd_selected_exact_dlc_5()
{
	Frame f = makeFrame(CAN_ID_FSD_MUX, 5);
	f.data[4] = 0x40;
	TEST_ASSERT_TRUE(isFSDSelectedInUI(f));
}

/* ── driveChecksum (defined inline in ids.h) ─────────────────────────────── */

void test_drive_checksum_from_ids()
{
	uint8_t data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
	uint8_t crc = driveChecksum(data, 8);
	uint8_t expected = 0x01 + 0x02 + 0x03 + 0x04 + 0x05 + 0x06 + 0x07;
	TEST_ASSERT_EQUAL_UINT8(expected, crc);
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_read_follow_distance_value_1);
	RUN_TEST(test_read_follow_distance_value_2);
	RUN_TEST(test_read_follow_distance_value_3);
	RUN_TEST(test_read_follow_distance_value_4);
	RUN_TEST(test_read_follow_distance_value_0);
	RUN_TEST(test_read_follow_distance_max_value);
	RUN_TEST(test_read_follow_distance_returns_zero_when_dlc_too_short);

	RUN_TEST(test_is_fsd_selected_bit_set);
	RUN_TEST(test_is_fsd_selected_bit_clear);
	RUN_TEST(test_is_fsd_selected_other_bits_set);
	RUN_TEST(test_is_fsd_selected_returns_false_when_dlc_too_short);
	RUN_TEST(test_is_fsd_selected_exact_dlc_5);

	RUN_TEST(test_drive_checksum_from_ids);

	return UNITY_END();
}
