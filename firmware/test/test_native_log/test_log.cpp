// ── io/log.h Test ───────────────────────────────────────────────────────────
// log.h is a thin forward-declaration header (sendLog) that lets feature
// handlers emit log messages without depending on the platform serial layer.
// This test verifies the link-time contract: sendLog(const char*) and
// sendLog(const __FlashStringHelper*) exist and dispatch to the implementation.

#include <unity.h>
#include <cstring>
#include <string>

class __FlashStringHelper;
#define F(s) (reinterpret_cast<const __FlashStringHelper*>(s))

// Capture log output instead of writing to Serial
static std::string capturedLog;

#include "io/log.h"

// Provide implementation for the forward-declared sendLog overloads
void sendLog(const char* msg) { capturedLog += msg; capturedLog += "|c\n"; }
void sendLog(const __FlashStringHelper* msg) {
  capturedLog += reinterpret_cast<const char*>(msg);
  capturedLog += "|f\n";
}

void setUp() { capturedLog.clear(); }
void tearDown() {}

void test_sendLog_const_char_dispatches() {
  sendLog("hello");
  TEST_ASSERT_EQUAL_STRING("hello|c\n", capturedLog.c_str());
}
void test_sendLog_flash_string_dispatches() {
  sendLog(F("world"));
  TEST_ASSERT_EQUAL_STRING("world|f\n", capturedLog.c_str());
}
void test_sendLog_called_from_feature_pattern() {
  // Emulate a feature handler emitting a log line
  auto handlerLog = [](const char* event) { sendLog(event); };
  handlerLog("fsd:on:applied");
  TEST_ASSERT_EQUAL_STRING("fsd:on:applied|c\n", capturedLog.c_str());
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_sendLog_const_char_dispatches);
  RUN_TEST(test_sendLog_flash_string_dispatches);
  RUN_TEST(test_sendLog_called_from_feature_pattern);
  return UNITY_END();
}
