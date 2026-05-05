// ── HW4 Handler Tests ────────────────────────────────────────────────────────
// Tests handleHW4() CAN frame processing: FSD activation, ISA chime, nag, profile.
// Stubs driverSend/sendLog to capture output without hardware.

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
#include "feature/isa_chime.h"

// ── Stubs ────────────────────────────────────────────────────────────────────
#include "support/stubs.h"

// Include handler (uses our stubs)
#include "handler/variant/hw4.h"

#include "support/helpers.h"

static State makeState()
{
	State s = {};
	s.variant = HW4;
	s.speedProfile = 1;
	s.fsdEnabled = true;
	s.nagSuppress = true;
	s.isaChimeSuppress = false;
	return s;
}

void setUp()
{
	stub_send_count = 0;
	resetHW4LogFlags();
}
void tearDown() {}

// ── ISA Speed Chime ──────────────────────────────────────────────────────────

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

// ── Follow Distance → Profile ────────────────────────────────────────────────

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

// ── FSD Mux 0 ────────────────────────────────────────────────────────────────

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

// ── Nag Mux 1 ────────────────────────────────────────────────────────────────

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
	s.nagSuppress = false;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x01;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

// ── Speed Profile Mux 2 ─────────────────────────────────────────────────────

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
	f.data[1] = 0xC0; // preserve upper 2 bits
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

// ── Enhanced Autopilot (EAP) bit 46 on mux=1 ─────────────────────────────────

void test_hw4_eap_sets_bit46_on_mux1()
{
	State s = makeState();
	s.enhancedAutopilot = true;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x01;
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	// bit 46 = byte 5, bit 6
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
	// EAP is mux=1 only; with fsdEnabled=false the mux=0 path is not taken
	// so no frame should be sent when only EAP is enabled
	State s = makeState();
	s.fsdEnabled = false;
	s.nagSuppress = false;
	s.enhancedAutopilot = true;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x00; // mux 0
	f.data[4] = 0x40; // UI selected bit
	handleHW4(f, s);
	TEST_ASSERT_EQUAL_INT(0, stub_send_count); // no send — EAP only applies on mux=1
}

// ── Emergency Vehicle Detection (EVD) bit 59 on mux=0 ────────────────────────

void test_hw4_evd_sets_bit59_on_mux0()
{
	State s = makeState();
	s.evdEnabled = true;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x00;
	f.data[4] = 0x40; // UI selected
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	// bit 59 = byte 7, bit 3
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

// ── P2-01 Assist Nav Enable (bits 13, 48, 49 on 0x3F8) ───────────────────────

void test_hw4_assist_nav_sets_bits_13_48_49()
{
	State s = makeState();
	s.assistNavEnable = true;
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	// bit 13 = byte 1 bit 5
	TEST_ASSERT_EQUAL_HEX8(0x20, stub_sends[0].f.data[1] & 0x20);
	// bit 48 = byte 6 bit 0
	TEST_ASSERT_EQUAL_HEX8(0x01, stub_sends[0].f.data[6] & 0x01);
	// bit 49 = byte 6 bit 1
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

// ── P2-02 Assist Hands-Off (bit 14 on 0x3F8) ─────────────────────────────────

void test_hw4_assist_hands_off_sets_bit14()
{
	State s = makeState();
	s.assistHandsOff = true;
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	// bit 14 = byte 1 bit 6
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

// ── P2-03 Assist Dev Mode (bit 5 on 0x3F8) ───────────────────────────────────

void test_hw4_assist_dev_mode_sets_bit5()
{
	State s = makeState();
	s.assistDevMode = true;
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	// bit 5 = byte 0 bit 5
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

// ── P2-04 Lane Graph (bit 45 on 0x3FD mux1) ─────────────────────────────────

void test_hw4_lane_graph_sets_bit45_on_mux1()
{
	State s = makeState();
	s.laneGraphEnable = true;
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	f.data[0] = 0x01; // mux 1
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	// bit 45 = byte 5 bit 5
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

// ── P2-05 Assist Telemetry Off (bit 43 cleared on 0x3F8) ─────────────────────

void test_hw4_assist_telemetry_off_clears_bit43()
{
	State s = makeState();
	s.assistTelemetryOff = true;
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	f.data[5] = 0xFF; // all bits set
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	// bit 43 = byte 5 bit 3
	TEST_ASSERT_EQUAL_HEX8(0x00, stub_sends[0].f.data[5] & 0x08);
}

void test_hw4_assist_telemetry_off_disabled_preserves_bit43()
{
	State s = makeState();
	s.assistTelemetryOff = false;
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	f.data[5] = 0x08; // bit 43 set
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

// ── Combined: multiple assist flags together ──────────────────────────────────

void test_hw4_assist_nav_and_hands_off_combined()
{
	State s = makeState();
	s.assistNavEnable = true;
	s.assistHandsOff = true;
	Frame f = makeFrame(CAN_ID_FOLLOW_DIST);
	handleHW4(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	// bit 13 = byte 1 bit 5
	TEST_ASSERT_EQUAL_HEX8(0x20, stub_sends[0].f.data[1] & 0x20);
	// bit 14 = byte 1 bit 6
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

	// P2-01: Assist Nav Enable
	RUN_TEST(test_hw4_assist_nav_sets_bits_13_48_49);
	RUN_TEST(test_hw4_assist_nav_off_does_not_send);
	RUN_TEST(test_hw4_assist_nav_blocked_when_ap_gate_closed);

	// P2-02: Assist Hands-Off
	RUN_TEST(test_hw4_assist_hands_off_sets_bit14);
	RUN_TEST(test_hw4_assist_hands_off_disabled_no_bit14);

	// P2-03: Assist Dev Mode
	RUN_TEST(test_hw4_assist_dev_mode_sets_bit5);
	RUN_TEST(test_hw4_assist_dev_mode_disabled_no_bit5);

	// P2-04: Lane Graph
	RUN_TEST(test_hw4_lane_graph_sets_bit45_on_mux1);
	RUN_TEST(test_hw4_lane_graph_disabled_no_bit45);

	// P2-05: Assist Telemetry Off
	RUN_TEST(test_hw4_assist_telemetry_off_clears_bit43);
	RUN_TEST(test_hw4_assist_telemetry_off_disabled_preserves_bit43);

	// P2-01+02 combined
	RUN_TEST(test_hw4_assist_nav_and_hands_off_combined);

	return UNITY_END();
}
