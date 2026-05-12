/** @file firmware/test/test_native_hw3/test_hw3.cpp
 *  @brief Unit tests for HW3 variant frame handler
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
#define BOARD_ENABLE_BLE 0
#define BOARD_ENABLE_WIFI 0

#include "core/types.h"
#include "vehicle/can/ids.h"
#include "feature/profile.h"
#include "feature/offsets.h"

#include "support/stubs.h"

#include "handler/variant/hw3.h"

#include "support/helpers.h"

static State makeState()
{
	State s = {};
	s.variant = HW3;
	s.speedProfile = 1;
	s.fsdEnabled = true;
	s.nagMode = NAG_MODE_BIT19;
	return s;
}

void setUp()
{
	stub_send_count = 0;
	resetHW3LogFlags();
}
void tearDown() {}


void test_hw3_follow_distance_1_sets_profile_2()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	f.data[5] = 0b00100000;
	handleHW3(f, s);
	TEST_ASSERT_EQUAL_INT(2, s.speedProfile);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_hw3_follow_distance_2_sets_profile_1()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	f.data[5] = 0b01000000;
	handleHW3(f, s);
	TEST_ASSERT_EQUAL_INT(1, s.speedProfile);
}

void test_hw3_follow_distance_3_sets_profile_0()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	f.data[5] = 0b01100000;
	handleHW3(f, s);
	TEST_ASSERT_EQUAL_INT(0, s.speedProfile);
}

void test_hw3_follow_distance_0_keeps_default()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	f.data[5] = 0x00;
	handleHW3(f, s);
	TEST_ASSERT_EQUAL_INT(1, s.speedProfile);
}

void test_hw3_follow_dist_pinned_unchanged()
{
	State s = makeState();
	s.profileOverride = true;
	s.speedProfile = 2;
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	f.data[5] = 0b01100000;
	handleHW3(f, s);
	TEST_ASSERT_EQUAL_INT(2, s.speedProfile);
}


void test_hw3_fsd_mux0_sends_with_bit46()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x00;
	f.data[4] = 0x40;
	handleHW3(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL_HEX8(0x40, stub_sends[0].f.data[5] & 0x40);
}

void test_hw3_fsd_mux0_no_send_when_disabled()
{
	State s = makeState();
	s.fsdEnabled = false;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x00;
	f.data[4] = 0x40;
	handleHW3(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_hw3_fsd_mux0_no_send_when_ui_not_selected()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x00;
	f.data[4] = 0x00;
	handleHW3(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_hw3_fsd_mux0_blocked_when_ap_gate_closed()
{
	State s = makeState();
	s.apInjectionGateEnabled = true;
	s.apGateApActive = false;
	s.apGateParked = false;
	s.apGateSummoning = false;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x00;
	f.data[4] = 0x40;
	handleHW3(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_hw3_fsd_mux0_allows_when_ap_gate_open_by_park()
{
	State s = makeState();
	s.apInjectionGateEnabled = true;
	s.apGateApActive = false;
	s.apGateParked = true;
	s.apGateSummoning = false;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x00;
	f.data[4] = 0x40;
	handleHW3(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
}


void test_hw3_nag_mux1_clears_bit19()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x01;
	setBit(f, 19, true);
	handleHW3(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_FALSE((stub_sends[0].f.data[2] >> 3) & 0x01);
}

void test_hw3_nag_mux1_no_send_when_disabled()
{
	State s = makeState();
	s.nagMode = NAG_MODE_OFF;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x01;
	handleHW3(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}


void test_hw3_mux2_sends_when_fsd_enabled()
{
	State s = makeState();
	s.speedOffset = 25;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x02;
	f.data[4] = 0x40;
	handleHW3(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
}

void test_hw3_mux2_no_send_when_fsd_disabled()
{
	State s = makeState();
	s.fsdEnabled = false;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x02;
	f.data[4] = 0x00;
	handleHW3(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}


void test_hw3_ignores_unrelated_id()
{
	State s = makeState();
	Frame f = makeFrame(999);
	handleHW3(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_hw3_sends_on_bus_0()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x00;
	f.data[4] = 0x40;
	handleHW3(f, s);
	TEST_ASSERT_EQUAL(BUS_CHASSIS, stub_sends[0].bus);
}


void test_hw3_assist_nav_sets_bits_13_48_49()
{
	State s = makeState();
	s.assistNavEnable = true;
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	handleHW3(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL_HEX8(0x20, stub_sends[0].f.data[1] & 0x20);
	TEST_ASSERT_EQUAL_HEX8(0x01, stub_sends[0].f.data[6] & 0x01);
	TEST_ASSERT_EQUAL_HEX8(0x02, stub_sends[0].f.data[6] & 0x02);
}

void test_hw3_assist_nav_blocked_when_ap_gate_closed()
{
	State s = makeState();
	s.assistNavEnable = true;
	s.apInjectionGateEnabled = true;
	s.apGateApActive = false;
	s.apGateParked = false;
	s.apGateSummoning = false;
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	handleHW3(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_hw3_assist_hands_off_sets_bit14()
{
	State s = makeState();
	s.assistHandsOff = true;
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	handleHW3(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL_HEX8(0x40, stub_sends[0].f.data[1] & 0x40);
}

void test_hw3_assist_dev_mode_sets_bit5()
{
	State s = makeState();
	s.assistDevMode = true;
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	handleHW3(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL_HEX8(0x20, stub_sends[0].f.data[0] & 0x20);
}

void test_hw3_assist_tel_off_clears_bit43()
{
	State s = makeState();
	s.assistTelemetryOff = true;
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	f.data[5] = 0xFF;
	handleHW3(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL_HEX8(0x00, stub_sends[0].f.data[5] & 0x08);
}

void test_hw3_lane_graph_sets_bit45_on_mux1()
{
	State s = makeState();
	s.laneGraphEnable = true;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x01;
	f.data[4] = 0x40;
	handleHW3(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL_HEX8(0x20, stub_sends[0].f.data[5] & 0x20);
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_hw3_follow_distance_1_sets_profile_2);
	RUN_TEST(test_hw3_follow_distance_2_sets_profile_1);
	RUN_TEST(test_hw3_follow_distance_3_sets_profile_0);
	RUN_TEST(test_hw3_follow_distance_0_keeps_default);
	RUN_TEST(test_hw3_follow_dist_pinned_unchanged);

	RUN_TEST(test_hw3_fsd_mux0_sends_with_bit46);
	RUN_TEST(test_hw3_fsd_mux0_no_send_when_disabled);
	RUN_TEST(test_hw3_fsd_mux0_no_send_when_ui_not_selected);
	RUN_TEST(test_hw3_fsd_mux0_blocked_when_ap_gate_closed);
	RUN_TEST(test_hw3_fsd_mux0_allows_when_ap_gate_open_by_park);

	RUN_TEST(test_hw3_nag_mux1_clears_bit19);
	RUN_TEST(test_hw3_nag_mux1_no_send_when_disabled);

	RUN_TEST(test_hw3_mux2_sends_when_fsd_enabled);
	RUN_TEST(test_hw3_mux2_no_send_when_fsd_disabled);

	RUN_TEST(test_hw3_ignores_unrelated_id);
	RUN_TEST(test_hw3_sends_on_bus_0);

	RUN_TEST(test_hw3_assist_nav_sets_bits_13_48_49);
	RUN_TEST(test_hw3_assist_nav_blocked_when_ap_gate_closed);
	RUN_TEST(test_hw3_assist_hands_off_sets_bit14);
	RUN_TEST(test_hw3_assist_dev_mode_sets_bit5);
	RUN_TEST(test_hw3_assist_tel_off_clears_bit43);
	RUN_TEST(test_hw3_lane_graph_sets_bit45_on_mux1);

	return UNITY_END();
}

