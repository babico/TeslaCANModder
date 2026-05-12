/**
 * @file firmware/test/test_native_ap_gate/test_ap_gate.cpp
 * @brief Unit tests for autopilot injection gate logic
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <unity.h>
#include <cstring>

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

#include "core/types.h"
#include "vehicle/can/ids.h"

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
#include "vehicle/can/burst.h"

/** @brief Reset fake millis and stub send buffer before each test */
void setUp()
{
	fake_millis_val = 0;
	stub_send_count = 0;
}

/** @brief Cleanup after each test */
void tearDown() {}

/**
 * @brief Simulate the standby transition that occurs on CAN timeout
 *
 * Mirrors the real firmware behavior: clears online flags, sets standby and parked state,
 * and cancels any active summon session.
 */
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

/** @brief Gate is always open when the injection gate feature is disabled */
void test_apgate_open_when_disabled()
{
	State s = {};
	s.apInjectionGateEnabled = false;
	s.apGateApActive = false;
	s.apGateParked = false;
	s.apGateSummoning = false;
	TEST_ASSERT_TRUE(s.apGateOpen());
}

/** @brief Gate is closed when enabled but no qualifying condition is met */
void test_apgate_closed_waiting_state()
{
	State s = {};
	s.apInjectionGateEnabled = true;
	s.apGateApActive = false;
	s.apGateParked = false;
	s.apGateSummoning = false;
	TEST_ASSERT_FALSE(s.apGateOpen());
}

/** @brief Gate opens when autopilot is active */
void test_apgate_open_ap_only()
{
	State s = {};
	s.apInjectionGateEnabled = true;
	s.apGateApActive = true;
	s.apGateParked = false;
	s.apGateSummoning = false;
	TEST_ASSERT_TRUE(s.apGateOpen());
}

/** @brief Gate opens when vehicle is parked */
void test_apgate_open_park_only()
{
	State s = {};
	s.apInjectionGateEnabled = true;
	s.apGateApActive = false;
	s.apGateParked = true;
	s.apGateSummoning = false;
	TEST_ASSERT_TRUE(s.apGateOpen());
}

/** @brief Gate opens when summon is active */
void test_apgate_open_summon_only()
{
	State s = {};
	s.apInjectionGateEnabled = true;
	s.apGateApActive = false;
	s.apGateParked = false;
	s.apGateSummoning = true;
	TEST_ASSERT_TRUE(s.apGateOpen());
}

/** @brief Drive mode tick produces no output when the gate is closed */
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

/** @brief Drive mode tick sends a frame when the gate is open via AP active */
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

/** @brief Seatbelt emulation tick is suppressed when the gate is closed */
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

/** @brief Seatbelt emulation tick sends when the gate is open via summoning */
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

/** @brief startBurst is blocked and burstRemaining stays zero when gate is closed */
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

/** @brief startBurst succeeds and populates burst state when gate is open via park */
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

/** @brief Standby transition sets parked flag, opening the gate for parked injection */
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

/** @brief Standby transition keeps gate open when the feature is disabled entirely */
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
