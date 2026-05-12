/** @file firmware/test/test_native_log/test_log.cpp
 *  @brief Unit tests for log ring buffer
 *  @author Tesla CAN Mod Contributors
 *  @license GPL-3.0
 */

#include <unity.h>
#include <cstring>
#include <string>

class __FlashStringHelper;
#define F(s) (reinterpret_cast<const __FlashStringHelper *>(s))

static std::string capturedLog;

#include "io/log.h"

void sendLog(const char *msg)
{
	capturedLog += msg;
	capturedLog += "|c\n";
}
void sendLog(const __FlashStringHelper *msg)
{
	capturedLog += reinterpret_cast<const char *>(msg);
	capturedLog += "|f\n";
}

void setUp()
{
	capturedLog.clear();
}
void tearDown() {}

void test_sendLog_const_char_dispatches()
{
	sendLog("hello");
	TEST_ASSERT_EQUAL_STRING("hello|c\n", capturedLog.c_str());
}
void test_sendLog_flash_string_dispatches()
{
	sendLog(F("world"));
	TEST_ASSERT_EQUAL_STRING("world|f\n", capturedLog.c_str());
}
void test_sendLog_called_from_feature_pattern()
{
	auto handlerLog = [](const char *event) { sendLog(event); };
	handlerLog("fsd:on:applied");
	TEST_ASSERT_EQUAL_STRING("fsd:on:applied|c\n", capturedLog.c_str());
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_sendLog_const_char_dispatches);
	RUN_TEST(test_sendLog_flash_string_dispatches);
	RUN_TEST(test_sendLog_called_from_feature_pattern);
	return UNITY_END();
}

