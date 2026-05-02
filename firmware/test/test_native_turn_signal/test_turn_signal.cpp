// ── Turn Signal Tests ────────────────────────────────────────────────────────
// Tests turn signal CAN frame bit manipulation and command parsing.

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
#include "infra/can.h"

// ── Stubs ────────────────────────────────────────────────────────────────────
void saveSettings(const State &) {}
void sendLog(const char *) {}
void sendLog(const __FlashStringHelper *) {}

struct SendCall
{
	Frame f;
	uint8_t bus;
};
static SendCall stub_sends[32];
static uint8_t stub_send_count = 0;

void driverSend(const Frame &f, uint8_t bus)
{
	if (stub_send_count < 32)
	{
		stub_sends[stub_send_count].f = f;
		stub_sends[stub_send_count].bus = bus;
		stub_send_count++;
	}
}

#include "feature/turn_signal.h"

void setUp()
{
	stub_send_count = 0;
}
void tearDown() {}

// ── setTurnSignalRequest ────────────────────────────────────────────────────

void test_set_turn_off()
{
	Frame f = {};
	f.data[0] = 0xFF;
	setTurnSignalRequest(f, TURN_OFF);
	TEST_ASSERT_EQUAL_HEX8(0xFC, f.data[0]); // bits 1:0 cleared
}

void test_set_turn_left()
{
	Frame f = {};
	setTurnSignalRequest(f, TURN_LEFT_3);
	TEST_ASSERT_EQUAL_HEX8(0x01, f.data[0] & 0x03);
}

void test_set_turn_right()
{
	Frame f = {};
	setTurnSignalRequest(f, TURN_RIGHT_3);
	TEST_ASSERT_EQUAL_HEX8(0x02, f.data[0] & 0x03);
}

void test_set_turn_hazard()
{
	Frame f = {};
	setTurnSignalRequest(f, TURN_HAZARD);
	TEST_ASSERT_EQUAL_HEX8(0x03, f.data[0] & 0x03);
}

void test_set_preserves_upper_bits()
{
	Frame f = {};
	f.data[0] = 0xA8; // 10101000
	setTurnSignalRequest(f, TURN_LEFT_3);
	TEST_ASSERT_EQUAL_HEX8(0xA9, f.data[0]); // upper bits preserved, low 2 = 01
}

// ── enum values ─────────────────────────────────────────────────────────────

void test_enum_values()
{
	TEST_ASSERT_EQUAL(0, TURN_OFF);
	TEST_ASSERT_EQUAL(1, TURN_LEFT_3);
	TEST_ASSERT_EQUAL(2, TURN_RIGHT_3);
	TEST_ASSERT_EQUAL(3, TURN_HAZARD);
}

// ── execTurnSignalCmd ───────────────────────────────────────────────────────

void test_cmd_left3()
{
	State s = {};
	s.hasCtrl = true;
	TEST_ASSERT_TRUE(execTurnSignalCmd("turn:left3", s));
}

void test_cmd_right3()
{
	State s = {};
	s.hasCtrl = true;
	TEST_ASSERT_TRUE(execTurnSignalCmd("turn:right3", s));
}

void test_cmd_hazard()
{
	State s = {};
	s.hasCtrl = true;
	TEST_ASSERT_TRUE(execTurnSignalCmd("turn:hazard", s));
}

void test_cmd_off()
{
	State s = {};
	s.hasCtrl = true;
	TEST_ASSERT_TRUE(execTurnSignalCmd("turn:off", s));
}

void test_cmd_unknown_returns_false()
{
	State s = {};
	TEST_ASSERT_FALSE(execTurnSignalCmd("turn:invalid", s));
}

// ── runtime decode helpers (D-05) ─────────────────────────────────────────

void test_decode_turn_signal_left_active()
{
	Frame f = {};
	f.dlc = 8;
	f.data[6] = (1 << 2);
	TEST_ASSERT_TRUE(decodeTurnSignalLeftActive(f));
}

void test_decode_turn_signal_right_active()
{
	Frame f = {};
	f.dlc = 8;
	f.data[6] = (2 << 4);
	TEST_ASSERT_TRUE(decodeTurnSignalRightActive(f));
}

void test_decode_turn_signals_inactive_when_off()
{
	Frame f = {};
	f.dlc = 8;
	f.data[6] = 0;
	TEST_ASSERT_FALSE(decodeTurnSignalLeftActive(f));
	TEST_ASSERT_FALSE(decodeTurnSignalRightActive(f));
}

void test_decode_blind_spot_levels()
{
	Frame f = {};
	f.dlc = 8;
	f.data[0] = (2 << 4) | (1 << 6);
	TEST_ASSERT_EQUAL_UINT8(2, decodeBlindSpotLeftLevel(f));
	TEST_ASSERT_EQUAL_UINT8(1, decodeBlindSpotRightLevel(f));
}

void test_decode_blind_spot_sna_maps_to_zero()
{
	Frame f = {};
	f.dlc = 8;
	f.data[0] = (3 << 4) | (3 << 6);
	TEST_ASSERT_EQUAL_UINT8(0, decodeBlindSpotLeftLevel(f));
	TEST_ASSERT_EQUAL_UINT8(0, decodeBlindSpotRightLevel(f));
}

// ── main ────────────────────────────────────────────────────────────────────

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_set_turn_off);
	RUN_TEST(test_set_turn_left);
	RUN_TEST(test_set_turn_right);
	RUN_TEST(test_set_turn_hazard);
	RUN_TEST(test_set_preserves_upper_bits);
	RUN_TEST(test_enum_values);
	RUN_TEST(test_cmd_left3);
	RUN_TEST(test_cmd_right3);
	RUN_TEST(test_cmd_hazard);
	RUN_TEST(test_cmd_off);
	RUN_TEST(test_cmd_unknown_returns_false);
	RUN_TEST(test_decode_turn_signal_left_active);
	RUN_TEST(test_decode_turn_signal_right_active);
	RUN_TEST(test_decode_turn_signals_inactive_when_off);
	RUN_TEST(test_decode_blind_spot_levels);
	RUN_TEST(test_decode_blind_spot_sna_maps_to_zero);
	return UNITY_END();
}
