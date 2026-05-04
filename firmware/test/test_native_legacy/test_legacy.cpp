// ── Legacy Handler Tests ─────────────────────────────────────────────────────
// Tests handleLegacy() CAN frame processing: stalk profile, FSD, nag suppress.

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
#include "infra/can/bus.h"
#include "feature/profile.h"
#include "feature/offsets.h"

// ── Stubs ────────────────────────────────────────────────────────────────────
#include "support/stubs.h"
#include "handler/legacy.h"
#include "support/helpers.h"

static State makeState()
{
	State s = {};
	s.variant = LEGACY;
	s.speedProfile = 1;
	s.fsdEnabled = true;
	s.nagSuppress = true;
	return s;
}

void setUp()
{
	stub_send_count = 0;
	resetLegacyLogFlags();
}
void tearDown() {}

// ── Stalk Position → Profile ─────────────────────────────────────────────────

void test_legacy_stalk_pos0_sets_profile_2()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_LEGACY_STALK);
	f.data[1] = 0x00; // pos = 0
	handleLegacy(f, s);
	TEST_ASSERT_EQUAL_INT(2, s.speedProfile);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_legacy_stalk_pos1_sets_profile_2()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_LEGACY_STALK);
	f.data[1] = 0x21; // pos = 0x21 >> 5 = 1
	handleLegacy(f, s);
	TEST_ASSERT_EQUAL_INT(2, s.speedProfile);
}

void test_legacy_stalk_pos2_sets_profile_1()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_LEGACY_STALK);
	f.data[1] = 0x42; // pos = 0x42 >> 5 = 2
	handleLegacy(f, s);
	TEST_ASSERT_EQUAL_INT(1, s.speedProfile);
}

void test_legacy_stalk_pos3_sets_profile_0()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_LEGACY_STALK);
	f.data[1] = 0x64; // pos = 0x64 >> 5 = 3
	handleLegacy(f, s);
	TEST_ASSERT_EQUAL_INT(0, s.speedProfile);
}

void test_legacy_stalk_pinned_unchanged()
{
	State s = makeState();
	s.profileOverride = true;
	s.speedProfile = 2;
	Frame f = makeFrame(CAN_ID_LEGACY_STALK);
	f.data[1] = 0x64;
	handleLegacy(f, s);
	TEST_ASSERT_EQUAL_INT(2, s.speedProfile);
}

void test_legacy_stalk_short_frame_ignored()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_LEGACY_STALK, 1); // dlc < 2
	handleLegacy(f, s);
	TEST_ASSERT_EQUAL_INT(1, s.speedProfile); // unchanged
}

// ── FSD Mux 0 ────────────────────────────────────────────────────────────────

void test_legacy_fsd_mux0_sends_with_bit46()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_LEGACY_FSD_MUX);
	f.data[0] = 0x00;
	f.data[4] = 0x40;
	handleLegacy(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL_HEX8(0x40, stub_sends[0].f.data[5] & 0x40);
}

void test_legacy_fsd_mux0_no_send_when_disabled()
{
	State s = makeState();
	s.fsdEnabled = false;
	Frame f = makeFrame(CAN_ID_LEGACY_FSD_MUX);
	f.data[0] = 0x00;
	f.data[4] = 0x40;
	handleLegacy(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_legacy_fsd_mux0_no_send_when_ui_not_selected()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_LEGACY_FSD_MUX);
	f.data[0] = 0x00;
	f.data[4] = 0x00;
	handleLegacy(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_legacy_fsd_mux0_blocked_when_ap_gate_closed()
{
	State s = makeState();
	s.apInjectionGateEnabled = true;
	s.apGateApActive = false;
	s.apGateParked = false;
	s.apGateSummoning = false;
	Frame f = makeFrame(CAN_ID_LEGACY_FSD_MUX);
	f.data[0] = 0x00;
	f.data[4] = 0x40;
	handleLegacy(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_legacy_fsd_mux0_allows_when_ap_gate_open_by_summon()
{
	State s = makeState();
	s.apInjectionGateEnabled = true;
	s.apGateApActive = false;
	s.apGateParked = false;
	s.apGateSummoning = true;
	Frame f = makeFrame(CAN_ID_LEGACY_FSD_MUX);
	f.data[0] = 0x00;
	f.data[4] = 0x40;
	handleLegacy(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
}

void test_legacy_fsd_sets_speed_profile_in_frame()
{
	State s = makeState();
	s.speedProfile = 2;
	Frame f = makeFrame(CAN_ID_LEGACY_FSD_MUX);
	f.data[0] = 0x00;
	f.data[4] = 0x40;
	handleLegacy(f, s);
	TEST_ASSERT_EQUAL_HEX8(0x04, stub_sends[0].f.data[6] & 0x06);
}

// ── Nag Mux 1 ────────────────────────────────────────────────────────────────

void test_legacy_nag_mux1_clears_bit19()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_LEGACY_FSD_MUX);
	f.data[0] = 0x01;
	setBit(f, 19, true);
	handleLegacy(f, s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_FALSE((stub_sends[0].f.data[2] >> 3) & 0x01);
}

void test_legacy_nag_mux1_no_send_when_disabled()
{
	State s = makeState();
	s.nagSuppress = false;
	Frame f = makeFrame(CAN_ID_LEGACY_FSD_MUX);
	f.data[0] = 0x01;
	handleLegacy(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

// ── Misc ─────────────────────────────────────────────────────────────────────

void test_legacy_ignores_unrelated_id()
{
	State s = makeState();
	Frame f = makeFrame(999);
	handleLegacy(f, s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_legacy_sends_on_bus_0()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_LEGACY_FSD_MUX);
	f.data[0] = 0x00;
	f.data[4] = 0x40;
	handleLegacy(f, s);
	TEST_ASSERT_EQUAL(BUS_CHASSIS, stub_sends[0].bus);
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_legacy_stalk_pos0_sets_profile_2);
	RUN_TEST(test_legacy_stalk_pos1_sets_profile_2);
	RUN_TEST(test_legacy_stalk_pos2_sets_profile_1);
	RUN_TEST(test_legacy_stalk_pos3_sets_profile_0);
	RUN_TEST(test_legacy_stalk_pinned_unchanged);
	RUN_TEST(test_legacy_stalk_short_frame_ignored);

	RUN_TEST(test_legacy_fsd_mux0_sends_with_bit46);
	RUN_TEST(test_legacy_fsd_mux0_no_send_when_disabled);
	RUN_TEST(test_legacy_fsd_mux0_no_send_when_ui_not_selected);
	RUN_TEST(test_legacy_fsd_mux0_blocked_when_ap_gate_closed);
	RUN_TEST(test_legacy_fsd_mux0_allows_when_ap_gate_open_by_summon);
	RUN_TEST(test_legacy_fsd_sets_speed_profile_in_frame);

	RUN_TEST(test_legacy_nag_mux1_clears_bit19);
	RUN_TEST(test_legacy_nag_mux1_no_send_when_disabled);

	RUN_TEST(test_legacy_ignores_unrelated_id);
	RUN_TEST(test_legacy_sends_on_bus_0);

	return UNITY_END();
}
