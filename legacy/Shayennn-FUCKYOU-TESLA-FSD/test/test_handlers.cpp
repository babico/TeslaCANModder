/*
 * Desktop tests for shared CAN handler logic.
 *
 *   cd test && make
 */

#include <cstdint>
#include <cstdio>
#include <vector>

#include "../shared/vehicle_logic.h"

using tesla_fsd::FrameSink;
using tesla_fsd::HandleResult;
using tesla_fsd::HW3Handler;
using tesla_fsd::HW4Handler;
using tesla_fsd::LegacyHandler;
using tesla_fsd::UI_APMV3_BRANCH_DEV;
using tesla_fsd::UI_APMV3_BRANCH_OVERRIDE;
using tesla_fsd::UI_DRIVING_SIDE_LEFT;
using tesla_fsd::UI_DRIVING_SIDE_OVERRIDE;
using tesla_fsd::can_frame;
using tesla_fsd::isFSDSelectedInUI;
using tesla_fsd::readApmv3Branch;
using tesla_fsd::readDrivingSide;
using tesla_fsd::readMuxID;
using tesla_fsd::setBit;
using tesla_fsd::setSpeedProfileV12V13;

static std::vector<can_frame> sentFrames;
static bool sendSucceeds = true;

static bool recordFrameSend(void*, const can_frame& frame) {
  sentFrames.push_back(frame);
  return sendSucceeds;
}

static FrameSink makeSink() {
  return {nullptr, &recordFrameSend};
}

static int testsPassed = 0;
static int testsFailed = 0;

#define RUN(fn) do { \
  sentFrames.clear(); \
  sendSucceeds = true; \
  printf("  %-60s ", #fn); \
  fn(); \
  printf("PASS\n"); \
  testsPassed++; \
} while (0)

#define ASSERT_TRUE(expr) do { \
  if (!(expr)) { \
    printf("FAIL\n    line %d: %s\n", __LINE__, #expr); \
    testsFailed++; return; \
  } \
} while (0)

#define ASSERT_FALSE(expr) ASSERT_TRUE(!(expr))

#define ASSERT_EQ(a, b) do { \
  auto _va = (a); auto _vb = (b); \
  if (_va != _vb) { \
    printf("FAIL\n    line %d: %s == %d, expected %s == %d\n", \
           __LINE__, #a, (int)_va, #b, (int)_vb); \
    testsFailed++; return; \
  } \
} while (0)

static bool getBit(const can_frame& frame, int bit) {
  return (frame.data[bit / 8] >> (bit % 8)) & 0x01;
}

static uint8_t getField(const can_frame& frame, int startBit, int width) {
  return (frame.data[startBit / 8] >> (startBit % 8)) & ((1 << width) - 1);
}

static can_frame makeFrame(uint32_t id,
    uint8_t d0=0, uint8_t d1=0, uint8_t d2=0, uint8_t d3=0,
    uint8_t d4=0, uint8_t d5=0, uint8_t d6=0, uint8_t d7=0) {
  can_frame frame{};
  frame.can_id = id;
  frame.can_dlc = 8;
  frame.data[0] = d0;
  frame.data[1] = d1;
  frame.data[2] = d2;
  frame.data[3] = d3;
  frame.data[4] = d4;
  frame.data[5] = d5;
  frame.data[6] = d6;
  frame.data[7] = d7;
  return frame;
}

static void assertSentResult(const HandleResult& result) {
  ASSERT_TRUE(result.handled);
  ASSERT_TRUE(result.attemptedSend);
  ASSERT_TRUE(result.sent);
}

void test_setBit_sets_and_preserves() {
  can_frame frame = makeFrame(0);
  setBit(frame, 5, true);
  ASSERT_TRUE(getBit(frame, 5));
  ASSERT_FALSE(getBit(frame, 4));
  ASSERT_FALSE(getBit(frame, 6));
}

void test_setBit_clears_and_preserves() {
  can_frame frame = makeFrame(0);
  frame.data[0] = 0xFF;
  setBit(frame, 3, false);
  ASSERT_FALSE(getBit(frame, 3));
  ASSERT_TRUE(getBit(frame, 2));
  ASSERT_TRUE(getBit(frame, 4));
}

void test_setBit_cross_byte_boundaries() {
  can_frame frame = makeFrame(0);
  setBit(frame, 14, true);
  ASSERT_EQ(frame.data[1], 0x40);
  setBit(frame, 38, true);
  ASSERT_EQ(frame.data[4], 0x40);
  setBit(frame, 60, true);
  ASSERT_EQ(frame.data[7], 0x10);
}

void test_readMuxID_extracts_lower_3_bits() {
  can_frame frame = makeFrame(0, 0x05);
  ASSERT_EQ(readMuxID(frame), 5);
  frame.data[0] = 0xFE;
  ASSERT_EQ(readMuxID(frame), 6);
  frame.data[0] = 0x08;
  ASSERT_EQ(readMuxID(frame), 0);
}

void test_isFSDSelectedInUI_reads_bit38() {
  can_frame frame = makeFrame(0, 0, 0, 0, 0, 0x40);
  ASSERT_TRUE(isFSDSelectedInUI(frame));
  frame.data[4] = 0xBF;
  ASSERT_FALSE(isFSDSelectedInUI(frame));
}

void test_setSpeedProfileV12V13_writes_bits_1_2() {
  can_frame frame = makeFrame(0);
  frame.data[6] = 0xFF;
  setSpeedProfileV12V13(frame, 2);
  ASSERT_EQ(frame.data[6] & 0x06, 0x04);
  ASSERT_EQ(frame.data[6] & 0xF9, 0xF9);
  setSpeedProfileV12V13(frame, 0);
  ASSERT_EQ(frame.data[6] & 0x06, 0x00);
}

void test_readDrivingSide_extracts_bits_40_41() {
  can_frame frame = makeFrame(1016, 0, 0, 0, 0, 0, 0x02);
  ASSERT_EQ(readDrivingSide(frame), 2);
  frame.data[5] = 0x01;
  ASSERT_EQ(readDrivingSide(frame), 1);
  frame.data[5] = 0x00;
  ASSERT_EQ(readDrivingSide(frame), 0);
}

void test_readApmv3Branch_extracts_bits_40_42() {
  can_frame frame = makeFrame(1021, 0x01, 0, 0, 0, 0, 0x05);
  ASSERT_EQ(readApmv3Branch(frame), 5);
  frame.data[5] = 0x03;
  ASSERT_EQ(readApmv3Branch(frame), 3);
  frame.data[5] = 0x00;
  ASSERT_EQ(readApmv3Branch(frame), 0);
}

void test_legacy_unhandled_id_returns_false() {
  LegacyHandler handler;
  can_frame frame = makeFrame(999);
  HandleResult result = handler.handleMessage(frame, makeSink());
  ASSERT_FALSE(result.handled);
  ASSERT_FALSE(result.attemptedSend);
  ASSERT_EQ((int)sentFrames.size(), 0);
}

void test_legacy_1006_mux0_sets_bit46_and_speed_profile() {
  LegacyHandler handler;
  can_frame frame = makeFrame(1006, 0x00, 0, 0, 62);
  HandleResult result = handler.handleMessage(frame, makeSink());
  assertSentResult(result);
  ASSERT_EQ((int)sentFrames.size(), 1);
  ASSERT_TRUE(getBit(sentFrames[0], 46));
  ASSERT_EQ(handler.speedProfile, 1);
  ASSERT_EQ(getField(sentFrames[0], 49, 2), 1);
}

void test_legacy_1006_mux1_clears_eceR79() {
  LegacyHandler handler;
  can_frame frame = makeFrame(1006, 0x01);
  frame.data[2] = 0xFF;
  HandleResult result = handler.handleMessage(frame, makeSink());
  assertSentResult(result);
  ASSERT_EQ((int)sentFrames.size(), 1);
  ASSERT_FALSE(getBit(sentFrames[0], 19));
}

void test_legacy_2047_sets_autopilot_to_3() {
  LegacyHandler handler;
  can_frame frame = makeFrame(2047, 2, 0, 0, 0, 0, 0xFF);
  HandleResult result = handler.handleMessage(frame, makeSink());
  assertSentResult(result);
  ASSERT_EQ((int)sentFrames.size(), 1);
  ASSERT_EQ(getField(sentFrames[0], 42, 3), 3);
}

void test_hw3_unhandled_id_returns_false() {
  HW3Handler handler;
  can_frame frame = makeFrame(999);
  HandleResult result = handler.handleMessage(frame, makeSink());
  ASSERT_FALSE(result.handled);
  ASSERT_FALSE(result.attemptedSend);
  ASSERT_EQ((int)sentFrames.size(), 0);
}

void test_hw3_1016_enables_nav_on_maps_bits() {
  HW3Handler handler;
  can_frame frame = makeFrame(1016, 0, 0, 0, 0, 0, 2u << 5);
  HandleResult result = handler.handleMessage(frame, makeSink());
  assertSentResult(result);
  ASSERT_EQ((int)sentFrames.size(), 1);
  ASSERT_TRUE(getBit(sentFrames[0], 5));
  ASSERT_TRUE(getBit(sentFrames[0], 13));
  ASSERT_TRUE(getBit(sentFrames[0], 14));
  ASSERT_TRUE(getBit(sentFrames[0], 48));
  ASSERT_TRUE(getBit(sentFrames[0], 49));
  ASSERT_EQ(readDrivingSide(sentFrames[0]), UI_DRIVING_SIDE_OVERRIDE);
  ASSERT_EQ(handler.speedProfile, 1);
}

void test_hw3_1016_follow_distance_1() {
  HW3Handler handler;
  can_frame frame = makeFrame(1016, 0, 0, 0, 0, 0, 1u << 5);
  handler.handleMessage(frame, makeSink());
  ASSERT_EQ(handler.speedProfile, 2);
}

void test_hw3_1016_follow_distance_3() {
  HW3Handler handler;
  can_frame frame = makeFrame(1016, 0, 0, 0, 0, 0, 3u << 5);
  handler.handleMessage(frame, makeSink());
  ASSERT_EQ(handler.speedProfile, 0);
}

void test_hw3_1021_mux0_sets_fsdStops_and_bit46() {
  HW3Handler handler;
  can_frame frame = makeFrame(1021, 0x00, 0, 0, 60);
  HandleResult result = handler.handleMessage(frame, makeSink());
  assertSentResult(result);
  ASSERT_EQ((int)sentFrames.size(), 1);
  ASSERT_TRUE(getBit(sentFrames[0], 38));
  ASSERT_TRUE(getBit(sentFrames[0], 46));
}

void test_hw3_1021_mux0_preserves_speedProfile_from_1016() {
  HW3Handler handler;
  can_frame frame1016 = makeFrame(1016, 0, 0, 0, 0, 0, 1u << 5);
  handler.handleMessage(frame1016, makeSink());
  ASSERT_EQ(handler.speedProfile, 2);
  sentFrames.clear();
  can_frame frame1021 = makeFrame(1021, 0x00, 0, 0, 60);
  handler.handleMessage(frame1021, makeSink());
  ASSERT_EQ(handler.speedProfile, 2);
}

void test_hw3_1021_mux0_runs_without_fsd_gate() {
  HW3Handler handler;
  can_frame frame = makeFrame(1021, 0x00, 0, 0, 60, 0x00);
  ASSERT_FALSE(isFSDSelectedInUI(frame));
  HandleResult result = handler.handleMessage(frame, makeSink());
  assertSentResult(result);
  ASSERT_EQ((int)sentFrames.size(), 1);
  ASSERT_TRUE(getBit(sentFrames[0], 38));
}

void test_hw3_1021_mux1_sets_branch_and_lane_graph_bits() {
  HW3Handler handler;
  can_frame frame = makeFrame(1021, 0x01);
  frame.data[2] = 0xFF;
  frame.data[5] = 0x08;
  HandleResult result = handler.handleMessage(frame, makeSink());
  assertSentResult(result);
  ASSERT_EQ((int)sentFrames.size(), 1);
  ASSERT_FALSE(getBit(sentFrames[0], 19));
  ASSERT_TRUE(getBit(sentFrames[0], 45));
  ASSERT_EQ(readApmv3Branch(sentFrames[0]), UI_APMV3_BRANCH_OVERRIDE);
}

void test_hw3_1021_mux2_writes_speed_offset() {
  HW3Handler handler;
  handler.speedOffset = 25;
  can_frame frame = makeFrame(1021, 0x02, 0xFF);
  HandleResult result = handler.handleMessage(frame, makeSink());
  assertSentResult(result);
  ASSERT_EQ((int)sentFrames.size(), 1);
  uint8_t lo = (sentFrames[0].data[0] >> 6) & 0x03;
  uint8_t hi = sentFrames[0].data[1] & 0x3F;
  ASSERT_EQ((int)(lo | (hi << 2)), 25);
}

void test_hw3_2047_sets_autopilot_to_3() {
  HW3Handler handler;
  can_frame frame = makeFrame(2047, 2, 0, 0, 0, 0, 0xFF);
  HandleResult result = handler.handleMessage(frame, makeSink());
  assertSentResult(result);
  ASSERT_EQ(getField(sentFrames[0], 42, 3), 3);
}

void test_hw4_unhandled_id_returns_false() {
  HW4Handler handler;
  can_frame frame = makeFrame(999);
  HandleResult result = handler.handleMessage(frame, makeSink());
  ASSERT_FALSE(result.handled);
  ASSERT_FALSE(result.attemptedSend);
  ASSERT_EQ((int)sentFrames.size(), 0);
}

void test_hw4_1016_enables_nav_on_maps_bits() {
  HW4Handler handler;
  can_frame frame = makeFrame(1016, 0, 0, 0, 0, 0, 2u << 5);
  HandleResult result = handler.handleMessage(frame, makeSink());
  assertSentResult(result);
  ASSERT_EQ((int)sentFrames.size(), 1);
  ASSERT_TRUE(getBit(sentFrames[0], 5));
  ASSERT_TRUE(getBit(sentFrames[0], 13));
  ASSERT_TRUE(getBit(sentFrames[0], 14));
  ASSERT_TRUE(getBit(sentFrames[0], 48));
  ASSERT_TRUE(getBit(sentFrames[0], 49));
  ASSERT_EQ(readDrivingSide(sentFrames[0]), UI_DRIVING_SIDE_OVERRIDE);
  ASSERT_EQ(handler.speedProfile, 2);
}

void test_hw4_1016_follow_distance_mapping() {
  struct MappingCase { int followDistance; int profile; } cases[] = {
    {1, 3}, {2, 2}, {3, 1}, {4, 0}, {5, 4},
  };

  for (const auto& testCase : cases) {
    sentFrames.clear();
    HW4Handler handler;
    can_frame frame = makeFrame(1016, 0, 0, 0, 0, 0, static_cast<uint8_t>(testCase.followDistance << 5));
    handler.handleMessage(frame, makeSink());
    ASSERT_EQ(handler.speedProfile, testCase.profile);
  }
}

void test_hw4_1021_mux0_sets_bits_38_46_60() {
  HW4Handler handler;
  can_frame frame = makeFrame(1021, 0x00);
  HandleResult result = handler.handleMessage(frame, makeSink());
  assertSentResult(result);
  ASSERT_EQ((int)sentFrames.size(), 1);
  ASSERT_TRUE(getBit(sentFrames[0], 38));
  ASSERT_TRUE(getBit(sentFrames[0], 46));
  ASSERT_TRUE(getBit(sentFrames[0], 60));
}

void test_hw4_1021_mux1_sets_branch_and_bit47() {
  HW4Handler handler;
  can_frame frame = makeFrame(1021, 0x01);
  frame.data[2] = 0xFF;
  frame.data[5] = 0x08;
  HandleResult result = handler.handleMessage(frame, makeSink());
  assertSentResult(result);
  ASSERT_EQ((int)sentFrames.size(), 1);
  ASSERT_FALSE(getBit(sentFrames[0], 19));
  ASSERT_TRUE(getBit(sentFrames[0], 45));
  ASSERT_TRUE(getBit(sentFrames[0], 47));
  ASSERT_EQ(readApmv3Branch(sentFrames[0]), UI_APMV3_BRANCH_OVERRIDE);
}

void test_hw4_1021_mux2_writes_speed_profile() {
  HW4Handler handler;
  handler.speedProfile = 5;
  can_frame frame = makeFrame(1021, 0x02);
  HandleResult result = handler.handleMessage(frame, makeSink());
  assertSentResult(result);
  ASSERT_EQ((int)sentFrames.size(), 1);
  ASSERT_EQ((sentFrames[0].data[7] >> 4) & 0x07, 5);
}

void test_hw4_2047_sets_autopilot_to_4() {
  HW4Handler handler;
  can_frame frame = makeFrame(2047, 2, 0, 0, 0, 0, 0xFF);
  HandleResult result = handler.handleMessage(frame, makeSink());
  assertSentResult(result);
  ASSERT_EQ(getField(sentFrames[0], 42, 3), 4);
}

void test_hw3_navStopsDisabled_clears_nav_bits() {
  HW3Handler handler;
  handler.navStopsEnabled = false;
  can_frame frame = makeFrame(1016, 0, 0, 0, 0, 0, 2u << 5);
  handler.handleMessage(frame, makeSink());
  ASSERT_EQ((int)sentFrames.size(), 1);
  ASSERT_TRUE(getBit(sentFrames[0], 5));   // dasDeveloper still set
  ASSERT_FALSE(getBit(sentFrames[0], 13)); // driveOnMapsEnable off
  ASSERT_TRUE(getBit(sentFrames[0], 14));  // handsOnRequirementDisable still set
  ASSERT_FALSE(getBit(sentFrames[0], 48)); // hasDriveOnNav off
  ASSERT_FALSE(getBit(sentFrames[0], 49)); // followNavRouteEnable off
}

void test_hw3_navStopsDisabled_clears_fsdStops() {
  HW3Handler handler;
  handler.navStopsEnabled = false;
  can_frame frame = makeFrame(1021, 0x00, 0, 0, 60);
  handler.handleMessage(frame, makeSink());
  ASSERT_EQ((int)sentFrames.size(), 1);
  ASSERT_FALSE(getBit(sentFrames[0], 38)); // fsdStopsControlEnabled off
  ASSERT_TRUE(getBit(sentFrames[0], 46));  // showTrackLabels still set
}

void test_hw4_navStopsDisabled_clears_nav_bits() {
  HW4Handler handler;
  handler.navStopsEnabled = false;
  can_frame frame = makeFrame(1016, 0, 0, 0, 0, 0, 2u << 5);
  handler.handleMessage(frame, makeSink());
  ASSERT_EQ((int)sentFrames.size(), 1);
  ASSERT_TRUE(getBit(sentFrames[0], 5));
  ASSERT_FALSE(getBit(sentFrames[0], 13));
  ASSERT_TRUE(getBit(sentFrames[0], 14));
  ASSERT_FALSE(getBit(sentFrames[0], 48));
  ASSERT_FALSE(getBit(sentFrames[0], 49));
}

void test_hw4_navStopsDisabled_clears_fsdStops_and_visionStops() {
  HW4Handler handler;
  handler.navStopsEnabled = false;
  can_frame frame = makeFrame(1021, 0x00);
  handler.handleMessage(frame, makeSink());
  ASSERT_EQ((int)sentFrames.size(), 1);
  ASSERT_FALSE(getBit(sentFrames[0], 38)); // fsdStopsControlEnabled off
  ASSERT_TRUE(getBit(sentFrames[0], 46));  // showTrackLabels still set
  ASSERT_FALSE(getBit(sentFrames[0], 60)); // enableVisionOnlyStops off
}

void test_handle_result_tracks_send_failure() {
  HW3Handler handler;
  can_frame frame = makeFrame(1016, 0, 0, 0, 0, 0, 1u << 5);
  sendSucceeds = false;
  HandleResult result = handler.handleMessage(frame, makeSink());
  ASSERT_TRUE(result.handled);
  ASSERT_TRUE(result.attemptedSend);
  ASSERT_FALSE(result.sent);
  ASSERT_EQ((int)sentFrames.size(), 1);
}

int main() {
  printf("=== Utility tests ===\n");
  RUN(test_setBit_sets_and_preserves);
  RUN(test_setBit_clears_and_preserves);
  RUN(test_setBit_cross_byte_boundaries);
  RUN(test_readMuxID_extracts_lower_3_bits);
  RUN(test_isFSDSelectedInUI_reads_bit38);
  RUN(test_setSpeedProfileV12V13_writes_bits_1_2);
  RUN(test_readDrivingSide_extracts_bits_40_41);
  RUN(test_readApmv3Branch_extracts_bits_40_42);

  printf("\n=== LegacyHandler tests ===\n");
  RUN(test_legacy_unhandled_id_returns_false);
  RUN(test_legacy_1006_mux0_sets_bit46_and_speed_profile);
  RUN(test_legacy_1006_mux1_clears_eceR79);
  RUN(test_legacy_2047_sets_autopilot_to_3);

  printf("\n=== HW3Handler tests ===\n");
  RUN(test_hw3_unhandled_id_returns_false);
  RUN(test_hw3_1016_enables_nav_on_maps_bits);
  RUN(test_hw3_1016_follow_distance_1);
  RUN(test_hw3_1016_follow_distance_3);
  RUN(test_hw3_1021_mux0_sets_fsdStops_and_bit46);
  RUN(test_hw3_1021_mux0_preserves_speedProfile_from_1016);
  RUN(test_hw3_1021_mux0_runs_without_fsd_gate);
  RUN(test_hw3_1021_mux1_sets_branch_and_lane_graph_bits);
  RUN(test_hw3_1021_mux2_writes_speed_offset);
  RUN(test_hw3_2047_sets_autopilot_to_3);

  printf("\n=== HW4Handler tests ===\n");
  RUN(test_hw4_unhandled_id_returns_false);
  RUN(test_hw4_1016_enables_nav_on_maps_bits);
  RUN(test_hw4_1016_follow_distance_mapping);
  RUN(test_hw4_1021_mux0_sets_bits_38_46_60);
  RUN(test_hw4_1021_mux1_sets_branch_and_bit47);
  RUN(test_hw4_1021_mux2_writes_speed_profile);
  RUN(test_hw4_2047_sets_autopilot_to_4);

  printf("\n=== navStopsEnabled toggle tests ===\n");
  RUN(test_hw3_navStopsDisabled_clears_nav_bits);
  RUN(test_hw3_navStopsDisabled_clears_fsdStops);
  RUN(test_hw4_navStopsDisabled_clears_nav_bits);
  RUN(test_hw4_navStopsDisabled_clears_fsdStops_and_visionStops);

  printf("\n=== Result tests ===\n");
  RUN(test_handle_result_tracks_send_failure);

  printf("\n========================================\n");
  printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);
  return testsFailed > 0 ? 1 : 0;
}
