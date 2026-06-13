/**
 * @file firmware/test/test_native_dispatch/test_dispatch.cpp
 * @brief Unit tests for message dispatch, filter application, summon tick, and frame caching
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <unity.h>
#include <cstring>

class __FlashStringHelper;

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

unsigned long millis();

#include "core/types.h"
#include "vehicle/can/ids.h"
#include "feature/body/summon.h"

struct McpFilterCall
{
	uint8_t idx;
	uint8_t count;
	bool cleared;
};
static McpFilterCall stub_mcp_calls[8];
static uint8_t stub_mcp_call_count = 0;

bool mcpAvailable[BUS_MAX] = {true, true, true};

void driverSetBusFilters(uint8_t bus, const uint32_t *ids, uint8_t count)
{
	if (stub_mcp_call_count < 8)
	{
		stub_mcp_calls[stub_mcp_call_count++] = {bus, count, ids == nullptr};
	}
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

static int log_call_count = 0;
void sendLog(const char *)
{
	log_call_count++;
}
void sendLog(const __FlashStringHelper *)
{
	log_call_count++;
}

static int hw4_call_count = 0;
static int hw3_call_count = 0;
static int legacy_call_count = 0;

void handleHW4(Frame &, State &)
{
	hw4_call_count++;
}
void handleHW3(Frame &, State &)
{
	hw3_call_count++;
}
void handleLegacy(Frame &, State &)
{
	legacy_call_count++;
}
void resetHW4LogFlags() {}
void resetHW3LogFlags() {}
void resetLegacyLogFlags() {}
void resetHandlerLogFlags()
{
	resetHW4LogFlags();
	resetHW3LogFlags();
	resetLegacyLogFlags();
}
void saveSettings(const State &) {}

#ifndef F
#define F(x) (reinterpret_cast<const __FlashStringHelper *>(x))
#endif

static unsigned long fake_millis = 0;
unsigned long millis()
{
	return fake_millis;
}

void applyFilters(State &s)
{
	if (s.rawCanListen)
	{
		driverSetBusFilters(0, nullptr, 0);
	}
	else
	{
		uint32_t ids[10];
		uint8_t count = 0;
		bool isaAdded = false;
		bool legacyMuxAdded = false;
		switch (s.variant)
		{
		case HW4:
			if (s.isaChimeSuppress)
			{
				ids[count++] = CAN_ID_ISA_SPEED;
				isaAdded = true;
			}
			if (s.fsdEnabled)
				ids[count++] = CAN_ID_FOLLOW_DIST;
			if (s.fsdEnabled || nagModeUsesBit19(s.nagMode))
				ids[count++] = CAN_ID_FSD_MUX;
			break;
		case HW3:
			if (s.fsdEnabled)
				ids[count++] = CAN_ID_FOLLOW_DIST;
			if (s.fsdEnabled || nagModeUsesBit19(s.nagMode))
				ids[count++] = CAN_ID_FSD_MUX;
			break;
		case LEGACY:
			if (s.fsdEnabled)
				ids[count++] = CAN_ID_LEGACY_STALK;
			if (s.fsdEnabled || nagModeUsesBit19(s.nagMode))
			{
				ids[count++] = CAN_ID_LEGACY_FSD_MUX;
				legacyMuxAdded = true;
			}
			break;
		}
		if (s.variantAutoDetect && !s.hwAutoDetected)
		{
			if (!isaAdded)
				ids[count++] = CAN_ID_ISA_SPEED;
			if (!legacyMuxAdded)
				ids[count++] = CAN_ID_LEGACY_FSD_MUX;
		}
		if (count > 0)
		{
			driverSetBusFilters(BUS_CHASSIS, ids, count);
		}
		else
		{
			static const uint32_t none[] = {0x000};
			driverSetBusFilters(BUS_CHASSIS, none, 1);
		}
	}

#if BUS_VEHICLE_ACTIVE
	if (s.rawCanListen)
	{
		driverSetBusFilters(BUS_VEHICLE, nullptr, 0);
	}
	else
	{
		static const uint32_t vehIds[] = {CAN_ID_UI_VEHICLE_CTRL, CAN_ID_CLIMATE, CAN_ID_CHARGE, CAN_ID_DRIVE_CONFIG};
		driverSetBusFilters(BUS_VEHICLE, vehIds, 4);
	}

	if (s.rawCanListen)
	{
		driverSetBusFilters(BUS_BODY, nullptr, 0);
	}
	else
	{
		static const uint32_t bodyIds[] = {CAN_ID_WINDOW_VENT, CAN_ID_SENTRY, CAN_ID_TRUNK_CTRL};
		driverSetBusFilters(BUS_BODY, bodyIds, 3);
	}
#endif
}

void handleMessage(Frame &f, uint8_t bus, State &s)
{
	if (bus == BUS_CHASSIS)
	{
		if (s.variantAutoDetect && !s.hwAutoDetected)
		{
			if (f.id == CAN_ID_ISA_SPEED && s.variant != HW4)
			{
				bool fromLegacy = (s.variant == LEGACY);
				s.variant = HW4;
				if (fromLegacy)
					s.speedProfile = 1;
				applyFilters(s);
				resetHandlerLogFlags();
				sendLog(F("Fallback: HW4 inferred from ISA_SPEED"));
			}
			else if (f.id == CAN_ID_LEGACY_FSD_MUX && s.variant != LEGACY)
			{
				s.variant = LEGACY;
				applyFilters(s);
				resetHandlerLogFlags();
				sendLog(F("Fallback: LEGACY inferred from legacy mux frame"));
			}
		}
		switch (s.variant)
		{
		case HW4:
			handleHW4(f, s);
			break;
		case HW3:
			handleHW3(f, s);
			break;
		case LEGACY:
			handleLegacy(f, s);
			break;
		}
		return;
	}

#if BUS_VEHICLE_ACTIVE
	if (bus == BUS_VEHICLE)
	{
		if (f.id == CAN_ID_UI_VEHICLE_CTRL && f.dlc >= 8)
		{
			memcpy(s.lastCtrl, f.data, 8);
			s.hasCtrl = true;
			return;
		}
		if (f.id == CAN_ID_CLIMATE && f.dlc >= 5)
		{
			memcpy(s.lastClimate, f.data, 5);
			s.hasClimate = true;
			return;
		}
		if (f.id == CAN_ID_CHARGE && f.dlc >= 5)
		{
			memcpy(s.lastCharge, f.data, 5);
			s.hasCharge = true;
			return;
		}
		if (f.id == CAN_ID_DRIVE_CONFIG && f.dlc >= 8)
		{
			memcpy(s.lastDrive, f.data, 8);
			s.hasDrive = true;
			return;
		}

		if (f.id == CAN_ID_GTW_CAR_CFG && f.dlc >= 1)
		{
			uint8_t hw = (f.data[0] >> 6) & 0x03; // HW version in bits [7:6]
			s.detectedHW = hw;
			s.hwAutoDetected = true;
			if (s.variantAutoDetect)
			{
				Variant detected;
				if (hw == 3)
					detected = HW4;
				else if (hw == 2)
					detected = HW3;
				else
					detected = LEGACY;
				if (s.variant != detected)
				{
					bool fromLegacy = (s.variant == LEGACY);
					s.variant = detected;
					if (fromLegacy && detected != LEGACY)
						s.speedProfile = 1;
					applyFilters(s);
					resetHandlerLogFlags();
					if (detected == HW4)
						sendLog(F("Auto-detected HW4"));
					else if (detected == HW3)
						sendLog(F("Auto-detected HW3"));
					else
						sendLog(F("Auto-detected Legacy"));
				}
			}
			return;
		}
	}
#endif
}

void summonTick(State &s)
{
#if !BUS_VEHICLE_ACTIVE
	(void)s;
	return;
#else
	if (s.summonRemaining == 0 || !s.hasCtrl || !s.summonInject)
		return;
	unsigned long now = millis();
	if (now - s.summonLastMs < 20)
		return;
	s.summonLastMs = now;

	Frame f;
	f.id = CAN_ID_UI_VEHICLE_CTRL;
	f.dlc = 8;
	memcpy(f.data, s.lastCtrl, 8);
	setSummonActive(f, true);
	setSummonDirection(f, s.summonDirection);
	setSummonMode(f, s.summonMode);
	driverSend(f, BUS_VEHICLE);
	s.summonRemaining--;
	if (s.summonRemaining == 0)
		sendLog("Summon burst complete");
#endif
}

void burstTick(State &s)
{
	if (s.burstRemaining == 0)
		return;
	unsigned long now = millis();
	if (now - s.burstLastMs < s.burstDelayMs)
		return;
	s.burstLastMs = now;
	driverSend(s.burstFrame, s.burstBus);
	s.burstRemaining--;
}

/** @brief Creates a State with the given variant and autodetect disabled */
static State makeState(Variant v = HW4)
{
	State s = {};
	s.variant = v;
	s.speedProfile = 1;
	s.variantAutoDetect = false;
	return s;
}

/** @brief Creates a Frame with the given CAN ID and DLC */
static Frame makeFrame(uint32_t id, uint8_t dlc = 8)
{
	Frame f = {};
	f.id = id;
	f.dlc = dlc;
	return f;
}

/** @brief Resets all stub counters and fake clock before each test */
void setUp()
{
	stub_mcp_call_count = 0;
	stub_send_count = 0;
	hw4_call_count = 0;
	hw3_call_count = 0;
	legacy_call_count = 0;
	log_call_count = 0;
	fake_millis = 0;
}

/** @brief Test fixture teardown — no cleanup required */
void tearDown() {}

/** @brief Verifies UI_VEHICLE_CTRL frame is cached in state.lastCtrl */
void test_dispatch_caches_ctrl_frame()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_UI_VEHICLE_CTRL);
	f.data[0] = 0xAA;
	handleMessage(f, BUS_VEHICLE, s);
	TEST_ASSERT_TRUE(s.hasCtrl);
	TEST_ASSERT_EQUAL(0xAA, s.lastCtrl[0]);
}

/** @brief Verifies CLIMATE frame is cached in state.lastClimate */
void test_dispatch_caches_climate_frame()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_CLIMATE, 5);
	f.data[0] = 0xBB;
	handleMessage(f, BUS_VEHICLE, s);
	TEST_ASSERT_TRUE(s.hasClimate);
	TEST_ASSERT_EQUAL(0xBB, s.lastClimate[0]);
}

/** @brief Verifies CHARGE frame is cached in state.lastCharge */
void test_dispatch_caches_charge_frame()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_CHARGE, 5);
	f.data[2] = 0xCC;
	handleMessage(f, BUS_VEHICLE, s);
	TEST_ASSERT_TRUE(s.hasCharge);
	TEST_ASSERT_EQUAL(0xCC, s.lastCharge[2]);
}

/** @brief Verifies DRIVE_CONFIG frame is cached in state.lastDrive */
void test_dispatch_caches_drive_frame()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_DRIVE_CONFIG);
	f.data[7] = 0xDD;
	handleMessage(f, BUS_VEHICLE, s);
	TEST_ASSERT_TRUE(s.hasDrive);
	TEST_ASSERT_EQUAL(0xDD, s.lastDrive[7]);
}

/** @brief Verifies HW4 variant routes chassis frames to the HW4 handler */
void test_dispatch_hw4_routes_to_hw4_handler()
{
	State s = makeState(HW4);
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	handleMessage(f, 0, s);
	TEST_ASSERT_EQUAL(1, hw4_call_count);
	TEST_ASSERT_EQUAL(0, hw3_call_count);
	TEST_ASSERT_EQUAL(0, legacy_call_count);
}

/** @brief Verifies HW3 variant routes chassis frames to the HW3 handler */
void test_dispatch_hw3_routes_to_hw3_handler()
{
	State s = makeState(HW3);
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	handleMessage(f, 0, s);
	TEST_ASSERT_EQUAL(0, hw4_call_count);
	TEST_ASSERT_EQUAL(1, hw3_call_count);
}

/** @brief Verifies LEGACY variant routes chassis frames to the legacy handler */
void test_dispatch_legacy_routes_to_legacy_handler()
{
	State s = makeState(LEGACY);
	Frame f = makeFrame(CAN_ID_LEGACY_FSD_MUX);
	handleMessage(f, 0, s);
	TEST_ASSERT_EQUAL(1, legacy_call_count);
}

/** @brief Verifies handler frames from bus 1 (vehicle) are not routed to chassis handlers */
void test_dispatch_ignores_handler_frames_from_bus1()
{
	State s = makeState(HW4);
	Frame f = makeFrame(CAN_ID_FSD_MUX);
	handleMessage(f, 1, s);
	TEST_ASSERT_EQUAL(0, hw4_call_count);
}

/** @brief Verifies ctrl frame with DLC < 8 is not cached */
void test_dispatch_ctrl_short_frame_ignored()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_UI_VEHICLE_CTRL, 4);
	handleMessage(f, BUS_VEHICLE, s);
	TEST_ASSERT_FALSE(s.hasCtrl);
}

/** @brief Verifies HW4 with ISA + FSD enabled sets 3 chassis filter IDs */
void test_apply_filters_hw4_sets_fsd_3_ids()
{
	State s = makeState(HW4);
	s.isaChimeSuppress = true;
	s.fsdEnabled = true;
	applyFilters(s);
	TEST_ASSERT_TRUE(stub_mcp_call_count >= 1);
	TEST_ASSERT_EQUAL(BUS_CHASSIS, stub_mcp_calls[0].idx);
	TEST_ASSERT_EQUAL(3, stub_mcp_calls[0].count);
}

/** @brief Verifies HW3 with FSD enabled sets 2 chassis filter IDs */
void test_apply_filters_hw3_sets_fsd_2_ids()
{
	State s = makeState(HW3);
	s.fsdEnabled = true;
	applyFilters(s);
	TEST_ASSERT_TRUE(stub_mcp_call_count >= 1);
	TEST_ASSERT_EQUAL(BUS_CHASSIS, stub_mcp_calls[0].idx);
	TEST_ASSERT_EQUAL(2, stub_mcp_calls[0].count);
}

/** @brief Verifies LEGACY with FSD + NAG sets 2 chassis filter IDs */
void test_apply_filters_legacy_sets_fsd_2_ids()
{
	State s = makeState(LEGACY);
	s.fsdEnabled = true;
	s.nagMode = NAG_MODE_BIT19;
	applyFilters(s);
	TEST_ASSERT_TRUE(stub_mcp_call_count >= 1);
	TEST_ASSERT_EQUAL(BUS_CHASSIS, stub_mcp_calls[0].idx);
	TEST_ASSERT_EQUAL(2, stub_mcp_calls[0].count);
}

/** @brief Verifies raw CAN listen mode clears all bus filters */
void test_apply_filters_raw_can_clears_all()
{
	State s = makeState(HW4);
	s.rawCanListen = true;
	applyFilters(s);
	for (uint8_t i = 0; i < stub_mcp_call_count; i++)
	{
		TEST_ASSERT_TRUE(stub_mcp_calls[i].cleared);
	}
}

/** @brief Verifies vehicle bus gets 4 IDs and body bus gets 3 IDs */
void test_apply_filters_sets_vehicle_and_body_buses()
{
	State s = makeState(HW4);
	s.fsdEnabled = true;
	applyFilters(s);
	TEST_ASSERT_EQUAL(3, stub_mcp_call_count);
	TEST_ASSERT_EQUAL(BUS_VEHICLE, stub_mcp_calls[1].idx);
	TEST_ASSERT_EQUAL(4, stub_mcp_calls[1].count);
	TEST_ASSERT_EQUAL(BUS_BODY, stub_mcp_calls[2].idx);
	TEST_ASSERT_EQUAL(3, stub_mcp_calls[2].count);
}

/** @brief Verifies no features enabled sets a single blocking filter on chassis */
void test_apply_filters_no_features_blocks_fsd_bus()
{
	State s = makeState(HW4);
	applyFilters(s);
	TEST_ASSERT_TRUE(stub_mcp_call_count >= 1);
	TEST_ASSERT_EQUAL(BUS_CHASSIS, stub_mcp_calls[0].idx);
	TEST_ASSERT_EQUAL(1, stub_mcp_calls[0].count);
	TEST_ASSERT_FALSE(stub_mcp_calls[0].cleared);
}

/** @brief Verifies NAG mode alone adds the FSD mux filter */
void test_apply_filters_nag_only_sets_mux_filter()
{
	State s = makeState(HW4);
	s.nagMode = NAG_MODE_BIT19;
	applyFilters(s);
	TEST_ASSERT_EQUAL(BUS_CHASSIS, stub_mcp_calls[0].idx);
	TEST_ASSERT_EQUAL(1, stub_mcp_calls[0].count);
}

/** @brief Verifies ISA chime suppress alone adds the ISA speed filter */
void test_apply_filters_isa_only_sets_isa_filter()
{
	State s = makeState(HW4);
	s.isaChimeSuppress = true;
	applyFilters(s);
	TEST_ASSERT_EQUAL(BUS_CHASSIS, stub_mcp_calls[0].idx);
	TEST_ASSERT_EQUAL(1, stub_mcp_calls[0].count);
}

/** @brief Verifies fallback discriminator IDs are added when HW is unknown */
void test_apply_filters_fallback_adds_discriminator_ids_when_hw_unknown()
{
	State s = makeState(HW3);
	s.variantAutoDetect = true;
	s.hwAutoDetected = false;
	applyFilters(s);
	TEST_ASSERT_TRUE(stub_mcp_call_count >= 1);
	TEST_ASSERT_EQUAL(BUS_CHASSIS, stub_mcp_calls[0].idx);
	TEST_ASSERT_EQUAL(2, stub_mcp_calls[0].count);
}

/** @brief Verifies no fallback IDs are added when HW is already detected */
void test_apply_filters_no_fallback_ids_when_hw_already_detected()
{
	State s = makeState(HW3);
	s.variantAutoDetect = true;
	s.hwAutoDetected = true;
	applyFilters(s);
	TEST_ASSERT_TRUE(stub_mcp_call_count >= 1);
	TEST_ASSERT_EQUAL(BUS_CHASSIS, stub_mcp_calls[0].idx);
	TEST_ASSERT_EQUAL(1, stub_mcp_calls[0].count);
}

/** @brief Verifies summon tick does nothing when remaining count is zero */
void test_summon_tick_does_nothing_when_remaining_zero()
{
	State s = makeState();
	s.summonRemaining = 0;
	summonTick(s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

/** @brief Verifies summon tick does nothing without a cached ctrl frame */
void test_summon_tick_does_nothing_without_ctrl()
{
	State s = makeState();
	s.summonRemaining = 5;
	s.hasCtrl = false;
	summonTick(s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

/** @brief Verifies summon tick sends a frame and decrements remaining */
void test_summon_tick_sends_burst_frame()
{
	State s = makeState();
	s.summonRemaining = 3;
	s.hasCtrl = true;
	s.summonInject = true;
	s.summonDirection = SUMMON_FORWARD;
	s.summonMode = SUMMON_START;
	memset(s.lastCtrl, 0, 8);
	fake_millis = 100;
	s.summonLastMs = 0;

	summonTick(s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL(2, s.summonRemaining);
	TEST_ASSERT_EQUAL(CAN_ID_UI_VEHICLE_CTRL, stub_sends[0].f.id);
	TEST_ASSERT_EQUAL(BUS_VEHICLE, stub_sends[0].bus);
	TEST_ASSERT_BITS(0x11, 0x11, stub_sends[0].f.data[0]);
}

/** @brief Verifies summon tick respects the 20ms minimum interval */
void test_summon_tick_respects_20ms_interval()
{
	State s = makeState();
	s.summonRemaining = 5;
	s.hasCtrl = true;
	s.summonInject = true;
	memset(s.lastCtrl, 0, 8);
	s.summonLastMs = 90;
	fake_millis = 100;

	summonTick(s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
	TEST_ASSERT_EQUAL(5, s.summonRemaining);
}

/** @brief Verifies summon tick decrements remaining to zero on last frame */
void test_summon_tick_decrements_to_zero()
{
	State s = makeState();
	s.summonRemaining = 1;
	s.hasCtrl = true;
	s.summonInject = true;
	memset(s.lastCtrl, 0, 8);
	s.summonLastMs = 0;
	fake_millis = 100;

	summonTick(s);
	TEST_ASSERT_EQUAL(0, s.summonRemaining);
}

/** @brief Verifies summon tick sets the reverse direction bit */
void test_summon_tick_reverse_direction()
{
	State s = makeState();
	s.summonRemaining = 1;
	s.hasCtrl = true;
	s.summonInject = true;
	s.summonDirection = SUMMON_REVERSE;
	s.summonMode = SUMMON_START;
	memset(s.lastCtrl, 0, 8);
	s.summonLastMs = 0;
	fake_millis = 100;

	summonTick(s);
	TEST_ASSERT_BITS(0x20, 0x20, stub_sends[0].f.data[0]);
}

/** @brief Verifies GTW_CAR_CFG with hw==3 switches variant to HW4 */
void test_autodetect_hw4_switches_variant()
{
	State s = makeState(LEGACY);
	s.variantAutoDetect = true;
	Frame f = makeFrame(CAN_ID_GTW_CAR_CFG);
	f.data[0] = 3 << 6;
	handleMessage(f, BUS_VEHICLE, s);
	TEST_ASSERT_EQUAL(3, s.detectedHW);
	TEST_ASSERT_TRUE(s.hwAutoDetected);
	TEST_ASSERT_EQUAL(HW4, s.variant);
	TEST_ASSERT_TRUE(log_call_count > 0);
}

/** @brief Verifies GTW_CAR_CFG with hw==2 switches variant to HW3 */
void test_autodetect_hw3_switches_variant()
{
	State s = makeState(HW4);
	s.variantAutoDetect = true;
	Frame f = makeFrame(CAN_ID_GTW_CAR_CFG);
	f.data[0] = 2 << 6;
	handleMessage(f, BUS_VEHICLE, s);
	TEST_ASSERT_EQUAL(2, s.detectedHW);
	TEST_ASSERT_EQUAL(HW3, s.variant);
}

/** @brief Verifies autodetect disabled does not switch variant even with valid HW byte */
void test_autodetect_disabled_no_variant_switch()
{
	State s = makeState(LEGACY);
	s.variantAutoDetect = false;
	Frame f = makeFrame(CAN_ID_GTW_CAR_CFG);
	f.data[0] = 3 << 6;
	handleMessage(f, BUS_VEHICLE, s);
	TEST_ASSERT_EQUAL(3, s.detectedHW);
	TEST_ASSERT_TRUE(s.hwAutoDetected);
	TEST_ASSERT_EQUAL(LEGACY, s.variant);
}

/** @brief Verifies no log is emitted when detected variant matches current */
void test_autodetect_same_variant_no_log()
{
	State s = makeState(HW4);
	s.variantAutoDetect = true;
	Frame f = makeFrame(CAN_ID_GTW_CAR_CFG);
	f.data[0] = 3 << 6;
	handleMessage(f, BUS_VEHICLE, s);
	TEST_ASSERT_EQUAL(3, s.detectedHW);
	TEST_ASSERT_EQUAL(HW4, s.variant);
	TEST_ASSERT_EQUAL(0, log_call_count);
}

/** @brief Verifies hw==1 maps to LEGACY variant */
void test_autodetect_invalid_hw_ignored()
{
	State s = makeState(HW4);
	s.variantAutoDetect = true;
	Frame f = makeFrame(CAN_ID_GTW_CAR_CFG);
	f.data[0] = 1 << 6;
	handleMessage(f, BUS_VEHICLE, s);
	TEST_ASSERT_EQUAL(1, s.detectedHW);
	TEST_ASSERT_TRUE(s.hwAutoDetected);
	TEST_ASSERT_EQUAL(LEGACY, s.variant);
}

/** @brief Verifies hw==0 maps to LEGACY variant */
void test_autodetect_legacy_hw0_switches_variant()
{
	State s = makeState(HW3);
	s.variantAutoDetect = true;
	Frame f = makeFrame(CAN_ID_GTW_CAR_CFG);
	f.data[0] = 0 << 6;
	handleMessage(f, BUS_VEHICLE, s);
	TEST_ASSERT_EQUAL(0, s.detectedHW);
	TEST_ASSERT_TRUE(s.hwAutoDetected);
	TEST_ASSERT_EQUAL(LEGACY, s.variant);
	TEST_ASSERT_TRUE(log_call_count > 0);
}

/** @brief Verifies ISA_SPEED on chassis infers HW4 from LEGACY and resets speed profile */
void test_fallback_isa_speed_infers_hw4_from_legacy()
{
	State s = makeState(LEGACY);
	s.variantAutoDetect = true;
	s.hwAutoDetected = false;
	s.speedProfile = 2;
	Frame f = makeFrame(CAN_ID_ISA_SPEED);
	handleMessage(f, BUS_CHASSIS, s);
	TEST_ASSERT_EQUAL(HW4, s.variant);
	TEST_ASSERT_EQUAL(1, s.speedProfile);
	TEST_ASSERT_TRUE(log_call_count > 0);
}

/** @brief Verifies ISA_SPEED does not switch variant if already HW4 */
void test_fallback_isa_speed_no_switch_if_already_hw4()
{
	State s = makeState(HW4);
	s.variantAutoDetect = true;
	s.hwAutoDetected = false;
	Frame f = makeFrame(CAN_ID_ISA_SPEED);
	handleMessage(f, BUS_CHASSIS, s);
	TEST_ASSERT_EQUAL(HW4, s.variant);
	TEST_ASSERT_EQUAL(0, log_call_count);
}

/** @brief Verifies legacy mux frame on chassis infers LEGACY from HW4 */
void test_fallback_legacy_mux_infers_legacy_from_hw4()
{
	State s = makeState(HW4);
	s.variantAutoDetect = true;
	s.hwAutoDetected = false;
	Frame f = makeFrame(CAN_ID_LEGACY_FSD_MUX);
	handleMessage(f, BUS_CHASSIS, s);
	TEST_ASSERT_EQUAL(LEGACY, s.variant);
	TEST_ASSERT_TRUE(log_call_count > 0);
}

/** @brief Verifies fallback does not switch when HW is already detected */
void test_fallback_no_switch_when_hw_already_detected()
{
	State s = makeState(LEGACY);
	s.variantAutoDetect = true;
	s.hwAutoDetected = true;
	Frame f = makeFrame(CAN_ID_ISA_SPEED);
	handleMessage(f, BUS_CHASSIS, s);
	TEST_ASSERT_EQUAL(LEGACY, s.variant);
}

/** @brief Verifies fallback does not switch when autodetect is off */
void test_fallback_no_switch_when_autodetect_off()
{
	State s = makeState(LEGACY);
	s.variantAutoDetect = false;
	s.hwAutoDetected = false;
	Frame f = makeFrame(CAN_ID_ISA_SPEED);
	handleMessage(f, BUS_CHASSIS, s);
	TEST_ASSERT_EQUAL(LEGACY, s.variant);
}

/** @brief Verifies LEGACY to HW3 via 0x398 resets speed profile to 1 */
void test_legacy_to_hw3_via_0x398_resets_speed_profile()
{
	State s = makeState(LEGACY);
	s.variantAutoDetect = true;
	s.speedProfile = 2;
	Frame f = makeFrame(CAN_ID_GTW_CAR_CFG);
	f.data[0] = 2 << 6;
	handleMessage(f, BUS_VEHICLE, s);
	TEST_ASSERT_EQUAL(HW3, s.variant);
	TEST_ASSERT_EQUAL(1, s.speedProfile);
}

/** @brief Verifies LEGACY to HW4 via 0x398 resets speed profile to 1 */
void test_legacy_to_hw4_via_0x398_resets_speed_profile()
{
	State s = makeState(LEGACY);
	s.variantAutoDetect = true;
	s.speedProfile = 0;
	Frame f = makeFrame(CAN_ID_GTW_CAR_CFG);
	f.data[0] = 3 << 6;
	handleMessage(f, BUS_VEHICLE, s);
	TEST_ASSERT_EQUAL(HW4, s.variant);
	TEST_ASSERT_EQUAL(1, s.speedProfile);
}

/** @brief Verifies HW4 to HW3 via 0x398 does not reset speed profile */
void test_hw4_to_hw3_via_0x398_does_not_reset_speed_profile()
{
	State s = makeState(HW4);
	s.variantAutoDetect = true;
	s.speedProfile = 3;
	Frame f = makeFrame(CAN_ID_GTW_CAR_CFG);
	f.data[0] = 2 << 6;
	handleMessage(f, BUS_VEHICLE, s);
	TEST_ASSERT_EQUAL(HW3, s.variant);
	TEST_ASSERT_EQUAL(3, s.speedProfile);
}

/** @brief Verifies burst tick does nothing when remaining is zero */
void test_burst_tick_does_nothing_when_remaining_zero()
{
	State s = makeState();
	s.burstRemaining = 0;
	burstTick(s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
}

/** @brief Verifies burst tick sends the configured frame and decrements remaining */
void test_burst_tick_sends_frame()
{
	State s = makeState();
	Frame f = makeFrame(0x284, 5);
	f.data[0] = 0x20;
	s.burstFrame = f;
	s.burstBus = BUS_BODY;
	s.burstRemaining = 3;
	s.burstDelayMs = 20;
	s.burstLastMs = 0;
	fake_millis = 100;

	burstTick(s);
	TEST_ASSERT_EQUAL(1, stub_send_count);
	TEST_ASSERT_EQUAL(2, s.burstRemaining);
	TEST_ASSERT_EQUAL(0x284, stub_sends[0].f.id);
	TEST_ASSERT_EQUAL(BUS_BODY, stub_sends[0].bus);
}

/** @brief Verifies burst tick respects the configured delay interval */
void test_burst_tick_respects_delay()
{
	State s = makeState();
	Frame f = makeFrame(0x284, 5);
	s.burstFrame = f;
	s.burstBus = BUS_BODY;
	s.burstRemaining = 5;
	s.burstDelayMs = 100;
	s.burstLastMs = 90;
	fake_millis = 100;

	burstTick(s);
	TEST_ASSERT_EQUAL(0, stub_send_count);
	TEST_ASSERT_EQUAL(5, s.burstRemaining);
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_dispatch_caches_ctrl_frame);
	RUN_TEST(test_dispatch_caches_climate_frame);
	RUN_TEST(test_dispatch_caches_charge_frame);
	RUN_TEST(test_dispatch_caches_drive_frame);
	RUN_TEST(test_dispatch_ctrl_short_frame_ignored);

	RUN_TEST(test_dispatch_hw4_routes_to_hw4_handler);
	RUN_TEST(test_dispatch_hw3_routes_to_hw3_handler);
	RUN_TEST(test_dispatch_legacy_routes_to_legacy_handler);
	RUN_TEST(test_dispatch_ignores_handler_frames_from_bus1);

	RUN_TEST(test_apply_filters_hw4_sets_fsd_3_ids);
	RUN_TEST(test_apply_filters_hw3_sets_fsd_2_ids);
	RUN_TEST(test_apply_filters_legacy_sets_fsd_2_ids);
	RUN_TEST(test_apply_filters_raw_can_clears_all);
	RUN_TEST(test_apply_filters_sets_vehicle_and_body_buses);
	RUN_TEST(test_apply_filters_no_features_blocks_fsd_bus);
	RUN_TEST(test_apply_filters_nag_only_sets_mux_filter);
	RUN_TEST(test_apply_filters_isa_only_sets_isa_filter);
	RUN_TEST(test_apply_filters_fallback_adds_discriminator_ids_when_hw_unknown);
	RUN_TEST(test_apply_filters_no_fallback_ids_when_hw_already_detected);

	RUN_TEST(test_summon_tick_does_nothing_when_remaining_zero);
	RUN_TEST(test_summon_tick_does_nothing_without_ctrl);
	RUN_TEST(test_summon_tick_sends_burst_frame);
	RUN_TEST(test_summon_tick_respects_20ms_interval);
	RUN_TEST(test_summon_tick_decrements_to_zero);
	RUN_TEST(test_summon_tick_reverse_direction);

	RUN_TEST(test_autodetect_hw4_switches_variant);
	RUN_TEST(test_autodetect_hw3_switches_variant);
	RUN_TEST(test_autodetect_disabled_no_variant_switch);
	RUN_TEST(test_autodetect_same_variant_no_log);
	RUN_TEST(test_autodetect_invalid_hw_ignored);
	RUN_TEST(test_autodetect_legacy_hw0_switches_variant);

	RUN_TEST(test_fallback_isa_speed_infers_hw4_from_legacy);
	RUN_TEST(test_fallback_isa_speed_no_switch_if_already_hw4);
	RUN_TEST(test_fallback_legacy_mux_infers_legacy_from_hw4);
	RUN_TEST(test_fallback_no_switch_when_hw_already_detected);
	RUN_TEST(test_fallback_no_switch_when_autodetect_off);

	RUN_TEST(test_legacy_to_hw3_via_0x398_resets_speed_profile);
	RUN_TEST(test_legacy_to_hw4_via_0x398_resets_speed_profile);
	RUN_TEST(test_hw4_to_hw3_via_0x398_does_not_reset_speed_profile);

	RUN_TEST(test_burst_tick_does_nothing_when_remaining_zero);
	RUN_TEST(test_burst_tick_sends_frame);
	RUN_TEST(test_burst_tick_respects_delay);

	return UNITY_END();
}
