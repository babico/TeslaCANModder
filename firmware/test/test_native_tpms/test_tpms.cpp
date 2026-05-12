/** @file firmware/test/test_native_tpms/test_tpms.cpp
 *  @brief Unit tests for tire pressure monitoring
 *  @author Tesla CAN Mod Contributors
 *  @license GPL-3.0
 */

#include <unity.h>
#include <cstring>

#include "core/types.h"
#include "vehicle/can/ids.h"
#include "feature/tpms.h"

void setUp() {}
void tearDown() {}


void test_tpms_decode_all_zeros()
{
	Frame f = {};
	f.id = CAN_ID_TPMS;
	f.dlc = 8;
	memset(f.data, 0, 8);
	State s = {};
	decodeTpms(f, s);
	TEST_ASSERT_TRUE(s.hasTpms);
	TEST_ASSERT_FLOAT_WITHIN(0.001, 0.0, s.tpmsPressure[0]);
	TEST_ASSERT_FLOAT_WITHIN(0.001, 0.0, s.tpmsPressure[1]);
	TEST_ASSERT_FLOAT_WITHIN(0.001, 0.0, s.tpmsPressure[2]);
	TEST_ASSERT_FLOAT_WITHIN(0.001, 0.0, s.tpmsPressure[3]);
	TEST_ASSERT_EQUAL_INT8(-40, s.tpmsTemp[0]);
	TEST_ASSERT_EQUAL_INT8(-40, s.tpmsTemp[1]);
	TEST_ASSERT_EQUAL_INT8(-40, s.tpmsTemp[2]);
	TEST_ASSERT_EQUAL_INT8(-40, s.tpmsTemp[3]);
}

void test_tpms_decode_known_pressure()
{
	Frame f = {};
	f.id = CAN_ID_TPMS;
	f.dlc = 8;
	f.data[0] = 120;
	f.data[1] = 100;
	f.data[2] = 80;
	f.data[3] = 140;
	f.data[4] = 65;
	f.data[5] = 70;
	f.data[6] = 60;
	f.data[7] = 80;
	State s = {};
	decodeTpms(f, s);
	TEST_ASSERT_TRUE(s.hasTpms);
	TEST_ASSERT_FLOAT_WITHIN(0.001, 3.0, s.tpmsPressure[0]);
	TEST_ASSERT_FLOAT_WITHIN(0.001, 2.5, s.tpmsPressure[1]);
	TEST_ASSERT_FLOAT_WITHIN(0.001, 2.0, s.tpmsPressure[2]);
	TEST_ASSERT_FLOAT_WITHIN(0.001, 3.5, s.tpmsPressure[3]);
	TEST_ASSERT_EQUAL_INT8(25, s.tpmsTemp[0]);
	TEST_ASSERT_EQUAL_INT8(30, s.tpmsTemp[1]);
	TEST_ASSERT_EQUAL_INT8(20, s.tpmsTemp[2]);
	TEST_ASSERT_EQUAL_INT8(40, s.tpmsTemp[3]);
}

void test_tpms_decode_max_values()
{
	Frame f = {};
	f.id = CAN_ID_TPMS;
	f.dlc = 8;
	memset(f.data, 0xFF, 8);
	State s = {};
	decodeTpms(f, s);
	TEST_ASSERT_FLOAT_WITHIN(0.001, 6.375, s.tpmsPressure[0]);
	TEST_ASSERT_EQUAL_INT8((int8_t)(255 - 40), s.tpmsTemp[0]);
}

void test_tpms_sets_hasTpms_flag()
{
	Frame f = {};
	f.id = CAN_ID_TPMS;
	f.dlc = 8;
	f.data[0] = 100;
	State s = {};
	s.hasTpms = false;
	decodeTpms(f, s);
	TEST_ASSERT_TRUE(s.hasTpms);
}

void test_tpms_preserves_other_state()
{
	Frame f = {};
	f.id = CAN_ID_TPMS;
	f.dlc = 8;
	f.data[0] = 80;
	State s = {};
	s.fsdEnabled = true;
	s.variant = HW4;
	decodeTpms(f, s);
	TEST_ASSERT_TRUE(s.fsdEnabled);
	TEST_ASSERT_EQUAL(HW4, s.variant);
}


int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_tpms_decode_all_zeros);
	RUN_TEST(test_tpms_decode_known_pressure);
	RUN_TEST(test_tpms_decode_max_values);
	RUN_TEST(test_tpms_sets_hasTpms_flag);
	RUN_TEST(test_tpms_preserves_other_state);
	return UNITY_END();
}

