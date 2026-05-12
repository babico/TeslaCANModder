/** @file firmware/test/test_native_state_persistence/test_state_persistence.cpp
 *  @brief Unit tests for state persistence across reboots
 *  @author Tesla CAN Mod Contributors
 *  @license GPL-3.0
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

void setUp() {}
void tearDown() {}

static State roundtrip(const State &in)
{
	uint8_t blob[sizeof(State)];
	memcpy(blob, &in, sizeof(State));
	State out;
	memcpy(&out, blob, sizeof(State));
	return out;
}

void test_persist_preserves_feature_toggles()
{
	State s = {};
	s.variant = HW4;
	s.fsdEnabled = true;
	s.nagMode = NAG_MODE_ORGANIC;
	s.preconditionEnabled = true;
	s.trackModeEnabled = true;
	s.streamEnabled = true;
	s.rawCanListen = true;
	s.mirrorAutoFoldEnabled = true;
	s.wiperPersistEnabled = true;
	s.savedWiperSpeed = 2;
	s.speedProfile = 5;
	s.profileOverride = true;

	State r = roundtrip(s);
	TEST_ASSERT_EQUAL(HW4, r.variant);
	TEST_ASSERT_TRUE(r.fsdEnabled);
	TEST_ASSERT_EQUAL(NAG_MODE_ORGANIC, r.nagMode);
	TEST_ASSERT_TRUE(r.preconditionEnabled);
	TEST_ASSERT_TRUE(r.trackModeEnabled);
	TEST_ASSERT_TRUE(r.streamEnabled);
	TEST_ASSERT_TRUE(r.rawCanListen);
	TEST_ASSERT_TRUE(r.mirrorAutoFoldEnabled);
	TEST_ASSERT_TRUE(r.wiperPersistEnabled);
	TEST_ASSERT_EQUAL_UINT8(2, r.savedWiperSpeed);
	TEST_ASSERT_EQUAL_INT8(5, r.speedProfile);
	TEST_ASSERT_TRUE(r.profileOverride);
}

void test_persist_zero_initialized_state_roundtrip_clean()
{
	State s = {};
	State r = roundtrip(s);
	TEST_ASSERT_FALSE(r.fsdEnabled);
	TEST_ASSERT_EQUAL(NAG_MODE_OFF, r.nagMode);
	TEST_ASSERT_FALSE(r.streamEnabled);
	TEST_ASSERT_FALSE(r.preconditionEnabled);
}

void test_persist_distinct_states_remain_distinct()
{
	State a = {};
	a.variant = HW4;
	a.fsdEnabled = true;
	State b = {};
	b.variant = LEGACY;
	b.fsdEnabled = false;
	State ar = roundtrip(a), br = roundtrip(b);
	TEST_ASSERT_TRUE(ar.fsdEnabled);
	TEST_ASSERT_FALSE(br.fsdEnabled);
	TEST_ASSERT_EQUAL(HW4, ar.variant);
	TEST_ASSERT_EQUAL(LEGACY, br.variant);
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_persist_preserves_feature_toggles);
	RUN_TEST(test_persist_zero_initialized_state_roundtrip_clean);
	RUN_TEST(test_persist_distinct_states_remain_distinct);
	return UNITY_END();
}

