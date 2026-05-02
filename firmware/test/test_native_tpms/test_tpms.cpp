// ── TPMS Decode Tests ────────────────────────────────────────────────────────
// Tests TPMS tire pressure and temperature decoding from CAN 0x219.

#include <unity.h>
#include <cstring>

#include "core/types.h"
#include "infra/can.h"
#include "feature/tpms.h"

void setUp() {}
void tearDown() {}

// ── decodeTpms ───────────────────────────────────────────────────────────────

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
	// temp = raw - 40, so 0 - 40 = -40
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
	// Pressure = raw * 0.025 bar
	// raw = 120 -> 3.0 bar
	f.data[0] = 120; // FL pressure
	f.data[1] = 100; // FR pressure -> 2.5 bar
	f.data[2] = 80;	 // RL pressure -> 2.0 bar
	f.data[3] = 140; // RR pressure -> 3.5 bar
	// Temps
	f.data[4] = 65; // FL temp -> 65 - 40 = 25°C
	f.data[5] = 70; // FR temp -> 30°C
	f.data[6] = 60; // RL temp -> 20°C
	f.data[7] = 80; // RR temp -> 40°C
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
	// 255 * 0.025 = 6.375 bar
	TEST_ASSERT_FLOAT_WITHIN(0.001, 6.375, s.tpmsPressure[0]);
	// 255 - 40 = 215°C
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

// ── main ─────────────────────────────────────────────────────────────────────

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
