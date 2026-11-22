/**
 * @file firmware/test/test_native_cmd_freshness/test_cmd_freshness.cpp
 * @brief C1-07a/C1-19: fresh 0x273 gate for one-shot cmds; fresh-boot diagnostics bypass it
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <unity.h>
#include <cstdio>
#include <cstring>
#include <string>

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0
#define BOARD_HW_NAME "ESP32S_DevKit"
#define BOARD_CAN_NAME "MCP2515_3x"
#define BOARD_DRIVER_NAME "arduino-mcp2515"

#include "../support/fake_preferences.h"
#define Preferences_h

#include "core/types.h"
#include "vehicle/can/ids.h"

 static std::string capturedOutput;
 class __FlashStringHelper;
 #define F(s) (reinterpret_cast<const __FlashStringHelper *>(s))
 #define HEX 16

 class FakeSerial
 {
   public:
 	void print(const char *s) { capturedOutput += s; }
 	void print(const __FlashStringHelper *s) { capturedOutput += reinterpret_cast<const char *>(s); }
 	void print(long n)
 	{
 		char buf[32];
 		snprintf(buf, sizeof(buf), "%ld", n);
 		capturedOutput += buf;
 	}
 	void print(uint8_t b, int base)
 	{
 		char buf[8];
 		snprintf(buf, sizeof(buf), base == HEX ? "%X" : "%u", b);
 		capturedOutput += buf;
 	}
 	void println() { capturedOutput += "\n"; }
 };
 static FakeSerial Serial;

 static unsigned long fake_millis = 0;
 unsigned long millis() { return fake_millis; }

 struct SendCall { Frame f; uint8_t bus; };
 static SendCall stub_sends[16];
 static uint8_t stub_send_count = 0;
 void driverSend(const Frame &f, uint8_t bus)
 {
 	if (stub_send_count < 16)
 		stub_sends[stub_send_count++] = {f, bus};
 }
 void driverSetBusFilters(uint8_t, const uint32_t *, uint8_t) {}
 void driverSetClockMHz(uint8_t) {}
 bool driverReinit() { return true; }
 uint8_t driverGetClockReqMHz() { return BOARD_CAN_CLOCK_MHZ; }
 uint8_t driverGetClockMHz() { return BOARD_CAN_CLOCK_MHZ; }
 void driverSetSingleShot(bool) {}
 uint32_t driverGetAndResetTxFails() { return 0; }
 uint32_t driverGetAndResetBusOffEvents() { return 0; }
 void driverPollBusErrors() {}
 bool mcpAvailable[BUS_MAX] = {true, true, true};

 void saveSettings(const State &) {}
 void resetHandlerLogFlags() {}
 void applyFilters(State &) {}

#include "core/util/parse.h"
#include "core/log/ring.h"
#include "vehicle/can/feature/misc/stream.h"
#include "vehicle/can/feature/misc/can_raw.h"
#include "vehicle/can/feature/misc/can_clock.h"
#include "vehicle/can/feature/das/das_drive.h"
#include "vehicle/can/feature/fsd/fsd.h"
#include "vehicle/can/feature/fsd/nag.h"
#include "vehicle/can/feature/fsd/auto_lane_change.h"
#include "vehicle/can/feature/safety/ban_shield.h"
#include "vehicle/can/feature/das/tlssc.h"
#include "vehicle/can/feature/fsd/profile.h"
#include "vehicle/can/feature/fsd/offsets.h"
#include "vehicle/can/feature/fsd/isa_chime.h"
#include "vehicle/can/feature/body/summon.h"
#include "vehicle/can/feature/misc/variant.h"
#include "vehicle/can/feature/telemetry/bms.h"
#include "vehicle/can/feature/telemetry/tpms.h"
#include "vehicle/can/feature/fsd/region.h"
#include "vehicle/can/feature/drive/drive_mode.h"
#include "vehicle/can/feature/body/mirror.h"
#include "vehicle/can/feature/body/lock.h"
#include "vehicle/can/feature/comfort/light.h"
#include "vehicle/can/feature/comfort/wiper.h"
#include "vehicle/can/feature/comfort/seat.h"
#include "vehicle/can/feature/comfort/display.h"
#include "vehicle/can/feature/body/power.h"
#include "vehicle/can/feature/comfort/climate.h"
#include "vehicle/can/feature/body/charge.h"
#include "vehicle/can/feature/drive/pedal.h"
#include "vehicle/can/feature/drive/regen.h"
#include "vehicle/can/feature/drive/stop.h"
#include "vehicle/can/feature/comfort/precondition.h"
#include "vehicle/can/feature/body/track_mode.h"
#include "vehicle/can/feature/body/turn_signal.h"
#include "vehicle/can/feature/comfort/seatbelt.h"
#include "vehicle/can/feature/comfort/air_recirc.h"
#include "vehicle/can/feature/telemetry/powertrain.h"
#include "vehicle/can/feature/misc/can_sim.h"
#include "vehicle/can/feature/safety/single_shot.h"
#include "vehicle/can/feature/misc/mqtt_bridge.h"
#include "vehicle/can/feature/telemetry/fw_compat.h"
#include "vehicle/can/feature/telemetry/vehicle_config.h"
#include "vehicle/can/feature/body/window.h"
#include "vehicle/can/feature/body/sentry.h"
#include "vehicle/can/feature/body/trunk.h"
#include "vehicle/can/feature/safety/ban_detect.h"
#include "vehicle/ble/feature/distance.h"

#include "client/command/messages.h"
#include "client/command/dispatch.h"

void setUp()
{
	capturedOutput.clear();
	stub_send_count = 0;
	fake_millis = 0;
}

void tearDown() {}

/** @brief Stale cached 0x273 must reject a one-shot command with no injection */
void test_one_shot_cmd_rejected_when_ctrl_stale()
{
	State s = {};
	s.hasCtrl = true;
	s.lastCtrlMs = 1; // captured long ago
	fake_millis = AP_GATE_FRESHNESS_MS + 2;

	executeCommand("lock", s, fake_millis);

	TEST_ASSERT_EQUAL(0, stub_send_count);
	TEST_ASSERT_EQUAL_UINT8(0, s.burstRemaining);
	TEST_ASSERT_NOT_NULL(strstr(capturedOutput.c_str(), "Waiting for 0x273 frame"));
}

/** @brief Fresh cached 0x273 must still accept the same one-shot command */
void test_one_shot_cmd_allowed_when_ctrl_fresh()
{
	State s = {};
	s.hasCtrl = true;
	s.lastCtrlMs = AP_GATE_FRESHNESS_MS - 50;
	fake_millis = AP_GATE_FRESHNESS_MS - 10;

	executeCommand("lock", s, fake_millis);

	TEST_ASSERT_EQUAL_UINT8(30, s.burstRemaining);
	TEST_ASSERT_NOT_NULL(strstr(capturedOutput.c_str(), "\"t\":\"ack\""));
}

// C1-19: fresh-boot diagnostics must dispatch through the real executeCommand
// path with zero CAN state (hasCtrl=false). They sit above the single
// hasFreshCtrl(now) gate because they need no cached 0x273 frame. These tests
// assert emitted JSON, not source order.
/** @brief log dumps its ring as JSON with no cached 0x273 frame */
void test_fresh_boot_log_emits_json_without_ctrl()
{
	State s = {};
	TEST_ASSERT_FALSE(s.hasCtrl);

	executeCommand("log", s, fake_millis);

	TEST_ASSERT_NOT_NULL(strstr(capturedOutput.c_str(), "\"t\":\"log\""));
	TEST_ASSERT_NULL(strstr(capturedOutput.c_str(), "Waiting for 0x273"));
}

/** @brief platform identity query emits JSON with no cached 0x273 frame */
void test_fresh_boot_platform_emits_json_without_ctrl()
{
	State s = {};
	TEST_ASSERT_FALSE(s.hasCtrl);

	executeCommand("platform", s, fake_millis);

	TEST_ASSERT_NOT_NULL(strstr(capturedOutput.c_str(), "\"t\":\"platform\""));
	TEST_ASSERT_NULL(strstr(capturedOutput.c_str(), "Waiting for 0x273"));
}

/** @brief recorder:status emits JSON with no cached 0x273 frame */
void test_fresh_boot_recorder_status_emits_json_without_ctrl()
{
	State s = {};
	TEST_ASSERT_FALSE(s.hasCtrl);

	executeCommand("recorder:status", s, fake_millis);

	TEST_ASSERT_NOT_NULL(strstr(capturedOutput.c_str(), "\"t\":\"recorder\""));
	TEST_ASSERT_NULL(strstr(capturedOutput.c_str(), "Waiting for 0x273"));
}

/** @brief Contrast: a gated vehicle command still errors with no CAN state */
void test_gated_cmd_errors_without_ctrl()
{
	State s = {};
	TEST_ASSERT_FALSE(s.hasCtrl);

	executeCommand("lock", s, fake_millis);

	TEST_ASSERT_EQUAL(0, stub_send_count);
	TEST_ASSERT_NOT_NULL(strstr(capturedOutput.c_str(), "Waiting for 0x273"));
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_one_shot_cmd_rejected_when_ctrl_stale);
	RUN_TEST(test_one_shot_cmd_allowed_when_ctrl_fresh);
	RUN_TEST(test_fresh_boot_log_emits_json_without_ctrl);
	RUN_TEST(test_fresh_boot_platform_emits_json_without_ctrl);
	RUN_TEST(test_fresh_boot_recorder_status_emits_json_without_ctrl);
	RUN_TEST(test_gated_cmd_errors_without_ctrl);
	return UNITY_END();
}
