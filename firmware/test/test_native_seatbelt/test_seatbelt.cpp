/** @file firmware/test/test_native_seatbelt/test_seatbelt.cpp
 *  @brief Unit tests for seatbelt emulation
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
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

#include "core/types.h"
#include "vehicle/can/ids.h"

void saveSettings(const State &) {}
void sendLog(const char *) {}
void sendLog(const __FlashStringHelper *) {}

static unsigned long fake_millis_val = 0;
unsigned long millis()
{
	return fake_millis_val;
}

struct SendCall
{
	Frame f;
	uint8_t bus;
};
static SendCall stub_sends[16];
static uint8_t stub_send_count = 0;

void driverSend(const Frame &f, uint8_t bus)
{
	if (stub_send_count < 16)
	{
		stub_sends[stub_send_count].f = f;
		stub_sends[stub_send_count].bus = bus;
		stub_send_count++;
	}
}

#include "feature/seatbelt.h"

void setUp()
{
	stub_send_count = 0;
	fake_millis_val = 0;
}
void tearDown() {}


void test_enable_sets_flag()
{
	State s = {};
	controlSeatbeltEmulation(s, true);
	TEST_ASSERT_TRUE(s.seatbeltEmulation);
}

void test_disable_clears_flag()
{
	State s = {};
	s.seatbeltEmulation = true;
	controlSeatbeltEmulation(s, false);
	TEST_ASSERT_FALSE(s.seatbeltEmulation);
}


void test_tick_does_nothing_when_disabled()
{
	State s = {};
	s.seatbeltEmulation = false;
	fake_millis_val = 1000;
	seatbeltEmulationTick(s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_tick_does_nothing_when_tx_paused()
{
	State s = {};
	s.seatbeltEmulation = true;
	s.txPaused = true;
	fake_millis_val = 1000;
	seatbeltEmulationTick(s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_tick_sends_frame_after_interval()
{
	State s = {};
	s.seatbeltEmulation = true;
	s.txPaused = false;
	s.seatbeltLastMs = 0;
	fake_millis_val = 600;
	seatbeltEmulationTick(s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL_HEX32(CAN_ID_SEATBELT_STATUS, stub_sends[0].f.id);
	TEST_ASSERT_EQUAL_HEX8(0x07, stub_sends[0].f.data[0]);
	TEST_ASSERT_EQUAL(BUS_VEHICLE, stub_sends[0].bus);
}

void test_tick_respects_interval()
{
	State s = {};
	s.seatbeltEmulation = true;
	s.txPaused = false;
	s.seatbeltLastMs = 500;
	fake_millis_val = 800;
	seatbeltEmulationTick(s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}


void test_cmd_on()
{
	State s = {};
	TEST_ASSERT_TRUE(executeSeatbeltCmd("seatbelt:on", s));
	TEST_ASSERT_TRUE(s.seatbeltEmulation);
}

void test_cmd_off()
{
	State s = {};
	s.seatbeltEmulation = true;
	TEST_ASSERT_TRUE(executeSeatbeltCmd("seatbelt:off", s));
	TEST_ASSERT_FALSE(s.seatbeltEmulation);
}

void test_cmd_unknown()
{
	State s = {};
	TEST_ASSERT_FALSE(executeSeatbeltCmd("seatbelt:toggle", s));
}


int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_enable_sets_flag);
	RUN_TEST(test_disable_clears_flag);
	RUN_TEST(test_tick_does_nothing_when_disabled);
	RUN_TEST(test_tick_does_nothing_when_tx_paused);
	RUN_TEST(test_tick_sends_frame_after_interval);
	RUN_TEST(test_tick_respects_interval);
	RUN_TEST(test_cmd_on);
	RUN_TEST(test_cmd_off);
	RUN_TEST(test_cmd_unknown);
	return UNITY_END();
}

