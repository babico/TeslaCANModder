/**
 * @file firmware/test/test_native_ring_buffer/test_ring_buffer.cpp
 * @brief Unit tests for lock-free CAN frame ring buffer
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <unity.h>
#include <cstring>

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

#include "core/types.h"
#include "core/can/ring_buffer.h"

void setUp()
{
	canRingBuffer.writeIdx = 0;
	canRingBuffer.seqCounter = 0;
	memset(canRingBuffer.entries, 0, sizeof(canRingBuffer.entries));
}
void tearDown() {}

/* ── ringPush / ringHasData ──────────────────────────────────────────────── */

void test_ring_push_makes_data_available()
{
	RingConsumer c;
	Frame f = {};
	f.id = 0x100;
	f.dlc = 8;
	ringPush(f, 0, 1000);
	TEST_ASSERT_TRUE(ringHasData(c));
}

void test_ring_push_increments_seq()
{
	Frame f = {};
	ringPush(f, 0, 1000);
	TEST_ASSERT_EQUAL_UINT32(1, ringTotalFrames());
}

void test_ring_push_multiple()
{
	Frame f = {};
	ringPush(f, 0, 1000);
	ringPush(f, 1, 1001);
	ringPush(f, 0, 1002);
	TEST_ASSERT_EQUAL_UINT32(3, ringTotalFrames());
}

/* ── ringPeek / ringAdvance ──────────────────────────────────────────────── */

void test_ring_peek_returns_entry()
{
	RingConsumer c;
	Frame f = {};
	f.id = 0x3FD;
	f.dlc = 8;
	f.data[0] = 0x42;
	ringPush(f, 1, 5000);
	const RingEntry *e = ringPeek(c);
	TEST_ASSERT_NOT_NULL(e);
	TEST_ASSERT_EQUAL_HEX32(0x3FD, e->frame.id);
	TEST_ASSERT_EQUAL_HEX8(0x42, e->frame.data[0]);
	TEST_ASSERT_EQUAL_UINT8(1, e->bus);
	TEST_ASSERT_EQUAL(5000, e->timestamp);
}

void test_ring_peek_returns_null_when_empty()
{
	RingConsumer c;
	TEST_ASSERT_NULL(ringPeek(c));
}

void test_ring_advance_moves_past_entry()
{
	RingConsumer c;
	Frame f = {};
	f.id = 0x100;
	ringPush(f, 0, 1000);
	ringPeek(c);
	ringAdvance(c);
	TEST_ASSERT_FALSE(ringHasData(c));
}

void test_ring_peek_multiple_entries()
{
	RingConsumer c;
	Frame f1 = {}, f2 = {};
	f1.id = 0x100;
	f2.id = 0x200;
	ringPush(f1, 0, 1000);
	ringPush(f2, 1, 1001);

	const RingEntry *e1 = ringPeek(c);
	TEST_ASSERT_EQUAL_HEX32(0x100, e1->frame.id);
	ringAdvance(c);

	const RingEntry *e2 = ringPeek(c);
	TEST_ASSERT_EQUAL_HEX32(0x200, e2->frame.id);
	ringAdvance(c);

	TEST_ASSERT_NULL(ringPeek(c));
}

/* ── ringAvailable ───────────────────────────────────────────────────────── */

void test_ring_available_zero_when_empty()
{
	RingConsumer c;
	TEST_ASSERT_EQUAL_UINT32(0, ringAvailable(c));
}

void test_ring_available_matches_push_count()
{
	RingConsumer c;
	Frame f = {};
	ringPush(f, 0, 1000);
	ringPush(f, 0, 1001);
	ringPush(f, 0, 1002);
	TEST_ASSERT_EQUAL_UINT32(3, ringAvailable(c));
}

void test_ring_available_decreases_after_advance()
{
	RingConsumer c;
	Frame f = {};
	ringPush(f, 0, 1000);
	ringPush(f, 0, 1001);
	TEST_ASSERT_EQUAL_UINT32(2, ringAvailable(c));
	ringPeek(c);
	ringAdvance(c);
	TEST_ASSERT_EQUAL_UINT32(1, ringAvailable(c));
}

/* ── ringReset ───────────────────────────────────────────────────────────── */

void test_ring_reset_skips_all_data()
{
	RingConsumer c;
	Frame f = {};
	ringPush(f, 0, 1000);
	ringPush(f, 0, 1001);
	ringReset(c);
	TEST_ASSERT_FALSE(ringHasData(c));
	TEST_ASSERT_EQUAL_UINT32(0, ringAvailable(c));
}

void test_ring_reset_clears_dropped()
{
	RingConsumer c;
	c.dropped = 99;
	ringReset(c);
	TEST_ASSERT_EQUAL_UINT32(0, c.dropped);
}

/* ── Overflow detection ──────────────────────────────────────────────────── */

void test_ring_overflow_detects_lapped_consumer()
{
	RingConsumer c;
	Frame f = {};
	for (int i = 0; i < RING_BUF_SIZE + 10; i++)
		ringPush(f, 0, 1000 + i);

	const RingEntry *e = ringPeek(c);
	TEST_ASSERT_NOT_NULL(e);
	TEST_ASSERT_EQUAL_UINT32(10, c.dropped);
}

void test_ring_available_capped_at_buffer_size()
{
	RingConsumer c;
	Frame f = {};
	for (int i = 0; i < RING_BUF_SIZE + 50; i++)
		ringPush(f, 0, 1000 + i);
	TEST_ASSERT_EQUAL_UINT32(RING_BUF_SIZE, ringAvailable(c));
}

/* ── Multiple independent consumers ──────────────────────────────────────── */

void test_ring_multiple_consumers_independent()
{
	RingConsumer c1, c2;
	Frame f = {};
	f.id = 0x100;
	ringPush(f, 0, 1000);

	ringPeek(c1);
	ringAdvance(c1);

	TEST_ASSERT_FALSE(ringHasData(c1));
	TEST_ASSERT_TRUE(ringHasData(c2));
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_ring_push_makes_data_available);
	RUN_TEST(test_ring_push_increments_seq);
	RUN_TEST(test_ring_push_multiple);
	RUN_TEST(test_ring_peek_returns_entry);
	RUN_TEST(test_ring_peek_returns_null_when_empty);
	RUN_TEST(test_ring_advance_moves_past_entry);
	RUN_TEST(test_ring_peek_multiple_entries);
	RUN_TEST(test_ring_available_zero_when_empty);
	RUN_TEST(test_ring_available_matches_push_count);
	RUN_TEST(test_ring_available_decreases_after_advance);
	RUN_TEST(test_ring_reset_skips_all_data);
	RUN_TEST(test_ring_reset_clears_dropped);
	RUN_TEST(test_ring_overflow_detects_lapped_consumer);
	RUN_TEST(test_ring_available_capped_at_buffer_size);
	RUN_TEST(test_ring_multiple_consumers_independent);

	return UNITY_END();
}
