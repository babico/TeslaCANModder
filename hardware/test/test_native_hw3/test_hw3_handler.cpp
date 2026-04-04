#include <unity.h>
#include "can/frame.h"
#include "can/helpers.h"
#include "handlers/index.h"
#include "../support/driver.h"

static TestDriver mock;
static Hw3 handler;

void setUp() {
    mock.reset();
    handler = Hw3();
}

void tearDown() {}

// --- Speed profile from follow distance (CAN ID 1016) ---

void test_hw3_follow_distance_1_sets_profile_2() {
    Frame f = {};
    f.id = 1016;
    f.dlc = 8;
    f.data[5] = 0b00100000; // followDistance = 1
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL_INT(2, handler.speedProfile());
    TEST_ASSERT_EQUAL(0, mock.sent.size());
}

void test_hw3_follow_distance_2_sets_profile_1() {
    Frame f = {};
    f.id = 1016;
    f.dlc = 8;
    f.data[5] = 0b01000000; // followDistance = 2
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL_INT(1, handler.speedProfile());
}

void test_hw3_follow_distance_3_sets_profile_0() {
    Frame f = {};
    f.id = 1016;
    f.dlc = 8;
    f.data[5] = 0b01100000; // followDistance = 3
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL_INT(0, handler.speedProfile());
}

void test_hw3_follow_distance_0_keeps_default() {
    Frame f = {};
    f.id = 1016;
    f.dlc = 8;
    f.data[5] = 0x00; // followDistance = 0
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL_INT(1, handler.speedProfile()); // default
}

// --- FSD shadowing fix regression test ---

void test_hw3_fsd_enabled_only_set_on_mux0() {
    // Step 1: mux 0 with FSD bit set -> FSDEnabled should become true
    Frame f0 = {};
    f0.id = 1021;
    f0.dlc = 8;
    f0.data[0] = 0x00; // mux 0
    f0.data[4] = 0x40; // FSD selected
    handler.handleMessage(f0, mock);
    TEST_ASSERT_TRUE(handler.fsdEnabled());

    // Step 2: mux 2 with FSD bit NOT set -> FSDEnabled should STAY true
    mock.reset();
    Frame f2 = {};
    f2.id = 1021;
    f2.dlc = 8;
    f2.data[0] = 0x02; // mux 2
    f2.data[4] = 0x00; // FSD bit not set in this frame
    handler.handleMessage(f2, mock);
    TEST_ASSERT_TRUE(handler.fsdEnabled());
    TEST_ASSERT_EQUAL(1, mock.sent.size()); // mux 2 should still send because FSDEnabled is latched
}

void test_hw3_fsd_disabled_on_mux0_prevents_mux2_send() {
    // mux 0 with FSD disabled
    Frame f0 = {};
    f0.id = 1021;
    f0.dlc = 8;
    f0.data[0] = 0x00;
    f0.data[4] = 0x00; // FSD NOT selected
    handler.handleMessage(f0, mock);
    TEST_ASSERT_FALSE(handler.fsdEnabled());

    // mux 2 should NOT send
    mock.reset();
    Frame f2 = {};
    f2.id = 1021;
    f2.dlc = 8;
    f2.data[0] = 0x02;
    handler.handleMessage(f2, mock);
    TEST_ASSERT_EQUAL(0, mock.sent.size());
}

// --- FSD activation (mux 0) ---

void test_hw3_fsd_mux0_sends_with_bit46() {
    Frame f = {};
    f.id = 1021;
    f.dlc = 8;
    f.data[0] = 0x00;
    f.data[4] = 0x40;
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL(1, mock.sent.size());
    TEST_ASSERT_EQUAL_HEX8(0x40, mock.sent[0].data[5] & 0x40);
}

void test_hw3_mux0_short_frame_is_consumed_without_send() {
    Frame f = {};
    f.id = 1021;
    f.dlc = 6;
    f.data[0] = 0x00;
    f.data[4] = 0x40;
    handler.handleMessage(f, mock);
    TEST_ASSERT_FALSE(handler.fsdEnabled());
    TEST_ASSERT_EQUAL(0, mock.sent.size());
}

// --- Nag suppression (mux 1) ---

void test_hw3_nag_suppression_clears_bit19_on_mux1() {
    Frame f = {};
    f.id = 1021;
    f.dlc = 8;
    f.data[0] = 0x01;
    setBit(f, 19, true);
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL(1, mock.sent.size());
    TEST_ASSERT_FALSE((mock.sent[0].data[2] >> 3) & 0x01);
}

void test_hw3_mux1_short_frame_is_consumed_without_send() {
    Frame f = {};
    f.id = 1021;
    f.dlc = 2;
    f.data[0] = 0x01;
    setBit(f, 19, true);
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL(0, mock.sent.size());
}

void test_hw3_mux2_short_frame_is_consumed_without_send() {
    handler.fsdEnabled() = true;
    handler.speedOffsetValue() = 35;
    Frame f = {};
    f.id = 1021;
    f.dlc = 1;
    f.data[0] = 0x02;
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL(0, mock.sent.size());
}

// --- No sends on unrelated CAN IDs ---

void test_hw3_ignores_unrelated_can_id() {
    Frame f = {};
    f.id = 999;
    f.dlc = 8;
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL(0, mock.sent.size());
}

// --- Send counts ---

void test_hw3_fsd_enabled_mux0_sends_exactly_1() {
    Frame f = {};
    f.id = 1021;
    f.dlc = 8;
    f.data[0] = 0x00;
    f.data[4] = 0x40;
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL(1, mock.sent.size());
}

void test_hw3_mux1_sends_exactly_1() {
    Frame f = {};
    f.id = 1021;
    f.dlc = 8;
    f.data[0] = 0x01;
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL(1, mock.sent.size());
}

// --- Filter IDs ---

void test_hw3_filter_ids_count() {
    TEST_ASSERT_EQUAL_UINT8(2, handler.filterIdCount());
}

void test_hw3_filter_ids_values() {
    const uint32_t* ids = handler.filterIds();
    TEST_ASSERT_EQUAL_UINT32(1016, ids[0]);
    TEST_ASSERT_EQUAL_UINT32(1021, ids[1]);
}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_hw3_filter_ids_count);
    RUN_TEST(test_hw3_filter_ids_values);

    RUN_TEST(test_hw3_follow_distance_1_sets_profile_2);
    RUN_TEST(test_hw3_follow_distance_2_sets_profile_1);
    RUN_TEST(test_hw3_follow_distance_3_sets_profile_0);
    RUN_TEST(test_hw3_follow_distance_0_keeps_default);

    RUN_TEST(test_hw3_fsd_enabled_only_set_on_mux0);
    RUN_TEST(test_hw3_fsd_disabled_on_mux0_prevents_mux2_send);

    RUN_TEST(test_hw3_fsd_mux0_sends_with_bit46);
    RUN_TEST(test_hw3_mux0_short_frame_is_consumed_without_send);
    RUN_TEST(test_hw3_nag_suppression_clears_bit19_on_mux1);
    RUN_TEST(test_hw3_mux1_short_frame_is_consumed_without_send);
    RUN_TEST(test_hw3_mux2_short_frame_is_consumed_without_send);
    RUN_TEST(test_hw3_ignores_unrelated_can_id);

    RUN_TEST(test_hw3_fsd_enabled_mux0_sends_exactly_1);
    RUN_TEST(test_hw3_mux1_sends_exactly_1);

    return UNITY_END();
}
