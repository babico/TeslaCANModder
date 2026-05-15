/**
 * @file firmware/test/test_native_burst/test_burst.cpp
 * @brief Unit tests for non-blocking burst send logic
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
#include "core/can/bus.h"
#include "vehicle/can/burst.h"

void setUp() {}
void tearDown() {}

/* ── startBurst blocks when txPaused ─────────────────────────────────────── */

void test_burst_blocked_when_tx_paused()
{
	State s = {};
	s.txPaused = true;
	s.apInjectionGateEnabled = false;
	Frame f = {};
	f.id = 0x100;
	f.dlc = 8;
	startBurst(s, f, 1, 10, 20);
	TEST_ASSERT_EQUAL_UINT8(0, s.burstRemaining);
}

/* ── startBurst blocks when AP gate closed ───────────────────────────────── */

void test_burst_blocked_when_ap_gate_closed()
{
	State s = {};
	s.txPaused = false;
	s.apInjectionGateEnabled = true;
	s.apGateApActive = false;
	s.apGateParked = false;
	s.apGateSummoning = false;
	Frame f = {};
	f.id = 0x100;
	f.dlc = 8;
	startBurst(s, f, 1, 10, 20);
	TEST_ASSERT_EQUAL_UINT8(0, s.burstRemaining);
}

/* ── startBurst succeeds when gate open ──────────────────────────────────── */

void test_burst_sets_up_frame()
{
	State s = {};
	s.txPaused = false;
	s.apInjectionGateEnabled = false;
	Frame f = {};
	f.id = 0x273;
	f.dlc = 8;
	f.data[0] = 0x42;
	startBurst(s, f, BUS_VEHICLE, 20, 20);
	TEST_ASSERT_EQUAL_HEX32(0x273, s.burstFrame.id);
	TEST_ASSERT_EQUAL_HEX8(0x42, s.burstFrame.data[0]);
	TEST_ASSERT_EQUAL_UINT8(20, s.burstRemaining);
	TEST_ASSERT_EQUAL_UINT8(20, s.burstDelayMs);
}

void test_burst_sets_bus()
{
	State s = {};
	s.txPaused = false;
	s.apInjectionGateEnabled = false;
	Frame f = {};
	f.dlc = 8;
	startBurst(s, f, 2, 5, 10);
	TEST_ASSERT_EQUAL_UINT8(2, s.burstBus);
}

void test_burst_resets_timer()
{
	State s = {};
	s.txPaused = false;
	s.apInjectionGateEnabled = false;
	Frame f = {};
	f.dlc = 8;
	s.burstLastMs = 99999;
	startBurst(s, f, 0, 3, 50);
	TEST_ASSERT_EQUAL(0, s.burstLastMs);
}

/* ── startBurst allows when AP gate open via park ────────────────────────── */

void test_burst_allowed_when_parked()
{
	State s = {};
	s.txPaused = false;
	s.apInjectionGateEnabled = true;
	s.apGateParked = true;
	Frame f = {};
	f.id = 0x100;
	f.dlc = 8;
	startBurst(s, f, 1, 5, 10);
	TEST_ASSERT_EQUAL_UINT8(5, s.burstRemaining);
}

void test_burst_allowed_when_summoning()
{
	State s = {};
	s.txPaused = false;
	s.apInjectionGateEnabled = true;
	s.apGateSummoning = true;
	Frame f = {};
	f.id = 0x100;
	f.dlc = 8;
	startBurst(s, f, 1, 5, 10);
	TEST_ASSERT_EQUAL_UINT8(5, s.burstRemaining);
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_burst_blocked_when_tx_paused);
	RUN_TEST(test_burst_blocked_when_ap_gate_closed);
	RUN_TEST(test_burst_sets_up_frame);
	RUN_TEST(test_burst_sets_bus);
	RUN_TEST(test_burst_resets_timer);
	RUN_TEST(test_burst_allowed_when_parked);
	RUN_TEST(test_burst_allowed_when_summoning);

	return UNITY_END();
}
