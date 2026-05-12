/** @file firmware/test/test_native_id_filter/test_id_filter.cpp
 *  @brief Unit tests for CAN ID filter configuration
 *  @author Tesla CAN Mod Contributors
 *  @license GPL-3.0
 */

#include <unity.h>
#include <cstring>

#include "core/types.h"
#include "core/can/id_filter.h"

static IdFilter filter;

void setUp()
{
	idFilterClear(filter);
}
void tearDown() {}


void test_filter_clear_all_zero()
{
	idFilterAdd(filter, 0x100);
	idFilterClear(filter);
	for (int i = 0; i < ID_FILTER_WORDS; i++)
	{
		TEST_ASSERT_EQUAL_UINT32(0, filter.bits[i]);
	}
}


void test_filter_add_and_test_single_id()
{
	idFilterAdd(filter, 0x100);
	TEST_ASSERT_TRUE(idFilterTest(filter, 0x100));
}

void test_filter_test_unset_id_returns_false()
{
	TEST_ASSERT_FALSE(idFilterTest(filter, 0x200));
}

void test_filter_add_multiple_ids()
{
	idFilterAdd(filter, 0x001);
	idFilterAdd(filter, 0x100);
	idFilterAdd(filter, 0x7FF);
	TEST_ASSERT_TRUE(idFilterTest(filter, 0x001));
	TEST_ASSERT_TRUE(idFilterTest(filter, 0x100));
	TEST_ASSERT_TRUE(idFilterTest(filter, 0x7FF));
	TEST_ASSERT_FALSE(idFilterTest(filter, 0x002));
}

void test_filter_boundary_id_zero()
{
	idFilterAdd(filter, 0x000);
	TEST_ASSERT_TRUE(idFilterTest(filter, 0x000));
	TEST_ASSERT_FALSE(idFilterTest(filter, 0x001));
}

void test_filter_boundary_id_max()
{
	idFilterAdd(filter, 0x7FF);
	TEST_ASSERT_TRUE(idFilterTest(filter, 0x7FF));
	TEST_ASSERT_FALSE(idFilterTest(filter, 0x7FE));
}

void test_filter_add_is_idempotent()
{
	idFilterAdd(filter, 0x300);
	idFilterAdd(filter, 0x300);
	TEST_ASSERT_TRUE(idFilterTest(filter, 0x300));
}

void test_filter_adjacent_ids_independent()
{
	idFilterAdd(filter, 0x200);
	TEST_ASSERT_TRUE(idFilterTest(filter, 0x200));
	TEST_ASSERT_FALSE(idFilterTest(filter, 0x201));
	TEST_ASSERT_FALSE(idFilterTest(filter, 0x1FF));
}

void test_filter_all_ids_set()
{
	for (uint16_t id = 0; id < 2048; id++)
	{
		idFilterAdd(filter, id);
	}
	for (uint16_t id = 0; id < 2048; id++)
	{
		TEST_ASSERT_TRUE(idFilterTest(filter, id));
	}
}


int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_filter_clear_all_zero);
	RUN_TEST(test_filter_add_and_test_single_id);
	RUN_TEST(test_filter_test_unset_id_returns_false);
	RUN_TEST(test_filter_add_multiple_ids);
	RUN_TEST(test_filter_boundary_id_zero);
	RUN_TEST(test_filter_boundary_id_max);
	RUN_TEST(test_filter_add_is_idempotent);
	RUN_TEST(test_filter_adjacent_ids_independent);
	RUN_TEST(test_filter_all_ids_set);
	return UNITY_END();
}

