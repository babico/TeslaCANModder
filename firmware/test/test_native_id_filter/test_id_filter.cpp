/**
 * @file firmware/test/test_native_id_filter/test_id_filter.cpp
 * @brief Unit tests for CAN ID bitmask filter
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <unity.h>
#include <cstring>

#include "core/can/id_filter.h"

void setUp()
{
	swFilterInit();
}
void tearDown() {}

/* ── idFilterClear ───────────────────────────────────────────────────────── */

void test_id_filter_clear_all_zeros()
{
	IdFilter f;
	memset(&f, 0xFF, sizeof(f));
	idFilterClear(f);
	for (int i = 0; i < ID_FILTER_WORDS; i++)
		TEST_ASSERT_EQUAL_UINT32(0, f.bits[i]);
}

/* ── idFilterAdd / idFilterTest ──────────────────────────────────────────── */

void test_id_filter_add_and_test_id_0()
{
	IdFilter f;
	idFilterClear(f);
	idFilterAdd(f, 0);
	TEST_ASSERT_TRUE(idFilterTest(f, 0));
}

void test_id_filter_add_and_test_id_1021()
{
	IdFilter f;
	idFilterClear(f);
	idFilterAdd(f, 1021);
	TEST_ASSERT_TRUE(idFilterTest(f, 1021));
}

void test_id_filter_add_and_test_id_2047()
{
	IdFilter f;
	idFilterClear(f);
	idFilterAdd(f, 2047);
	TEST_ASSERT_TRUE(idFilterTest(f, 2047));
}

void test_id_filter_rejects_unadded_id()
{
	IdFilter f;
	idFilterClear(f);
	idFilterAdd(f, 100);
	TEST_ASSERT_FALSE(idFilterTest(f, 200));
}

void test_id_filter_rejects_id_beyond_2047()
{
	IdFilter f;
	idFilterClear(f);
	idFilterAdd(f, 100);
	TEST_ASSERT_FALSE(idFilterTest(f, 2048));
	TEST_ASSERT_FALSE(idFilterTest(f, 3000));
}

void test_id_filter_add_ignores_id_beyond_2047()
{
	IdFilter f;
	idFilterClear(f);
	idFilterAdd(f, 3000);
	TEST_ASSERT_FALSE(idFilterTest(f, 3000));
}

/* ── idFilterRemove ──────────────────────────────────────────────────────── */

void test_id_filter_remove_makes_id_fail()
{
	IdFilter f;
	idFilterClear(f);
	idFilterAdd(f, 500);
	TEST_ASSERT_TRUE(idFilterTest(f, 500));
	idFilterRemove(f, 500);
	TEST_ASSERT_FALSE(idFilterTest(f, 500));
}

void test_id_filter_remove_does_not_affect_other_ids()
{
	IdFilter f;
	idFilterClear(f);
	idFilterAdd(f, 100);
	idFilterAdd(f, 200);
	idFilterRemove(f, 100);
	TEST_ASSERT_FALSE(idFilterTest(f, 100));
	TEST_ASSERT_TRUE(idFilterTest(f, 200));
}

/* ── Multiple IDs ────────────────────────────────────────────────────────── */

void test_id_filter_multiple_ids()
{
	IdFilter f;
	idFilterClear(f);
	idFilterAdd(f, 0);
	idFilterAdd(f, 1021);
	idFilterAdd(f, 2047);
	TEST_ASSERT_TRUE(idFilterTest(f, 0));
	TEST_ASSERT_TRUE(idFilterTest(f, 1021));
	TEST_ASSERT_TRUE(idFilterTest(f, 2047));
	TEST_ASSERT_FALSE(idFilterTest(f, 1));
	TEST_ASSERT_FALSE(idFilterTest(f, 1020));
}

/* ── swFilterInit / swFilterAccept ───────────────────────────────────────── */

void test_sw_filter_init_clears_all()
{
	idFilterAdd(swFilterBus0, 100);
	idFilterAdd(swFilterBus1, 200);
	swFilterInit();
	TEST_ASSERT_FALSE(idFilterTest(swFilterBus0, 100));
	TEST_ASSERT_FALSE(idFilterTest(swFilterBus1, 200));
}

void test_sw_filter_accept_bus_0()
{
	idFilterAdd(swFilterBus0, 42);
	TEST_ASSERT_TRUE(swFilterAccept(0, 42));
	TEST_ASSERT_FALSE(swFilterAccept(0, 99));
}

void test_sw_filter_accept_bus_1()
{
	idFilterAdd(swFilterBus1, 42);
	TEST_ASSERT_TRUE(swFilterAccept(1, 42));
	TEST_ASSERT_FALSE(swFilterAccept(1, 99));
}

void test_sw_filter_accept_bus_2_always_true()
{
	TEST_ASSERT_TRUE(swFilterAccept(2, 0));
	TEST_ASSERT_TRUE(swFilterAccept(2, 2047));
	TEST_ASSERT_TRUE(swFilterAccept(2, 999));
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_id_filter_clear_all_zeros);
	RUN_TEST(test_id_filter_add_and_test_id_0);
	RUN_TEST(test_id_filter_add_and_test_id_1021);
	RUN_TEST(test_id_filter_add_and_test_id_2047);
	RUN_TEST(test_id_filter_rejects_unadded_id);
	RUN_TEST(test_id_filter_rejects_id_beyond_2047);
	RUN_TEST(test_id_filter_add_ignores_id_beyond_2047);
	RUN_TEST(test_id_filter_remove_makes_id_fail);
	RUN_TEST(test_id_filter_remove_does_not_affect_other_ids);
	RUN_TEST(test_id_filter_multiple_ids);
	RUN_TEST(test_sw_filter_init_clears_all);
	RUN_TEST(test_sw_filter_accept_bus_0);
	RUN_TEST(test_sw_filter_accept_bus_1);
	RUN_TEST(test_sw_filter_accept_bus_2_always_true);

	return UNITY_END();
}
