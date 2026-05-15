/**
 * @file firmware/test/test_native_frame_readers/test_frame_readers.cpp
 * @brief Unit tests for CAN frame decoders in frame_readers.h
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <unity.h>
#include <cstring>

#include "vehicle/can/handler/frame_readers.h"
#include "support/helpers.h"

void setUp() {}
void tearDown() {}

/* ── readDASAutopilotStatus ──────────────────────────────────────────────── */

void test_read_das_autopilot_status_zero()
{
	Frame f = makeFrame(0x399, 8);
	f.data[0] = 0x00;
	TEST_ASSERT_EQUAL_UINT8(0, readDASAutopilotStatus(f));
}

void test_read_das_autopilot_status_max()
{
	Frame f = makeFrame(0x399, 8);
	f.data[0] = 0x0F;
	TEST_ASSERT_EQUAL_UINT8(15, readDASAutopilotStatus(f));
}

void test_read_das_autopilot_status_masks_upper_bits()
{
	Frame f = makeFrame(0x399, 8);
	f.data[0] = 0xFF;
	TEST_ASSERT_EQUAL_UINT8(15, readDASAutopilotStatus(f));
}

void test_read_das_autopilot_status_specific_value()
{
	Frame f = makeFrame(0x399, 8);
	f.data[0] = 0x35;
	TEST_ASSERT_EQUAL_UINT8(5, readDASAutopilotStatus(f));
}

void test_read_das_autopilot_status_returns_zero_when_dlc_zero()
{
	Frame f = makeFrame(0x399, 0);
	f.data[0] = 0x0F;
	TEST_ASSERT_EQUAL_UINT8(0, readDASAutopilotStatus(f));
}

/* ── readDASAutopilotState ───────────────────────────────────────────────── */

void test_read_das_autopilot_state_zero()
{
	Frame f = makeFrame(0x39B, 8);
	f.data[1] = 0x00;
	TEST_ASSERT_EQUAL_UINT8(0, readDASAutopilotState(f));
}

void test_read_das_autopilot_state_max()
{
	Frame f = makeFrame(0x39B, 8);
	f.data[1] = 0xF0;
	TEST_ASSERT_EQUAL_UINT8(15, readDASAutopilotState(f));
}

void test_read_das_autopilot_state_masks_lower_bits()
{
	Frame f = makeFrame(0x39B, 8);
	f.data[1] = 0xFF;
	TEST_ASSERT_EQUAL_UINT8(15, readDASAutopilotState(f));
}

void test_read_das_autopilot_state_returns_zero_when_dlc_less_than_2()
{
	Frame f = makeFrame(0x39B, 1);
	f.data[1] = 0xF0;
	TEST_ASSERT_EQUAL_UINT8(0, readDASAutopilotState(f));
}

void test_read_das_autopilot_state_exact_dlc_2()
{
	Frame f = makeFrame(0x39B, 2);
	f.data[1] = 0x50;
	TEST_ASSERT_EQUAL_UINT8(5, readDASAutopilotState(f));
}

/* ── isDASAutopilotActive ────────────────────────────────────────────────── */

void test_das_autopilot_active_status_3()
{
	TEST_ASSERT_TRUE(isDASAutopilotActive(3));
}

void test_das_autopilot_active_status_4()
{
	TEST_ASSERT_TRUE(isDASAutopilotActive(4));
}

void test_das_autopilot_active_status_5()
{
	TEST_ASSERT_TRUE(isDASAutopilotActive(5));
}

void test_das_autopilot_not_active_status_0()
{
	TEST_ASSERT_FALSE(isDASAutopilotActive(0));
}

void test_das_autopilot_not_active_status_1()
{
	TEST_ASSERT_FALSE(isDASAutopilotActive(1));
}

void test_das_autopilot_not_active_status_2()
{
	TEST_ASSERT_FALSE(isDASAutopilotActive(2));
}

void test_das_autopilot_not_active_status_6()
{
	TEST_ASSERT_FALSE(isDASAutopilotActive(6));
}

void test_das_autopilot_not_active_status_255()
{
	TEST_ASSERT_FALSE(isDASAutopilotActive(255));
}

/* ── readGtwAutopilotTier ────────────────────────────────────────────────── */

void test_read_gtw_autopilot_tier_zero()
{
	Frame f = makeFrame(0x7FF, 8);
	f.data[0] = 0x02;
	f.data[5] = 0x00;
	TEST_ASSERT_EQUAL_INT8(0, readGtwAutopilotTier(f));
}

void test_read_gtw_autopilot_tier_self_driving()
{
	Frame f = makeFrame(0x7FF, 8);
	f.data[0] = 0x02;
	f.data[5] = 0x0C;
	TEST_ASSERT_EQUAL_INT8(3, readGtwAutopilotTier(f));
}

void test_read_gtw_autopilot_tier_basic()
{
	Frame f = makeFrame(0x7FF, 8);
	f.data[0] = 0x02;
	f.data[5] = 0x10;
	TEST_ASSERT_EQUAL_INT8(4, readGtwAutopilotTier(f));
}

void test_read_gtw_autopilot_tier_masks_other_bits()
{
	Frame f = makeFrame(0x7FF, 8);
	f.data[0] = 0x02;
	f.data[5] = 0xFF;
	TEST_ASSERT_EQUAL_INT8(7, readGtwAutopilotTier(f));
}

void test_read_gtw_autopilot_tier_returns_neg1_when_dlc_too_short()
{
	Frame f = makeFrame(0x7FF, 5);
	f.data[0] = 0x02;
	f.data[5] = 0x18;
	TEST_ASSERT_EQUAL_INT8(-1, readGtwAutopilotTier(f));
}

void test_read_gtw_autopilot_tier_returns_neg1_when_not_mux_2()
{
	Frame f = makeFrame(0x7FF, 8);
	f.data[0] = 0x00;
	f.data[5] = 0x0C;
	TEST_ASSERT_EQUAL_INT8(-1, readGtwAutopilotTier(f));
}

void test_read_gtw_autopilot_tier_returns_neg1_when_mux_1()
{
	Frame f = makeFrame(0x7FF, 8);
	f.data[0] = 0x01;
	f.data[5] = 0x0C;
	TEST_ASSERT_EQUAL_INT8(-1, readGtwAutopilotTier(f));
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_read_das_autopilot_status_zero);
	RUN_TEST(test_read_das_autopilot_status_max);
	RUN_TEST(test_read_das_autopilot_status_masks_upper_bits);
	RUN_TEST(test_read_das_autopilot_status_specific_value);
	RUN_TEST(test_read_das_autopilot_status_returns_zero_when_dlc_zero);

	RUN_TEST(test_read_das_autopilot_state_zero);
	RUN_TEST(test_read_das_autopilot_state_max);
	RUN_TEST(test_read_das_autopilot_state_masks_lower_bits);
	RUN_TEST(test_read_das_autopilot_state_returns_zero_when_dlc_less_than_2);
	RUN_TEST(test_read_das_autopilot_state_exact_dlc_2);

	RUN_TEST(test_das_autopilot_active_status_3);
	RUN_TEST(test_das_autopilot_active_status_4);
	RUN_TEST(test_das_autopilot_active_status_5);
	RUN_TEST(test_das_autopilot_not_active_status_0);
	RUN_TEST(test_das_autopilot_not_active_status_1);
	RUN_TEST(test_das_autopilot_not_active_status_2);
	RUN_TEST(test_das_autopilot_not_active_status_6);
	RUN_TEST(test_das_autopilot_not_active_status_255);

	RUN_TEST(test_read_gtw_autopilot_tier_zero);
	RUN_TEST(test_read_gtw_autopilot_tier_self_driving);
	RUN_TEST(test_read_gtw_autopilot_tier_basic);
	RUN_TEST(test_read_gtw_autopilot_tier_masks_other_bits);
	RUN_TEST(test_read_gtw_autopilot_tier_returns_neg1_when_dlc_too_short);
	RUN_TEST(test_read_gtw_autopilot_tier_returns_neg1_when_not_mux_2);
	RUN_TEST(test_read_gtw_autopilot_tier_returns_neg1_when_mux_1);

	return UNITY_END();
}
