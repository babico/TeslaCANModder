// ── Rate Limiter Tests ───────────────────────────────────────────────────────
// Tests per-CAN-ID TX rate limiting and self-echo detection ring buffer.

#include <unity.h>
#include <cstring>

#include "core/types.h"
#include "infra/rate_limit.h"

void setUp() {
  rateLimitInit();
  echoRingInit();
}
void tearDown() {}

// ── rateLimitCheck ───────────────────────────────────────────────────────────

void test_rate_limit_first_call_allowed() {
  TEST_ASSERT_TRUE(rateLimitCheck(0x100, 1000));
}

void test_rate_limit_immediate_repeat_denied() {
  rateLimitCheck(0x100, 1000);
  TEST_ASSERT_FALSE(rateLimitCheck(0x100, 1000));
}

void test_rate_limit_after_interval_allowed() {
  rateLimitCheck(0x100, 1000);
  // Default interval is 100ms (10 Hz)
  TEST_ASSERT_TRUE(rateLimitCheck(0x100, 1101));
}

void test_rate_limit_different_ids_independent() {
  rateLimitCheck(0x100, 1000);
  TEST_ASSERT_TRUE(rateLimitCheck(0x200, 1000));
}

void test_rate_limit_slot_eviction() {
  // Fill all slots
  for (uint32_t i = 0; i < RATE_LIMIT_SLOTS; i++) {
    rateLimitCheck(i + 1, 1000);
  }
  // One more should still work (evicts oldest)
  TEST_ASSERT_TRUE(rateLimitCheck(0xFFF, 1000));
}

// ── echoRingRecord / echoRingCheck (ID-only matching) ────────────────────────

void test_echo_ring_no_match_on_empty() {
  TEST_ASSERT_FALSE(echoRingCheck(0x100));
}

void test_echo_ring_record_then_check() {
  echoRingRecord(0x100);
  TEST_ASSERT_TRUE(echoRingCheck(0x100));
}

void test_echo_ring_check_consumes_entry() {
  echoRingRecord(0x100);
  TEST_ASSERT_TRUE(echoRingCheck(0x100));  // first check succeeds
  TEST_ASSERT_FALSE(echoRingCheck(0x100)); // second check fails (consumed)
}

void test_echo_ring_different_id_no_match() {
  echoRingRecord(0x100);
  TEST_ASSERT_FALSE(echoRingCheck(0x200));
}

void test_echo_ring_same_id_always_matches() {
  // Echo ring matches by ID only — data content is irrelevant
  echoRingRecord(0x100);
  TEST_ASSERT_TRUE(echoRingCheck(0x100));
}

void test_echo_ring_wraps_around() {
  // Push more than ring size, oldest should be gone
  for (uint8_t i = 0; i < ECHO_RING_SIZE + 2; i++) {
    echoRingRecord(i + 1);
  }
  // Oldest entry (id=1) should be evicted
  TEST_ASSERT_FALSE(echoRingCheck(1));

  // Recent entry should still match
  TEST_ASSERT_TRUE(echoRingCheck(ECHO_RING_SIZE + 2));
}

// ── main ─────────────────────────────────────────────────────────────────────

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_rate_limit_first_call_allowed);
  RUN_TEST(test_rate_limit_immediate_repeat_denied);
  RUN_TEST(test_rate_limit_after_interval_allowed);
  RUN_TEST(test_rate_limit_different_ids_independent);
  RUN_TEST(test_rate_limit_slot_eviction);
  RUN_TEST(test_echo_ring_no_match_on_empty);
  RUN_TEST(test_echo_ring_record_then_check);
  RUN_TEST(test_echo_ring_check_consumes_entry);
  RUN_TEST(test_echo_ring_different_id_no_match);
  RUN_TEST(test_echo_ring_same_id_always_matches);
  RUN_TEST(test_echo_ring_wraps_around);
  return UNITY_END();
}
