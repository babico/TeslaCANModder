/** @file firmware/test/test_native_ring_buffer/test_ring_buffer.cpp
 *  @brief Unit tests for generic ring buffer data structure
 *  @author Tesla CAN Mod Contributors
 *  @license GPL-3.0
 */

#include <unity.h>
#include <cstring>

class __FlashStringHelper;
#define F(s) (reinterpret_cast<const __FlashStringHelper *>(s))

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

#include "core/types.h"
#include "core/can/ring_buffer.h"
#include "core/log/ring.h"

void setUp()
{
	canRingBuffer.writeIdx = 0;
	canRingBuffer.seqCounter = 0;
}
void tearDown() {}


void test_push_increments_head()
{
	Frame f = {};
	f.id = 0x100;
	f.dlc = 8;
	ringPush(f, 0, 1000);
	TEST_ASSERT_EQUAL(1, canRingBuffer.writeIdx);
}

void test_push_stores_frame()
{
	Frame f = {};
	f.id = 0x200;
	f.dlc = 4;
	f.data[0] = 0xAA;
	ringPush(f, 1, 2000);
	TEST_ASSERT_EQUAL(0x200, canRingBuffer.entries[0].frame.id);
	TEST_ASSERT_EQUAL(4, canRingBuffer.entries[0].frame.dlc);
	TEST_ASSERT_EQUAL(0xAA, canRingBuffer.entries[0].frame.data[0]);
	TEST_ASSERT_EQUAL(1, canRingBuffer.entries[0].bus);
	TEST_ASSERT_EQUAL(2000, canRingBuffer.entries[0].timestamp);
}

void test_push_wraps_at_capacity()
{
	for (int i = 0; i < RING_BUF_SIZE + 5; i++)
	{
		Frame f = {};
		f.id = i;
		ringPush(f, 0, i * 10);
	}
	TEST_ASSERT_EQUAL(RING_BUF_SIZE + 5, canRingBuffer.writeIdx);
	TEST_ASSERT_EQUAL(RING_BUF_SIZE + 4, canRingBuffer.entries[4].frame.id);
}

void test_consumer_reads_pushed_frame()
{
	RingConsumer c = {};
	ringReset(c);

	Frame f = {};
	f.id = 0x300;
	ringPush(f, 2, 3000);

	TEST_ASSERT_TRUE(ringHasData(c));
	TEST_ASSERT_EQUAL(1, ringAvailable(c));

	const RingEntry *e = ringPeek(c);
	TEST_ASSERT_NOT_NULL(e);
	TEST_ASSERT_EQUAL(0x300, e->frame.id);
	TEST_ASSERT_EQUAL(2, e->bus);

	ringAdvance(c);
	TEST_ASSERT_FALSE(ringHasData(c));
}

void test_consumer_detects_overflow()
{
	RingConsumer c = {};
	ringReset(c);

	for (int i = 0; i < RING_BUF_SIZE + 10; i++)
	{
		Frame f = {};
		f.id = i;
		ringPush(f, 0, i);
	}

	const RingEntry *e = ringPeek(c);
	TEST_ASSERT_TRUE(c.dropped > 0);
	(void)e;
}

void test_multiple_consumers_independent()
{
	RingConsumer c1 = {};
	RingConsumer c2 = {};
	ringReset(c1);
	ringReset(c2);

	Frame f = {};
	f.id = 0x400;
	ringPush(f, 0, 4000);

	TEST_ASSERT_TRUE(ringHasData(c1));
	TEST_ASSERT_TRUE(ringHasData(c2));

	ringAdvance(c1);
	TEST_ASSERT_FALSE(ringHasData(c1));
	TEST_ASSERT_TRUE(ringHasData(c2));
}

void test_seq_increments()
{
	Frame f = {};
	ringPush(f, 0, 100);
	uint32_t s1 = canRingBuffer.entries[0].seq;
	ringPush(f, 0, 200);
	uint32_t s2 = canRingBuffer.entries[1].seq;
	TEST_ASSERT_EQUAL(s1 + 1, s2);

}

static void resetLogRing()
{
	logRingInit();
}

void test_log_ring_head_starts_at_zero()
{
	resetLogRing();
	TEST_ASSERT_EQUAL(0, logRingHead());
}

void test_log_ring_head_advances_after_push()
{
	resetLogRing();
	logRingPush("msg1", 100);
	TEST_ASSERT_EQUAL(1, logRingHead());
	logRingPush("msg2", 200);
	TEST_ASSERT_EQUAL(2, logRingHead());
}

void test_log_ring_read_since_empty_returns_zero()
{
	resetLogRing();
	uint16_t cursor = logRingHead();
	LogEntry out[4];
	uint16_t count = logRingReadSince(cursor, out, 4);
	TEST_ASSERT_EQUAL(0, count);
}

void test_log_ring_read_since_returns_new_entries()
{
	resetLogRing();
	uint16_t cursor = logRingHead();
	logRingPush("hello", 1000);
	logRingPush("world", 2000);
	LogEntry out[4];
	uint16_t count = logRingReadSince(cursor, out, 4);
	TEST_ASSERT_EQUAL(2, count);
	TEST_ASSERT_EQUAL_STRING("hello", out[0].msg);
	TEST_ASSERT_EQUAL_STRING("world", out[1].msg);
	TEST_ASSERT_EQUAL(1000, out[0].timestamp);
}

void test_log_ring_read_since_respects_maxout()
{
	resetLogRing();
	uint16_t cursor = logRingHead();
	for (int i = 0; i < 5; i++)
		logRingPush("x", (unsigned long)(i * 10));
	LogEntry out[2];
	uint16_t count = logRingReadSince(cursor, out, 2);
	TEST_ASSERT_EQUAL(2, count);
}

void test_log_ring_read_since_double_poll_no_duplicates()
{
	resetLogRing();
	logRingPush("first", 100);
	uint16_t cursor = logRingHead();
	logRingPush("second", 200);
	LogEntry out[4];
	uint16_t count = logRingReadSince(cursor, out, 4);
	TEST_ASSERT_EQUAL(1, count);
	TEST_ASSERT_EQUAL_STRING("second", out[0].msg);
}

void test_log_ring_read_since_cursor_at_current_head_returns_zero()
{
	resetLogRing();
	logRingPush("a", 10);
	logRingPush("b", 20);
	uint16_t cursor = logRingHead();
	LogEntry out[4];
	uint16_t count = logRingReadSince(cursor, out, 4);
	TEST_ASSERT_EQUAL(0, count);
}

int main()
{
	UNITY_BEGIN();
	RUN_TEST(test_push_increments_head);
	RUN_TEST(test_push_stores_frame);
	RUN_TEST(test_push_wraps_at_capacity);
	RUN_TEST(test_consumer_reads_pushed_frame);
	RUN_TEST(test_consumer_detects_overflow);
	RUN_TEST(test_multiple_consumers_independent);
	RUN_TEST(test_seq_increments);
	RUN_TEST(test_log_ring_head_starts_at_zero);
	RUN_TEST(test_log_ring_head_advances_after_push);
	RUN_TEST(test_log_ring_read_since_empty_returns_zero);
	RUN_TEST(test_log_ring_read_since_returns_new_entries);
	RUN_TEST(test_log_ring_read_since_respects_maxout);
	RUN_TEST(test_log_ring_read_since_double_poll_no_duplicates);
	RUN_TEST(test_log_ring_read_since_cursor_at_current_head_returns_zero);
	return UNITY_END();
}

