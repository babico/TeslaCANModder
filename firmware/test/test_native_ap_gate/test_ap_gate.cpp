#include <unity.h>
#include <cstring>

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

#include "core/types.h"
#include "infra/can/bus.h"

void saveSettings(const State &) {}

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

#include "feature/drive_mode.h"
#include "feature/seatbelt.h"
#include "infra/can/burst.h"

void setUp()
{
	fake_millis_val = 0;
	stub_send_count = 0;
}

void tearDown() {}

static void applyStandbyTransitionOnCanTimeout(State &s)
{
	s.chassisOnline = false;
	s.standby = true;
	s.hasCtrl = false;
	s.hasClimate = false;
	s.hasCharge = false;
	s.hasDrive = false;
	s.summonRemaining = 0;
	s.apGateSummoning = false;
	s.apGateParked = true;
}

void test_apgate_open_when_disabled()
{
	State s = {};
	s.apInjectionGateEnabled = false;
	s.apGateApActive = false;
	s.apGateParked = false;
	s.apGateSummoning = false;
	TEST_ASSERT_TRUE(s.apGateOpen());
}

void test_apgate_closed_waiting_state()
{
	State s = {};
	s.apInjectionGateEnabled = true;
	s.apGateApActive = false;
	s.apGateParked = false;
	s.apGateSummoning = false;
	TEST_ASSERT_FALSE(s.apGateOpen());
}

void test_apgate_open_ap_only()
{
	State s = {};
	s.apInjectionGateEnabled = true;
	s.apGateApActive = true;
	s.apGateParked = false;
	s.apGateSummoning = false;
	TEST_ASSERT_TRUE(s.apGateOpen());
}

void test_apgate_open_park_only()
{
	State s = {};
	s.apInjectionGateEnabled = true;
	s.apGateApActive = false;
	s.apGateParked = true;
	s.apGateSummoning = false;
	TEST_ASSERT_TRUE(s.apGateOpen());
}

void test_apgate_open_summon_only()
{
	State s = {};
	s.apInjectionGateEnabled = true;
	s.apGateApActive = false;
	s.apGateParked = false;
	s.apGateSummoning = true;
	TEST_ASSERT_TRUE(s.apGateOpen());
}

void test_drive_mode_tick_blocked_when_gate_closed()
{
	State s = {};
	s.hasDrive = true;
	s.driveModeOverride = DRIVE_MODE_CHILL;
	s.apInjectionGateEnabled = true;
	s.apGateApActive = false;
	s.apGateParked = false;
	s.apGateSummoning = false;
	fake_millis_val = 100;

	driveModeTick(s, fake_millis_val);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_drive_mode_tick_allows_when_ap_open()
{
	State s = {};
	s.hasDrive = true;
	s.driveModeOverride = DRIVE_MODE_STANDARD;
	s.apInjectionGateEnabled = true;
	s.apGateApActive = true;
	s.apGateParked = false;
	s.apGateSummoning = false;
	fake_millis_val = 100;

	driveModeTick(s, fake_millis_val);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL_UINT32(CAN_ID_DRIVE_MODE, stub_sends[0].f.id);
}

void test_seatbelt_tick_blocked_when_gate_closed()
{
	State s = {};
	s.seatbeltEmulation = true;
	s.apInjectionGateEnabled = true;
	s.apGateApActive = false;
	s.apGateParked = false;
	s.apGateSummoning = false;
	fake_millis_val = 700;

	seatbeltEmulationTick(s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

void test_seatbelt_tick_allows_when_summoning_open()
{
	State s = {};
	s.seatbeltEmulation = true;
	s.apInjectionGateEnabled = true;
	s.apGateApActive = false;
	s.apGateParked = false;
	s.apGateSummoning = true;
	fake_millis_val = 700;

	seatbeltEmulationTick(s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL_UINT32(CAN_ID_SEATBELT_STATUS, stub_sends[0].f.id);
	TEST_ASSERT_EQUAL(BUS_VEHICLE, stub_sends[0].bus);
}

void test_start_burst_blocked_when_gate_closed()
{
	State s = {};
	Frame f = {};
	f.id = CAN_ID_UI_VEHICLE_CTRL;
	f.dlc = 8;

	s.apInjectionGateEnabled = true;
	s.apGateApActive = false;
	s.apGateParked = false;
	s.apGateSummoning = false;

	startBurst(s, f, BUS_VEHICLE, 5, 20);
	TEST_ASSERT_EQUAL_UINT8(0, s.burstRemaining);
}

void test_start_burst_allows_when_gate_open()
{
	State s = {};
	Frame f = {};
	f.id = CAN_ID_UI_VEHICLE_CTRL;
	f.dlc = 8;

	s.apInjectionGateEnabled = true;
	s.apGateApActive = false;
	s.apGateParked = true;
	s.apGateSummoning = false;

	startBurst(s, f, BUS_VEHICLE, 5, 20);
	TEST_ASSERT_EQUAL_UINT8(5, s.burstRemaining);
	TEST_ASSERT_EQUAL_UINT8(BUS_VEHICLE, s.burstBus);
	TEST_ASSERT_EQUAL_UINT32(CAN_ID_UI_VEHICLE_CTRL, s.burstFrame.id);
}

void test_apgate_standby_transition_opens_by_park()
{
	State s = {};
	s.apInjectionGateEnabled = true;
	s.apGateApActive = false;
	s.apGateParked = false;
	s.apGateSummoning = true;
	s.summonRemaining = 12;
	s.hasDrive = true;
	s.chassisOnline = true;
	s.standby = false;

	applyStandbyTransitionOnCanTimeout(s);
	TEST_ASSERT_TRUE(s.standby);
	TEST_ASSERT_FALSE(s.apGateSummoning);
	TEST_ASSERT_TRUE(s.apGateParked);
	TEST_ASSERT_TRUE(s.apGateOpen());
	TEST_ASSERT_EQUAL(0, s.summonRemaining);
}

void test_apgate_standby_transition_stays_open_when_gate_disabled()
{
	State s = {};
	s.apInjectionGateEnabled = false;
	s.apGateApActive = false;
	s.apGateParked = false;
	s.apGateSummoning = true;

	applyStandbyTransitionOnCanTimeout(s);
	TEST_ASSERT_TRUE(s.apGateOpen());
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_apgate_open_when_disabled);
	RUN_TEST(test_apgate_closed_waiting_state);
	RUN_TEST(test_apgate_open_ap_only);
	RUN_TEST(test_apgate_open_park_only);
	RUN_TEST(test_apgate_open_summon_only);
	RUN_TEST(test_drive_mode_tick_blocked_when_gate_closed);
	RUN_TEST(test_drive_mode_tick_allows_when_ap_open);
	RUN_TEST(test_seatbelt_tick_blocked_when_gate_closed);
	RUN_TEST(test_seatbelt_tick_allows_when_summoning_open);
	RUN_TEST(test_start_burst_blocked_when_gate_closed);
	RUN_TEST(test_start_burst_allows_when_gate_open);
	RUN_TEST(test_apgate_standby_transition_opens_by_park);
	RUN_TEST(test_apgate_standby_transition_stays_open_when_gate_disabled);
	return UNITY_END();
}
