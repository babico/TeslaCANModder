// ── Wiper Persist Tests ─────────────────────────────────────────────────────
// Tests wiper speed persistence save/restore and command handling.

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
#include "vehicle/can/ids.h"

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

#include "feature/wiper.h"

void setUp()
{
	stub_send_count = 0;
}
void tearDown() {}

// ── wiperPersistSave ────────────────────────────────────────────────────────

void test_save_stores_speed_when_enabled()
{
	State s = {};
	s.wiperPersistEnabled = true;
	wiperPersistSave(s, 3);
	TEST_ASSERT_EQUAL(3, s.savedWiperSpeed);
}

void test_save_does_nothing_when_disabled()
{
	State s = {};
	s.wiperPersistEnabled = false;
	s.savedWiperSpeed = 0;
	wiperPersistSave(s, 5);
	TEST_ASSERT_EQUAL(0, s.savedWiperSpeed);
}

// ── wiperPersistRestore ─────────────────────────────────────────────────────

void test_restore_sends_frame_when_enabled()
{
	State s = {};
	s.wiperPersistEnabled = true;
	s.savedWiperSpeed = 2;
	s.hasCtrl = true;
	wiperPersistRestore(s);
	TEST_ASSERT_TRUE(s.burstRemaining > 0);
}

void test_restore_skips_when_disabled()
{
	State s = {};
	s.wiperPersistEnabled = false;
	s.savedWiperSpeed = 2;
	s.hasCtrl = true;
	wiperPersistRestore(s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_restore_skips_when_speed_zero()
{
	State s = {};
	s.wiperPersistEnabled = true;
	s.savedWiperSpeed = 0;
	s.hasCtrl = true;
	wiperPersistRestore(s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_restore_skips_without_ctrl()
{
	State s = {};
	s.wiperPersistEnabled = true;
	s.savedWiperSpeed = 2;
	s.hasCtrl = false;
	wiperPersistRestore(s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

// ── executeWiperPersistCmd ─────────────────────────────────────────────────────

void test_cmd_on()
{
	State s = {};
	TEST_ASSERT_TRUE(executeWiperPersistCmd("wiperpersist:on", s));
	TEST_ASSERT_TRUE(s.wiperPersistEnabled);
}

void test_cmd_off()
{
	State s = {};
	s.wiperPersistEnabled = true;
	TEST_ASSERT_TRUE(executeWiperPersistCmd("wiperpersist:off", s));
	TEST_ASSERT_FALSE(s.wiperPersistEnabled);
}

void test_cmd_unknown()
{
	State s = {};
	TEST_ASSERT_FALSE(executeWiperPersistCmd("wiperpersist:toggle", s));
}

// ── main ────────────────────────────────────────────────────────────────────

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_save_stores_speed_when_enabled);
	RUN_TEST(test_save_does_nothing_when_disabled);
	RUN_TEST(test_restore_sends_frame_when_enabled);
	RUN_TEST(test_restore_skips_when_disabled);
	RUN_TEST(test_restore_skips_when_speed_zero);
	RUN_TEST(test_restore_skips_without_ctrl);
	RUN_TEST(test_cmd_on);
	RUN_TEST(test_cmd_off);
	RUN_TEST(test_cmd_unknown);
	return UNITY_END();
}
