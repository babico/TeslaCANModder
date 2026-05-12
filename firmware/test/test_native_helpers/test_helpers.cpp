/**
 * @file firmware/test/test_native_helpers/test_helpers.cpp
 * @brief Unit tests for utility helper functions (setBit, readMuxID, isFSDSelectedInUI, etc.)
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <unity.h>
#include <cstring>

class __FlashStringHelper;

#include "core/types.h"
#include "vehicle/can/ids.h"

void sendLog(const char *) {}
void sendLog(const __FlashStringHelper *) {}
void saveSettings(const State &) {}
void resetHandlerLogFlags() {}
void applyFilters(State &) {}
void driverSend(const Frame &, uint8_t) {}

#include "feature/profile.h"
#include "handler/frame_readers.h"

/** @brief Set up test fixtures before each test */
void setUp() {}

/** @brief Tear down test fixtures after each test */
void tearDown() {}


/** @brief Verify setBit sets bit 0 in byte 0 */
void test_setBit_sets_bit0_of_byte0()
{
	Frame f = {};
	f.dlc = 8;
	setBit(f, 0, true);
	TEST_ASSERT_EQUAL_HEX8(0x01, f.data[0]);
}

/** @brief Verify setBit sets bit 7 (MSB) in byte 0 */
void test_setBit_sets_bit7_of_byte0()
{
	Frame f = {};
	f.dlc = 8;
	setBit(f, 7, true);
	TEST_ASSERT_EQUAL_HEX8(0x80, f.data[0]);
}

/** @brief Verify setBit sets bit 46 which maps to byte 5 */
void test_setBit_sets_bit_in_byte5()
{
	Frame f = {};
	f.dlc = 8;
	setBit(f, 46, true); // bit 46 = byte 5, bit 6
	TEST_ASSERT_EQUAL_HEX8(0x40, f.data[5]);
}

/** @brief Verify setBit sets bit 60 which maps to byte 7 */
void test_setBit_sets_bit_in_byte7()
{
	Frame f = {};
	f.dlc = 8;
	setBit(f, 60, true); // bit 60 = byte 7, bit 4
	TEST_ASSERT_EQUAL_HEX8(0x10, f.data[7]);
}

/** @brief Verify setBit can clear a previously set bit */
void test_setBit_clears_bit()
{
	Frame f = {};
	f.dlc = 8;
	f.data[2] = 0xFF;
	setBit(f, 19, false); // bit 19 = byte 2, bit 3
	TEST_ASSERT_EQUAL_HEX8(0xF7, f.data[2]);
}

/** @brief Verify setBit does not corrupt adjacent bytes */
void test_setBit_does_not_affect_other_bytes()
{
	Frame f = {};
	f.dlc = 8;
	f.data[0] = 0xAA;
	f.data[1] = 0xBB;
	setBit(f, 8, true); // bit 8 = byte 1, bit 0 — should only OR into data[1]
	TEST_ASSERT_EQUAL_HEX8(0xAA, f.data[0]);
	TEST_ASSERT_EQUAL_HEX8(0xBB, f.data[1]);
}

/** @brief Verify setBit safely ignores a negative bit index */
void test_setBit_ignores_negative_bit_index()
{
	Frame f = {};
	f.dlc = 8;
	setBit(f, -1, true);
	TEST_ASSERT_EQUAL_HEX8(0x00, f.data[0]);
}

/** @brief Verify setBit safely ignores a bit index beyond 63 */
void test_setBit_ignores_out_of_range_bit_index()
{
	Frame f = {};
	f.dlc = 8;
	setBit(f, 64, true);
	TEST_ASSERT_EQUAL_HEX8(0x00, f.data[7]);
}

/** @brief Verify setBit ignores bits that exceed the frame DLC */
void test_setBit_ignores_bits_beyond_frame_dlc()
{
	Frame f = {};
	f.dlc = 2; // only bytes 0-1 are valid
	setBit(f, 19, true); // byte 2 is out of DLC range
	TEST_ASSERT_EQUAL_HEX8(0x00, f.data[0]);
	TEST_ASSERT_EQUAL_HEX8(0x00, f.data[1]);
}


/** @brief Verify readMuxID extracts the lower 3 bits of data[0] */
void test_readMuxID_extracts_lower_3_bits()
{
	Frame f = {};
	f.dlc = 1;
	f.data[0] = 0x05;
	TEST_ASSERT_EQUAL_UINT8(5, readMuxID(f));
}

/** @brief Verify readMuxID masks off upper bits correctly */
void test_readMuxID_masks_upper_bits()
{
	Frame f = {};
	f.dlc = 1;
	f.data[0] = 0xFA; // lower 3 bits = 010 = 2
	TEST_ASSERT_EQUAL_UINT8(2, readMuxID(f));
}

/** @brief Verify readMuxID returns 0 for a zeroed frame */
void test_readMuxID_zero()
{
	Frame f = {};
	f.dlc = 1;
	f.data[0] = 0x00;
	TEST_ASSERT_EQUAL_UINT8(0, readMuxID(f));
}

/** @brief Verify readMuxID returns max value 7 (0b111) */
void test_readMuxID_max_value()
{
	Frame f = {};
	f.dlc = 1;
	f.data[0] = 0x07;
	TEST_ASSERT_EQUAL_UINT8(7, readMuxID(f));
}


/** @brief Verify isFSDSelectedInUI returns true when bit 6 of data[4] is set */
void test_isFSDSelectedInUI_true_when_bit6_set()
{
	Frame f = {};
	f.dlc = 5;
	f.data[4] = 0x40; // bit 6 set
	TEST_ASSERT_TRUE(isFSDSelectedInUI(f));
}

/** @brief Verify isFSDSelectedInUI returns false when bit 6 of data[4] is clear */
void test_isFSDSelectedInUI_false_when_bit6_clear()
{
	Frame f = {};
	f.dlc = 5;
	f.data[4] = 0x00;
	TEST_ASSERT_FALSE(isFSDSelectedInUI(f));
}

/** @brief Verify isFSDSelectedInUI ignores all bits except bit 6 */
void test_isFSDSelectedInUI_ignores_other_bits()
{
	Frame f = {};
	f.dlc = 5;
	f.data[4] = 0xBF; // all bits set except bit 6
	TEST_ASSERT_FALSE(isFSDSelectedInUI(f));
}

/** @brief Verify isFSDSelectedInUI returns true even with other bits set alongside bit 6 */
void test_isFSDSelectedInUI_true_with_other_bits()
{
	Frame f = {};
	f.dlc = 5;
	f.data[4] = 0xFF;
	TEST_ASSERT_TRUE(isFSDSelectedInUI(f));
}


/** @brief Verify readGtwAutopilotTier extracts bits [4:2] of data[5] on mux 2 */
void test_readGtwAutopilotTier_reads_mux2_bits_4_to_2()
{
	Frame f = {};
	f.dlc = 8;
	f.data[0] = 0x02; // mux ID = 2
	f.data[5] = (3 << 2); // tier value 3 in bits [4:2]
	TEST_ASSERT_EQUAL_INT8(3, readGtwAutopilotTier(f));
}

/** @brief Verify readGtwAutopilotTier returns -1 for non-mux-2 frames */
void test_readGtwAutopilotTier_ignores_non_mux2()
{
	Frame f = {};
	f.dlc = 8;
	f.data[0] = 0x01; // mux ID = 1, not 2
	f.data[5] = (4 << 2);
	TEST_ASSERT_EQUAL_INT8(-1, readGtwAutopilotTier(f));
}

/** @brief Verify readGtwAutopilotTier returns -1 when DLC is too short */
void test_readGtwAutopilotTier_requires_dlc_6()
{
	Frame f = {};
	f.dlc = 5; // needs at least 6 bytes
	f.data[0] = 0x02;
	TEST_ASSERT_EQUAL_INT8(-1, readGtwAutopilotTier(f));
}


/** @brief Verify setSpeedProfileV12V13 clears profile bits to set profile 0 */
void test_setSpeedProfileV12V13_sets_profile_0()
{
	Frame f = {};
	f.dlc = 7;
	f.data[6] = 0xFF;
	setSpeedProfileV12V13(f, 0); // clears bits [2:1] of data[6]
	TEST_ASSERT_EQUAL_HEX8(0xF9, f.data[6]);
}

/** @brief Verify setSpeedProfileV12V13 sets profile 1 in bits [2:1] */
void test_setSpeedProfileV12V13_sets_profile_1()
{
	Frame f = {};
	f.dlc = 7;
	f.data[6] = 0x00;
	setSpeedProfileV12V13(f, 1);
	TEST_ASSERT_EQUAL_HEX8(0x02, f.data[6]);
}

/** @brief Verify setSpeedProfileV12V13 sets profile 2 in bits [2:1] */
void test_setSpeedProfileV12V13_sets_profile_2()
{
	Frame f = {};
	f.dlc = 7;
	f.data[6] = 0x00;
	setSpeedProfileV12V13(f, 2);
	TEST_ASSERT_EQUAL_HEX8(0x04, f.data[6]);
}

/** @brief Verify setSpeedProfileV12V13 preserves bits outside the profile field */
void test_setSpeedProfileV12V13_preserves_other_bits()
{
	Frame f = {};
	f.dlc = 7;
	f.data[6] = 0xF9; // bits [2:1] already clear
	setSpeedProfileV12V13(f, 1);
	TEST_ASSERT_EQUAL_HEX8(0xFB, f.data[6]);
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_setBit_sets_bit0_of_byte0);
	RUN_TEST(test_setBit_sets_bit7_of_byte0);
	RUN_TEST(test_setBit_sets_bit_in_byte5);
	RUN_TEST(test_setBit_sets_bit_in_byte7);
	RUN_TEST(test_setBit_clears_bit);
	RUN_TEST(test_setBit_does_not_affect_other_bytes);
	RUN_TEST(test_setBit_ignores_negative_bit_index);
	RUN_TEST(test_setBit_ignores_out_of_range_bit_index);
	RUN_TEST(test_setBit_ignores_bits_beyond_frame_dlc);

	RUN_TEST(test_readMuxID_extracts_lower_3_bits);
	RUN_TEST(test_readMuxID_masks_upper_bits);
	RUN_TEST(test_readMuxID_zero);
	RUN_TEST(test_readMuxID_max_value);

	RUN_TEST(test_isFSDSelectedInUI_true_when_bit6_set);
	RUN_TEST(test_isFSDSelectedInUI_false_when_bit6_clear);
	RUN_TEST(test_isFSDSelectedInUI_ignores_other_bits);
	RUN_TEST(test_isFSDSelectedInUI_true_with_other_bits);

	RUN_TEST(test_readGtwAutopilotTier_reads_mux2_bits_4_to_2);
	RUN_TEST(test_readGtwAutopilotTier_ignores_non_mux2);
	RUN_TEST(test_readGtwAutopilotTier_requires_dlc_6);

	RUN_TEST(test_setSpeedProfileV12V13_sets_profile_0);
	RUN_TEST(test_setSpeedProfileV12V13_sets_profile_1);
	RUN_TEST(test_setSpeedProfileV12V13_sets_profile_2);
	RUN_TEST(test_setSpeedProfileV12V13_preserves_other_bits);

	return UNITY_END();
}
