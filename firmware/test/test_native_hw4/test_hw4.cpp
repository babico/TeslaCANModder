/** @file firmware/test/test_native_hw4/test_hw4.cpp
 *  @brief Unit tests for HW4 variant frame handler
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
#include "feature/fsd/profile.h"
#include "feature/fsd/isa_chime.h"

#include "support/stubs.h"

#include "handler/variant/hw4.h"

#include "support/helpers.h"

static State makeState()
{
	State s = {};
	s.variant = HW4;
	s.speedProfile = 1;
	s.fsdEnabled = true;
	s.nagMode = NAG_MODE_BIT19;
	s.isaChimeSuppress = false;
	return s;
}

void setUp()
{
	stub_send_count = 0;
	resetHW4LogFlags();
}
void tearDown() {}


void test_hw4_isa_suppress_disabled_ignores_921()
{
	State s = makeState();
	s.isaChimeSuppress = false;
	Frame f = makeFrame(CAN_ID_ISA_SPEED);
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_hw4_isa_suppress_enabled_sets_bit5_of_data1()
{
	State s = makeState();
	s.isaChimeSuppress = true;
	Frame f = makeFrame(CAN_ID_ISA_SPEED);
	f.data[1] = 0x00;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL_HEX8(0x20, stub_sends[0].f.data[1] & 0x20);
}

void test_hw4_isa_suppress_short_frame_ignored()
{
	State s = makeState();
	s.isaChimeSuppress = true;
	Frame f = makeFrame(CAN_ID_ISA_SPEED, 7);
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_hw4_isa_suppress_preserves_existing_bits()
{
	State s = makeState();
	s.isaChimeSuppress = true;
	Frame f = makeFrame(CAN_ID_ISA_SPEED);
	f.data[1] = 0xC3;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL_HEX8(0xE3, stub_sends[0].f.data[1]);
}

void test_hw4_isa_suppress_checksum()
{
	State s = makeState();
	s.isaChimeSuppress = true;
	Frame f = makeFrame(CAN_ID_ISA_SPEED);
	f.data[0] = 0x10;
	f.data[1] = 0x05;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL_HEX8(0xD1, stub_sends[0].f.data[7]);
}


void test_hw4_follow_distance_1_sets_profile_3()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	f.data[5] = 0b00100000;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL_INT(3, s.speedProfile);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_hw4_follow_distance_2_sets_profile_2()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	f.data[5] = 0b01000000;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL_INT(2, s.speedProfile);
}

void test_hw4_follow_distance_3_sets_profile_1()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	f.data[5] = 0b01100000;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL_INT(1, s.speedProfile);
}

void test_hw4_follow_distance_4_sets_profile_0()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	f.data[5] = 0b10000000;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL_INT(0, s.speedProfile);
}

void test_hw4_follow_distance_5_sets_profile_4()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	f.data[5] = 0b10100000;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL_INT(4, s.speedProfile);
}

void test_hw4_follow_dist_pinned_unchanged()
{
	State s = makeState();
	s.profileOverride = true;
	s.speedProfile = 3;
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	f.data[5] = 0b01000000;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL_INT(3, s.speedProfile);
}


void test_hw4_fsd_mux0_sets_bits_46_and_60()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x00;
	f.data[4] = 0x40;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL_HEX8(0x40, stub_sends[0].f.data[5] & 0x40);
	TEST_ASSERT_EQUAL_HEX8(0x10, stub_sends[0].f.data[7] & 0x10);
}

void test_hw4_fsd_mux0_no_send_when_disabled()
{
	State s = makeState();
	s.fsdEnabled = false;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x00;
	f.data[4] = 0x40;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_hw4_fsd_mux0_no_send_when_ui_not_selected()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x00;
	f.data[4] = 0x00;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_hw4_fsd_mux0_blocked_when_ap_gate_closed()
{
	State s = makeState();
	s.apInjectionGateEnabled = true;
	s.apGateApActive = false;
	s.apGateParked = false;
	s.apGateSummoning = false;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x00;
	f.data[4] = 0x40;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_hw4_fsd_mux0_allows_when_ap_gate_open_by_ap_active()
{
	State s = makeState();
	s.apInjectionGateEnabled = true;
	s.apGateApActive = true;
	s.apGateParked = false;
	s.apGateSummoning = false;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x00;
	f.data[4] = 0x40;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
}


void test_hw4_nag_mux1_clears_bit19_sets_bit47()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x01;
	setBit(f, 19, true);
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_FALSE((stub_sends[0].f.data[2] >> 3) & 0x01);
	TEST_ASSERT_EQUAL_HEX8(0x80, stub_sends[0].f.data[5] & 0x80);
}

void test_hw4_nag_mux1_no_send_when_disabled()
{
	State s = makeState();
	s.nagMode = NAG_MODE_OFF;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x01;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}


void test_hw4_mux2_injects_speed_profile()
{
	State s = makeState();
	s.speedProfile = 3;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x02;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	uint8_t injected = (stub_sends[0].f.data[7] >> 4) & 0x07;
	TEST_ASSERT_EQUAL_UINT8(3, injected);
}

void test_hw4_mux2_clears_old_profile_bits()
{
	State s = makeState();
	s.speedProfile = 0;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x02;
	f.data[7] = 0x70;
	handleHW4(f, s);
	uint8_t injected = (stub_sends[0].f.data[7] >> 4) & 0x07;
	TEST_ASSERT_EQUAL_UINT8(0, injected);
}

void test_hw4_mux2_applies_hw4_offset_override_when_enabled()
{
	State s = makeState();
	s.speedOffset = 17;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x02;
	f.data[1] = 0xC0;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL_HEX8(0xD1, stub_sends[0].f.data[1]);
}

void test_hw4_mux2_leaves_data1_when_hw4_offset_disabled()
{
	State s = makeState();
	s.speedOffset = 0;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x02;
	f.data[1] = 0xAB;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL_HEX8(0xAB, stub_sends[0].f.data[1]);
}

void test_hw4_ignores_unrelated_id()
{
	State s = makeState();
	Frame f = makeFrame(999);
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_hw4_sends_on_bus_0()
{
	State s = makeState();
	s.isaChimeSuppress = true;
	Frame f = makeFrame(CAN_ID_ISA_SPEED);
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(BUS_CHASSIS, stub_sends[0].bus);
}


void test_hw4_eap_sets_bit46_on_mux1()
{
	State s = makeState();
	s.enhancedAutopilot = true;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x01;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL_HEX8(0x40, stub_sends[0].f.data[5] & 0x40);
}

void test_hw4_eap_off_does_not_set_bit46_on_mux1()
{
	State s = makeState();
	s.enhancedAutopilot = false;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x01;
	f.data[5] = 0x00;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL_HEX8(0x00, stub_sends[0].f.data[5] & 0x40);
}

void test_hw4_eap_does_not_affect_mux0()
{
	State s = makeState();
	s.fsdEnabled = false;
	s.nagMode = NAG_MODE_OFF;
	s.enhancedAutopilot = true;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x00;
	f.data[4] = 0x40;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL_INT(0, stub_send_count);
}


void test_hw4_evd_sets_bit59_on_mux0()
{
	State s = makeState();
	s.evdEnabled = true;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x00;
	f.data[4] = 0x40;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL_HEX8(0x08, stub_sends[0].f.data[7] & 0x08);
}

void test_hw4_evd_off_does_not_set_bit59()
{
	State s = makeState();
	s.evdEnabled = false;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x00;
	f.data[4] = 0x40;
	f.data[7] = 0x00;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL_HEX8(0x00, stub_sends[0].f.data[7] & 0x08);
}


void test_hw4_assist_nav_sets_bits_13_48_49()
{
	State s = makeState();
	s.assistNavEnable = true;
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL_HEX8(0x20, stub_sends[0].f.data[1] & 0x20);
	TEST_ASSERT_EQUAL_HEX8(0x01, stub_sends[0].f.data[6] & 0x01);
	TEST_ASSERT_EQUAL_HEX8(0x02, stub_sends[0].f.data[6] & 0x02);
}

void test_hw4_assist_nav_off_does_not_send()
{
	State s = makeState();
	s.assistNavEnable = false;
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_hw4_assist_nav_blocked_when_ap_gate_closed()
{
	State s = makeState();
	s.assistNavEnable = true;
	s.apInjectionGateEnabled = true;
	s.apGateApActive = false;
	s.apGateParked = false;
	s.apGateSummoning = false;
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}


void test_hw4_assist_hands_off_sets_bit14()
{
	State s = makeState();
	s.assistHandsOff = true;
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL_HEX8(0x40, stub_sends[0].f.data[1] & 0x40);
}

void test_hw4_assist_hands_off_disabled_no_bit14()
{
	State s = makeState();
	s.assistHandsOff = false;
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	f.data[1] = 0x00;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}


void test_hw4_assist_dev_mode_sets_bit5()
{
	State s = makeState();
	s.assistDevMode = true;
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL_HEX8(0x20, stub_sends[0].f.data[0] & 0x20);
}

void test_hw4_assist_dev_mode_disabled_no_bit5()
{
	State s = makeState();
	s.assistDevMode = false;
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	f.data[0] = 0x00;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}


void test_hw4_lane_graph_sets_bit45_on_mux1()
{
	State s = makeState();
	s.laneGraphEnable = true;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x01;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL_HEX8(0x20, stub_sends[0].f.data[5] & 0x20);
}

void test_hw4_lane_graph_disabled_no_bit45()
{
	State s = makeState();
	s.laneGraphEnable = false;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x01;
	f.data[5] = 0x00;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL_HEX8(0x00, stub_sends[0].f.data[5] & 0x20);
}


void test_hw4_assist_telemetry_off_clears_bit43()
{
	State s = makeState();
	s.assistTelemetryOff = true;
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	f.data[5] = 0xFF;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL_HEX8(0x00, stub_sends[0].f.data[5] & 0x08);
}

void test_hw4_assist_telemetry_off_disabled_preserves_bit43()
{
	State s = makeState();
	s.assistTelemetryOff = false;
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	f.data[5] = 0x08;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}


void test_hw4_assist_nav_and_hands_off_combined()
{
	State s = makeState();
	s.assistNavEnable = true;
	s.assistHandsOff = true;
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL_HEX8(0x20, stub_sends[0].f.data[1] & 0x20);
	TEST_ASSERT_EQUAL_HEX8(0x40, stub_sends[0].f.data[1] & 0x40);
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_hw4_isa_suppress_disabled_ignores_921);
	RUN_TEST(test_hw4_isa_suppress_enabled_sets_bit5_of_data1);
	RUN_TEST(test_hw4_isa_suppress_short_frame_ignored);
	RUN_TEST(test_hw4_isa_suppress_preserves_existing_bits);
	RUN_TEST(test_hw4_isa_suppress_checksum);

	RUN_TEST(test_hw4_follow_distance_1_sets_profile_3);
	RUN_TEST(test_hw4_follow_distance_2_sets_profile_2);
	RUN_TEST(test_hw4_follow_distance_3_sets_profile_1);
	RUN_TEST(test_hw4_follow_distance_4_sets_profile_0);
	RUN_TEST(test_hw4_follow_distance_5_sets_profile_4);
	RUN_TEST(test_hw4_follow_dist_pinned_unchanged);

	RUN_TEST(test_hw4_fsd_mux0_sets_bits_46_and_60);
	RUN_TEST(test_hw4_fsd_mux0_no_send_when_disabled);
	RUN_TEST(test_hw4_fsd_mux0_no_send_when_ui_not_selected);
	RUN_TEST(test_hw4_fsd_mux0_blocked_when_ap_gate_closed);
	RUN_TEST(test_hw4_fsd_mux0_allows_when_ap_gate_open_by_ap_active);

	RUN_TEST(test_hw4_nag_mux1_clears_bit19_sets_bit47);
	RUN_TEST(test_hw4_nag_mux1_no_send_when_disabled);

	RUN_TEST(test_hw4_mux2_injects_speed_profile);
	RUN_TEST(test_hw4_mux2_clears_old_profile_bits);
	RUN_TEST(test_hw4_mux2_applies_hw4_offset_override_when_enabled);
	RUN_TEST(test_hw4_mux2_leaves_data1_when_hw4_offset_disabled);

	RUN_TEST(test_hw4_ignores_unrelated_id);
	RUN_TEST(test_hw4_sends_on_bus_0);

	RUN_TEST(test_hw4_eap_sets_bit46_on_mux1);
	RUN_TEST(test_hw4_eap_off_does_not_set_bit46_on_mux1);
	RUN_TEST(test_hw4_eap_does_not_affect_mux0);

	RUN_TEST(test_hw4_evd_sets_bit59_on_mux0);
	RUN_TEST(test_hw4_evd_off_does_not_set_bit59);

	RUN_TEST(test_hw4_assist_nav_sets_bits_13_48_49);
	RUN_TEST(test_hw4_assist_nav_off_does_not_send);
	RUN_TEST(test_hw4_assist_nav_blocked_when_ap_gate_closed);

	RUN_TEST(test_hw4_assist_hands_off_sets_bit14);
	RUN_TEST(test_hw4_assist_hands_off_disabled_no_bit14);

	RUN_TEST(test_hw4_assist_dev_mode_sets_bit5);
	RUN_TEST(test_hw4_assist_dev_mode_disabled_no_bit5);

	RUN_TEST(test_hw4_lane_graph_sets_bit45_on_mux1);
	RUN_TEST(test_hw4_lane_graph_disabled_no_bit45);

	RUN_TEST(test_hw4_assist_telemetry_off_clears_bit43);
	RUN_TEST(test_hw4_assist_telemetry_off_disabled_preserves_bit43);

	RUN_TEST(test_hw4_assist_nav_and_hands_off_combined);

	return UNITY_END();
}

