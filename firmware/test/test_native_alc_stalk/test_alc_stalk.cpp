/**
 * @file firmware/test/test_native_alc_stalk/test_alc_stalk.cpp
 * @brief ALC 0x249 stalk frame shape + CRC replay against the offline capture corpus
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 *
 * Capture evidence (read-only oracle, NOT vendored into shipping code):
 *   legacy/uhi22-tesla-crc/249_sortedFile.txt — 870 accepted 4-byte 0x249 frames.
 *   Provenance: legacy/uhi22-tesla-crc/readme.md (Ref2, openinverter.org p81331).
 *   Analysis: docs/legacy/uhi22-tesla-crc.md §2, §7, §8, §11.
 *
 * Derived rule (re-derived independently from the corpus, no legacy code copied):
 *   data[0]      = CRC-8/OPENSAFETY({data[1] & 0xF0, data[2], data[3], 0x00})
 *                  XOR magic249[data[1] & 0x0F]
 *   data[1][3:0] = alive counter (excluded from the CRC input window)
 *   data[2][3:0] = turn-stalk enum
 *   data[3]      = 0x00 in every captured frame
 *   magic249[16] = the 16 zero-payload frames (counter 0..15) at corpus lines 1-16.
 *
 * The vectors below are literal lines copied from the capture corpus as test data.
 * Real-vehicle acceptance of our injected frame remains UNVERIFIED (no bench capture).
 */

#include <unity.h>
#include <cstring>

#include "vehicle/can/feature/fsd/auto_lane_change.h"
#include "stubs.h"

/** @brief Test fixture setup — no per-test state required */
void setUp() {}

/** @brief Test fixture teardown — no cleanup required */
void tearDown() {}

/** @brief CRC byte, counter, stalk enum and tail byte from one captured 0x249 frame */
struct CorpusVector
{
	uint8_t crc;
	uint8_t b1; // [3:0] counter, [7:4] high-beam/wash-wipe nibble
	uint8_t b2; // [3:0] turn-stalk enum
	uint8_t b3; // always 0x00 in the corpus
};

/**
 * @brief Literal corpus vectors covering: all 16 counters at zero payload,
 * every observed stalk enum (1/2/4/6/8), every observed b1 high nibble,
 * and stalk+high-nibble combinations at nonzero counters.
 */
static const CorpusVector kCorpus[] = {
	{0x9B, 0x00, 0x00, 0x00},
	{0xE8, 0x01, 0x00, 0x00},
	{0x2A, 0x02, 0x00, 0x00},
	{0xD3, 0x03, 0x00, 0x00},
	{0xD3, 0x04, 0x00, 0x00},
	{0x83, 0x05, 0x00, 0x00},
	{0x4C, 0x06, 0x00, 0x00},
	{0x5E, 0x07, 0x00, 0x00},
	{0x3F, 0x08, 0x00, 0x00},
	{0x5E, 0x09, 0x00, 0x00},
	{0xE2, 0x0A, 0x00, 0x00},
	{0x28, 0x0B, 0x00, 0x00},
	{0x3A, 0x0C, 0x00, 0x00},
	{0x13, 0x0D, 0x00, 0x00},
	{0xAF, 0x0E, 0x00, 0x00},
	{0xCE, 0x0F, 0x00, 0x00},
	{0x87, 0x00, 0x02, 0x00},
	{0x42, 0x07, 0x02, 0x00},
	{0xA3, 0x00, 0x04, 0x00},
	{0x66, 0x07, 0x04, 0x00},
	{0xBF, 0x00, 0x06, 0x00},
	{0x7A, 0x07, 0x06, 0x00},
	{0xEB, 0x00, 0x08, 0x00},
	{0x2E, 0x07, 0x08, 0x00},
	{0xED, 0x10, 0x00, 0x00},
	{0x77, 0x20, 0x00, 0x00},
	{0x6C, 0x40, 0x00, 0x00},
	{0x1A, 0x50, 0x00, 0x00},
	{0x80, 0x60, 0x00, 0x00},
	{0x5A, 0x80, 0x00, 0x00},
	{0x2C, 0x90, 0x00, 0x00},
	{0xB6, 0xA0, 0x00, 0x00},
	{0x31, 0x23, 0x01, 0x00},
	{0x41, 0x5F, 0x01, 0x00},
	{0x6A, 0x93, 0x01, 0x00},
	{0x77, 0x9F, 0x01, 0x00},
	{0xB9, 0x13, 0x02, 0x00},
	{0xA4, 0x1F, 0x02, 0x00},
	{0x23, 0x23, 0x02, 0x00},
	{0x3E, 0x2F, 0x02, 0x00},
};

/** @brief Magic table re-derived from the 16 zero-payload corpus lines (1-16). */
static const uint8_t kDerivedMagic249[16] = {
	0x9B, 0xE8, 0x2A, 0xD3, 0xD3, 0x83, 0x4C, 0x5E,
	0x3F, 0x5E, 0xE2, 0x28, 0x3A, 0x13, 0xAF, 0xCE};

/** @brief Shipped magic table must equal the corpus-derived table. */
void test_alc_magic_table_matches_corpus()
{
	for (uint8_t c = 0; c < 16; c++)
		TEST_ASSERT_EQUAL_HEX8_MESSAGE(kDerivedMagic249[c], MAGIC_0x249[c], "magic249 slot mismatch vs corpus");
}

/** @brief Replay literal corpus vectors through the shipped CRC helper. */
void test_alc_corpus_crc_replay()
{
	for (size_t i = 0; i < sizeof(kCorpus) / sizeof(kCorpus[0]); i++)
	{
		const CorpusVector &v = kCorpus[i];
		uint8_t payload[3] = {static_cast<uint8_t>(v.b1 & 0xF0), v.b2, v.b3};
		uint8_t crc = teslaCrc8(payload, 3, v.b1 & 0x0F, MAGIC_0x249);
		TEST_ASSERT_EQUAL_HEX8_MESSAGE(v.crc, crc, "0x249 CRC disagrees with capture corpus");
	}
}

/** @brief buildStalkFrame emits the captured DLC-4 shape with the stalk enum in byte 2. */
void test_alc_stalk_frame_shape()
{
	Frame f;
	memset(&f, 0, sizeof(f));

	buildStalkFrame(f, true); // left

	TEST_ASSERT_EQUAL_HEX16(CAN_ID_DI_STEER, f.id);
	TEST_ASSERT_EQUAL_UINT8(4, f.dlc);
	TEST_ASSERT_EQUAL_HEX8(0x00, f.data[3]); // tail byte always 0x00 in corpus
	TEST_ASSERT_EQUAL_HEX8(0x00, f.data[1] & 0xF0); // idle high-beam/wash-wipe nibble

	// Stalk enum lives in byte 2 low nibble, never the old byte-1 bits [7:6].
	TEST_ASSERT_TRUE(f.data[2] == 0x08 || f.data[2] == 0x04);
	TEST_ASSERT_EQUAL_HEX8(0x00, f.data[2] & 0xF0);

	// CRC is computed over the counter-masked byte 1 + byte 2 + byte 3 + virtual 0.
	uint8_t payload[3] = {static_cast<uint8_t>(f.data[1] & 0xF0), f.data[2], f.data[3]};
	uint8_t expected = teslaCrc8(payload, 3, f.data[1] & 0x0F, MAGIC_0x249);
	TEST_ASSERT_EQUAL_HEX8(expected, f.data[0]);
}

/** @brief Left and right build distinct, corpus-observed stalk enum values. */
void test_alc_stalk_left_right_distinct()
{
	Frame left;
	Frame right;
	memset(&left, 0, sizeof(left));
	memset(&right, 0, sizeof(right));

	buildStalkFrame(left, true);
	buildStalkFrame(right, false);

	TEST_ASSERT_EQUAL_HEX8(0x08, left.data[2]);
	TEST_ASSERT_EQUAL_HEX8(0x04, right.data[2]);
	TEST_ASSERT_TRUE(left.data[2] != right.data[2]);
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_alc_magic_table_matches_corpus);
	RUN_TEST(test_alc_corpus_crc_replay);
	RUN_TEST(test_alc_stalk_frame_shape);
	RUN_TEST(test_alc_stalk_left_right_distinct);
	return UNITY_END();
}
