/**
 * @file firmware/test/test_native_parse/test_parse.cpp
 * @brief Unit tests for parseBoolCmd utility
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <unity.h>
#include <cstring>

#include "core/util/parse.h"

void setUp() {}
void tearDown() {}

void test_parse_bool_cmd_on()
{
	bool out = false;
	TEST_ASSERT_TRUE(parseBoolCmd("on", false, out));
	TEST_ASSERT_TRUE(out);
}

void test_parse_bool_cmd_off()
{
	bool out = true;
	TEST_ASSERT_TRUE(parseBoolCmd("off", true, out));
	TEST_ASSERT_FALSE(out);
}

void test_parse_bool_cmd_rejects_unknown()
{
	bool out = false;
	TEST_ASSERT_FALSE(parseBoolCmd("maybe", false, out));
}

void test_parse_bool_cmd_rejects_empty()
{
	bool out = false;
	TEST_ASSERT_FALSE(parseBoolCmd("", false, out));
}

void test_parse_bool_cmd_rejects_case_variant()
{
	bool out = false;
	TEST_ASSERT_FALSE(parseBoolCmd("ON", false, out));
	TEST_ASSERT_FALSE(parseBoolCmd("Off", false, out));
}

void test_parse_bool_cmd_current_unused()
{
	bool out = false;
	TEST_ASSERT_TRUE(parseBoolCmd("on", true, out));
	TEST_ASSERT_TRUE(out);
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_parse_bool_cmd_on);
	RUN_TEST(test_parse_bool_cmd_off);
	RUN_TEST(test_parse_bool_cmd_rejects_unknown);
	RUN_TEST(test_parse_bool_cmd_rejects_empty);
	RUN_TEST(test_parse_bool_cmd_rejects_case_variant);
	RUN_TEST(test_parse_bool_cmd_current_unused);

	return UNITY_END();
}
