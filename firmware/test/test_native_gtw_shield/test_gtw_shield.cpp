/**
 * @file firmware/test/test_native_gtw_shield/test_gtw_shield.cpp
 * @brief Unit tests for gateway shield frame filtering
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

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

static int saveCount = 0;
void saveSettings(const State &)
{
	saveCount++;
}
void resetHandlerLogFlags() {}
void applyFilters(State &) {}

#include "core/util/parse.h"
#include "feature/ban_shield.h"
#include "support/helpers.h"

/** @brief Creates a default empty State for gateway shield tests */
static State makeState()
{
	State s = {};
	return s;
}

/** @brief Resets save counter before each test */
void setUp()
{
	saveCount = 0;
}

/** @brief Test fixture teardown — no cleanup required */
void tearDown() {}

/** @brief Verifies gtwshield:arm command sets the armed flag */
void test_gtw_cmd_arm()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeGtwShieldCmd("gtwshield:arm", s));
	TEST_ASSERT_TRUE(s.gtwShieldArmed);
}

/** @brief Verifies gtwshield:disarm command clears the armed flag */
void test_gtw_cmd_disarm()
{
	State s = makeState();
	s.gtwShieldArmed = true;
	TEST_ASSERT_TRUE(executeGtwShieldCmd("gtwshield:disarm", s));
	TEST_ASSERT_FALSE(s.gtwShieldArmed);
}

/** @brief Verifies gtwshield:reset disarms, clears block counter, and wipes snapshots */
void test_gtw_cmd_reset_clears_snapshots()
{
	State s = makeState();
	s.gtwShieldArmed = true;
	s.gtwShieldBlocks = 5;
	s.gtwSnapshotValid[0] = true;
	s.gtwSnapshot[0][0] = 0xAB;
	TEST_ASSERT_TRUE(executeGtwShieldCmd("gtwshield:reset", s));
	TEST_ASSERT_FALSE(s.gtwShieldArmed);
	TEST_ASSERT_EQUAL(0, s.gtwShieldBlocks);
	TEST_ASSERT_FALSE(s.gtwSnapshotValid[0]);
	TEST_ASSERT_EQUAL(0, s.gtwSnapshot[0][0]);
}

/** @brief Verifies unknown subcommand returns false */
void test_gtw_cmd_unknown_returns_false()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeGtwShieldCmd("gtwshield:bogus", s));
}

/** @brief Verifies unrelated command prefix returns false */
void test_gtw_cmd_unrelated_returns_false()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeGtwShieldCmd("banshield:on", s));
}

/** @brief Verifies learning mode captures a snapshot for the given mux index */
void test_gtw_learning_captures_snapshot_per_mux()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_GTW_CONFIG_ETH);
	f.data[0] = 0x02; // mux index 2
	f.data[3] = 0xDE;
	bool result = handleGtwShield(f, s);
	TEST_ASSERT_FALSE(result);
	TEST_ASSERT_TRUE(s.gtwSnapshotValid[2]);
	TEST_ASSERT_EQUAL(0xDE, s.gtwSnapshot[2][3]);
}

/** @brief Verifies learning mode does not overwrite an already-captured snapshot */
void test_gtw_learning_does_not_overwrite_existing_snapshot()
{
	State s = makeState();
	s.gtwSnapshotValid[1] = true;
	s.gtwSnapshot[1][0] = 0x01;
	Frame f = makeFrame(CAN_ID_GTW_CONFIG_ETH);
	f.data[0] = 0x01;
	f.data[0] = 0x01;
	f.data[1] = 0xFF;
	handleGtwShield(f, s);
	TEST_ASSERT_EQUAL(0x01, s.gtwSnapshot[1][0]);
}

/** @brief Verifies frames shorter than 8 bytes are ignored */
void test_gtw_short_frame_ignored()
{
	State s = makeState();
	Frame f = makeFrame(CAN_ID_GTW_CONFIG_ETH, 7);
	f.data[0] = 0x00;
	bool result = handleGtwShield(f, s);
	TEST_ASSERT_FALSE(result);
	TEST_ASSERT_FALSE(s.gtwSnapshotValid[0]);
}

/** @brief Verifies armed mode passes through frames that match the snapshot */
void test_gtw_armed_matching_frame_not_blocked()
{
	State s = makeState();
	s.gtwShieldArmed = true;
	s.gtwSnapshotValid[0] = true;
	for (uint8_t i = 0; i < 8; i++)
		s.gtwSnapshot[0][i] = (uint8_t)(i * 10);
	Frame f = makeFrame(CAN_ID_GTW_CONFIG_ETH);
	for (uint8_t i = 0; i < 8; i++)
		f.data[i] = (uint8_t)(i * 10);
	bool result = handleGtwShield(f, s);
	TEST_ASSERT_FALSE(result);
	TEST_ASSERT_EQUAL(0, s.gtwShieldBlocks);
}

/** @brief Verifies armed mode blocks a changed frame and restores snapshot data */
void test_gtw_armed_changed_frame_restores_snapshot()
{
	State s = makeState();
	s.gtwShieldArmed = true;
	s.gtwSnapshotValid[0] = true;
	for (uint8_t i = 0; i < 8; i++)
		s.gtwSnapshot[0][i] = 0xAA;
	Frame f = makeFrame(CAN_ID_GTW_CONFIG_ETH);
	for (uint8_t i = 0; i < 8; i++)
		f.data[i] = 0xAA;
	f.data[4] = 0xBB; // introduce a change
	f.data[0] = 0x00; // mux index 0
	s.gtwSnapshot[0][0] = 0x00;
	bool result = handleGtwShield(f, s);
	TEST_ASSERT_TRUE(result);
	TEST_ASSERT_EQUAL(1, s.gtwShieldBlocks);
	TEST_ASSERT_EQUAL_HEX8(0xAA, f.data[4]);
}

/** @brief Verifies armed mode skips mux indices without a captured snapshot */
void test_gtw_armed_no_snapshot_for_mux_skips()
{
	State s = makeState();
	s.gtwShieldArmed = true;
	Frame f = makeFrame(CAN_ID_GTW_CONFIG_ETH);
	f.data[0] = 0x03;
	bool result = handleGtwShield(f, s);
	TEST_ASSERT_FALSE(result);
}

/** @brief Verifies block counter increments on each blocked frame */
void test_gtw_armed_block_counter_increments()
{
	State s = makeState();
	s.gtwShieldArmed = true;
	s.gtwSnapshotValid[0] = true;
	for (uint8_t i = 0; i < 8; i++)
		s.gtwSnapshot[0][i] = 0x11;
	for (int rep = 0; rep < 3; rep++)
	{
		Frame f = makeFrame(CAN_ID_GTW_CONFIG_ETH);
		for (uint8_t i = 0; i < 8; i++)
			f.data[i] = (rep == 0 ? 0x22 : 0x33);
		f.data[0] = (uint8_t)(f.data[0] & 0xF8); // preserve mux index 0
		handleGtwShield(f, s);
	}
	TEST_ASSERT_EQUAL(3, s.gtwShieldBlocks);
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_gtw_cmd_arm);
	RUN_TEST(test_gtw_cmd_disarm);
	RUN_TEST(test_gtw_cmd_reset_clears_snapshots);
	RUN_TEST(test_gtw_cmd_unknown_returns_false);
	RUN_TEST(test_gtw_cmd_unrelated_returns_false);

	RUN_TEST(test_gtw_learning_captures_snapshot_per_mux);
	RUN_TEST(test_gtw_learning_does_not_overwrite_existing_snapshot);
	RUN_TEST(test_gtw_short_frame_ignored);

	RUN_TEST(test_gtw_armed_matching_frame_not_blocked);
	RUN_TEST(test_gtw_armed_changed_frame_restores_snapshot);
	RUN_TEST(test_gtw_armed_no_snapshot_for_mux_skips);
	RUN_TEST(test_gtw_armed_block_counter_increments);

	return UNITY_END();
}
