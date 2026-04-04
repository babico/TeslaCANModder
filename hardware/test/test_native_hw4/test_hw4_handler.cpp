#include <unity.h>
#include "can/frame.h"
#include "can/helpers.h"
#include "handlers/index.h"
#include "../support/driver.h"

static TestDriver mock;
static Hw4 handler;

void setUp() {
    mock.reset();
    handler = Hw4();
}

void tearDown() {}

// --- Speed profile from follow distance (CAN ID 1016) ---

void test_hw4_follow_distance_1_sets_profile_3() {
    Frame f = {};
    f.id = 1016;
    f.dlc = 8;
    f.data[5] = 0b00100000; // fd = 1
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL_INT(3, handler.speedProfile());
}

void test_hw4_follow_distance_2_sets_profile_2() {
    Frame f = {};
    f.id = 1016;
    f.dlc = 8;
    f.data[5] = 0b01000000; // fd = 2
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL_INT(2, handler.speedProfile());
}

void test_hw4_follow_distance_3_sets_profile_1() {
    Frame f = {};
    f.id = 1016;
    f.dlc = 8;
    f.data[5] = 0b01100000; // fd = 3
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL_INT(1, handler.speedProfile());
}

void test_hw4_follow_distance_4_sets_profile_0() {
    Frame f = {};
    f.id = 1016;
    f.dlc = 8;
    f.data[5] = 0b10000000; // fd = 4
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL_INT(0, handler.speedProfile());
}

void test_hw4_follow_distance_5_sets_profile_4() {
    Frame f = {};
    f.id = 1016;
    f.dlc = 8;
    f.data[5] = 0b10100000; // fd = 5
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL_INT(4, handler.speedProfile());
}

// --- FSD shadowing fix regression test ---

void test_hw4_fsd_enabled_only_set_on_mux0() {
    Frame f0 = {};
    f0.id = 1021;
    f0.dlc = 8;
    f0.data[0] = 0x00;
    f0.data[4] = 0x40;
    handler.handleMessage(f0, mock);
    TEST_ASSERT_TRUE(handler.fsdEnabled());

    mock.reset();
    Frame f2 = {};
    f2.id = 1021;
    f2.dlc = 8;
    f2.data[0] = 0x02;
    f2.data[4] = 0x00; // FSD bit not set in mux 2
    handler.handleMessage(f2, mock);
    TEST_ASSERT_TRUE(handler.fsdEnabled()); // latched from mux 0
    TEST_ASSERT_EQUAL(1, mock.sent.size()); // mux 2 always sends for HW4
}

// --- FSD activation (mux 0) ---

void test_hw4_fsd_mux0_sets_bits_46_and_60() {
    Frame f = {};
    f.id = 1021;
    f.dlc = 8;
    f.data[0] = 0x00;
    f.data[4] = 0x40;
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL(1, mock.sent.size());
    TEST_ASSERT_EQUAL_HEX8(0x40, mock.sent[0].data[5] & 0x40); // bit 46
    TEST_ASSERT_EQUAL_HEX8(0x10, mock.sent[0].data[7] & 0x10); // bit 60
}

void test_hw4_mux0_short_frame_is_consumed_without_send() {
    Frame f = {};
    f.id = 1021;
    f.dlc = 7;
    f.data[0] = 0x00;
    f.data[4] = 0x40;
    handler.handleMessage(f, mock);
    TEST_ASSERT_FALSE(handler.fsdEnabled());
    TEST_ASSERT_EQUAL(0, mock.sent.size());
}

void test_hw4_fsd_mux0_does_not_set_emergency_bit59_by_default() {
    Frame f = {};
    f.id = 1021;
    f.dlc = 8;
    f.data[0] = 0x00;
    f.data[4] = 0x40;
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL(1, mock.sent.size());
    TEST_ASSERT_EQUAL_HEX8(0x00, mock.sent[0].data[7] & 0x08); // bit 59
}

void test_hw4_no_send_when_fsd_disabled_mux0() {
    Frame f = {};
    f.id = 1021;
    f.dlc = 8;
    f.data[0] = 0x00;
    f.data[4] = 0x00;
    handler.handleMessage(f, mock);
    TEST_ASSERT_FALSE(handler.fsdEnabled());
    TEST_ASSERT_EQUAL(0, mock.sent.size());
}

// --- Nag suppression (mux 1) ---

void test_hw4_nag_suppression_clears_bit19_sets_bit47() {
    Frame f = {};
    f.id = 1021;
    f.dlc = 8;
    f.data[0] = 0x01;
    setBit(f, 19, true);
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL(1, mock.sent.size());
    TEST_ASSERT_FALSE((mock.sent[0].data[2] >> 3) & 0x01); // bit 19 cleared
    TEST_ASSERT_EQUAL_HEX8(0x80, mock.sent[0].data[5] & 0x80); // bit 47 set
}

void test_hw4_mux1_short_frame_is_consumed_without_send() {
    Frame f = {};
    f.id = 1021;
    f.dlc = 2;
    f.data[0] = 0x01;
    setBit(f, 19, true);
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL(0, mock.sent.size());
}

// --- Speed profile injection (mux 2) ---

void test_hw4_mux2_injects_speed_profile() {
    handler.speedProfile() = 3;
    Frame f = {};
    f.id = 1021;
    f.dlc = 8;
    f.data[0] = 0x02;
    f.data[7] = 0x00;
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL(1, mock.sent.size());
    uint8_t injected = (mock.sent[0].data[7] >> 4) & 0x07;
    TEST_ASSERT_EQUAL_UINT8(3, injected);
}

void test_hw4_mux2_clears_old_profile_bits() {
    handler.speedProfile() = 0;
    Frame f = {};
    f.id = 1021;
    f.dlc = 8;
    f.data[0] = 0x02;
    f.data[7] = 0x70; // old profile bits all set
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL(1, mock.sent.size());
    uint8_t injected = (mock.sent[0].data[7] >> 4) & 0x07;
    TEST_ASSERT_EQUAL_UINT8(0, injected);
}

void test_hw4_mux2_short_frame_is_consumed_without_send() {
    handler.speedProfile() = 4;
    Frame f = {};
    f.id = 1021;
    f.dlc = 7;
    f.data[0] = 0x02;
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL(0, mock.sent.size());
}

// --- Send counts ---

void test_hw4_mux0_fsd_enabled_sends_1() {
    Frame f = {};
    f.id = 1021;
    f.dlc = 8;
    f.data[0] = 0x00;
    f.data[4] = 0x40;
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL(1, mock.sent.size());
}

void test_hw4_mux1_sends_1() {
    Frame f = {};
    f.id = 1021;
    f.dlc = 8;
    f.data[0] = 0x01;
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL(1, mock.sent.size());
}

void test_hw4_mux2_sends_1() {
    Frame f = {};
    f.id = 1021;
    f.dlc = 8;
    f.data[0] = 0x02;
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL(1, mock.sent.size());
}

void test_hw4_ignores_unrelated_can_id() {
    Frame f = {};
    f.id = 999;
    f.dlc = 8;
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL(0, mock.sent.size());
}

// --- ISA speed chime suppression (CAN ID 921) ---

void test_hw4_isa_suppress_disabled_ignores_921() {
    handler.isaSpeedChimeSuppressValue() = false;
    Frame f = {};
    f.id = 921;
    f.dlc = 8;
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL(0, mock.sent.size());
}

void test_hw4_isa_suppress_enabled_sets_bit5_of_data1() {
    handler.isaSpeedChimeSuppressValue() = true;
    Frame f = {};
    f.id = 921;
    f.dlc = 8;
    f.data[1] = 0x00;
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL(1, mock.sent.size());
    TEST_ASSERT_EQUAL_HEX8(0x20, mock.sent[0].data[1] & 0x20);
}

void test_hw4_isa_suppress_short_frame_is_consumed_without_send() {
    handler.isaSpeedChimeSuppressValue() = true;
    Frame f = {};
    f.id = 921;
    f.dlc = 7;
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL(0, mock.sent.size());
}

void test_hw4_isa_suppress_preserves_existing_data1_bits() {
    handler.isaSpeedChimeSuppressValue() = true;
    Frame f = {};
    f.id = 921;
    f.dlc = 8;
    f.data[1] = 0xC3;
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL_HEX8(0xE3, mock.sent[0].data[1]); // 0xC3 | 0x20
}

void test_hw4_isa_suppress_checksum_correct() {
    handler.isaSpeedChimeSuppressValue() = true;
    Frame f = {};
    f.id = 921;
    f.dlc = 8;
    f.data[0] = 0x10;
    f.data[1] = 0x05;
    f.data[2] = 0x00;
    f.data[3] = 0x00;
    f.data[4] = 0x00;
    f.data[5] = 0x00;
    f.data[6] = 0x00;
    handler.handleMessage(f, mock);
    // After OR: data[1] = 0x25
    // sum of data[0..6] = 0x10 + 0x25 = 0x35
    // sum += (921 & 0xFF) + (921 >> 8) = 0x99 + 0x03 = 0x9C
    // total = 0x35 + 0x9C = 0xD1
    TEST_ASSERT_EQUAL_HEX8(0xD1, mock.sent[0].data[7]);
}

void test_hw4_isa_suppress_returns_early_no_further_processing() {
    handler.isaSpeedChimeSuppressValue() = true;
    handler.fsdEnabled() = true;
    Frame f = {};
    f.id = 921;
    f.dlc = 8;
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL(1, mock.sent.size()); // only the ISA send, not any FSD logic
}

void test_hw4_short_mux0_frame_is_ignored_safely() {
    Frame f = {};
    f.id = 1021;
    f.dlc = 4;
    f.data[0] = 0x00;
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL(0, mock.sent.size());
}

void test_hw4_short_isa_frame_is_ignored_safely() {
    handler.isaSpeedChimeSuppressValue() = true;
    Frame f = {};
    f.id = 921;
    f.dlc = 7;
    handler.handleMessage(f, mock);
    TEST_ASSERT_EQUAL(0, mock.sent.size());
}

// --- Filter IDs ---

void test_hw4_filter_ids_count() {
    TEST_ASSERT_EQUAL_UINT8(3, handler.filterIdCount());
}

void test_hw4_filter_ids_values() {
    const uint32_t* ids = handler.filterIds();
    TEST_ASSERT_EQUAL_UINT32(921, ids[0]);
    TEST_ASSERT_EQUAL_UINT32(1016, ids[1]);
    TEST_ASSERT_EQUAL_UINT32(1021, ids[2]);
}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_hw4_filter_ids_count);
    RUN_TEST(test_hw4_filter_ids_values);

    RUN_TEST(test_hw4_follow_distance_1_sets_profile_3);
    RUN_TEST(test_hw4_follow_distance_2_sets_profile_2);
    RUN_TEST(test_hw4_follow_distance_3_sets_profile_1);
    RUN_TEST(test_hw4_follow_distance_4_sets_profile_0);
    RUN_TEST(test_hw4_follow_distance_5_sets_profile_4);

    RUN_TEST(test_hw4_fsd_enabled_only_set_on_mux0);
    RUN_TEST(test_hw4_fsd_mux0_sets_bits_46_and_60);
    RUN_TEST(test_hw4_mux0_short_frame_is_consumed_without_send);
    RUN_TEST(test_hw4_fsd_mux0_does_not_set_emergency_bit59_by_default);
    RUN_TEST(test_hw4_no_send_when_fsd_disabled_mux0);

    RUN_TEST(test_hw4_nag_suppression_clears_bit19_sets_bit47);
    RUN_TEST(test_hw4_mux1_short_frame_is_consumed_without_send);

    RUN_TEST(test_hw4_mux2_injects_speed_profile);
    RUN_TEST(test_hw4_mux2_clears_old_profile_bits);
    RUN_TEST(test_hw4_mux2_short_frame_is_consumed_without_send);

    RUN_TEST(test_hw4_mux0_fsd_enabled_sends_1);
    RUN_TEST(test_hw4_mux1_sends_1);
    RUN_TEST(test_hw4_mux2_sends_1);
    RUN_TEST(test_hw4_ignores_unrelated_can_id);

    RUN_TEST(test_hw4_isa_suppress_disabled_ignores_921);
    RUN_TEST(test_hw4_isa_suppress_enabled_sets_bit5_of_data1);
    RUN_TEST(test_hw4_isa_suppress_short_frame_is_consumed_without_send);
    RUN_TEST(test_hw4_isa_suppress_preserves_existing_data1_bits);
    RUN_TEST(test_hw4_isa_suppress_checksum_correct);
    RUN_TEST(test_hw4_isa_suppress_returns_early_no_further_processing);
    RUN_TEST(test_hw4_short_mux0_frame_is_ignored_safely);
    RUN_TEST(test_hw4_short_isa_frame_is_ignored_safely);

    return UNITY_END();
}
