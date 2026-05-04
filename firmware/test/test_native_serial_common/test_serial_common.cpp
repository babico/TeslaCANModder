// ── Serial Common — JsonLineBuilder & handleChar Tests ──────────────────────
// Exercises the io/serial/common.h public API directly (JSON builder, ack/error
// helpers, character buffer state machine). Complements test_native_serial which
// goes through executeCommand; this focuses on the builder primitives.

#include <unity.h>
#include <cstring>
#include <string>

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

#include "core/types.h"
#include "vehicle/can/ids.h"

// ── Captured-output fakes ──────────────────────────────────────────────────
static std::string capturedOutput;
class __FlashStringHelper;
#define F(s) (reinterpret_cast<const __FlashStringHelper *>(s))

void printStr(const char *s)
{
	capturedOutput += s;
}
void printStr(const __FlashStringHelper *s)
{
	capturedOutput += reinterpret_cast<const char *>(s);
}
void printNum(long n)
{
	char buf[32];
	snprintf(buf, sizeof(buf), "%ld", n);
	capturedOutput += buf;
}
void printHex(uint8_t b)
{
	char buf[4];
	snprintf(buf, sizeof(buf), "%02X", b);
	capturedOutput += buf;
}
void printLn()
{
	capturedOutput += "\n";
}
unsigned long millis()
{
	return 0;
}

// Stub: handleChar references executeCommand — needs a definition
static int executedCount = 0;
static std::string lastExecuted;
void executeCommand(const char *cmd, State &, unsigned long)
{
	executedCount++;
	lastExecuted = cmd;
}

#include "io/serial/esp32/common.h"

void setUp()
{
	capturedOutput.clear();
	executedCount = 0;
	lastExecuted.clear();
}
void tearDown() {}

// ── JsonLineBuilder ─────────────────────────────────────────────────────────
void test_json_line_str_field()
{
	jsonLine().str("t", "ack").end();
	TEST_ASSERT_EQUAL_STRING("{\"t\":\"ack\"}\n", capturedOutput.c_str());
}
void test_json_line_multiple_fields()
{
	jsonLine().str("t", "log").num("n", 42).boolean("ok", true).end();
	TEST_ASSERT_EQUAL_STRING("{\"t\":\"log\",\"n\":42,\"ok\":1}\n", capturedOutput.c_str());
}
void test_json_line_hex_field()
{
	uint8_t data[] = {0x01, 0xAB, 0xFF};
	jsonLine().hex("d", data, 3).end();
	TEST_ASSERT_EQUAL_STRING("{\"d\":\"01ABFF\"}\n", capturedOutput.c_str());
}
void test_json_line_nested_object()
{
	jsonLine().object("inner", [](JsonLineBuilder::JsonObjectBuilder &o) { o.str("a", "1").num("b", 2); }).end();
	TEST_ASSERT_EQUAL_STRING("{\"inner\":{\"a\":\"1\",\"b\":2}}\n", capturedOutput.c_str());
}

// ── sendAck / sendError / sendLog ───────────────────────────────────────────
void test_send_ack()
{
	sendAck("fsd:on");
	TEST_ASSERT_EQUAL_STRING("{\"t\":\"ack\",\"cmd\":\"fsd:on\"}\n", capturedOutput.c_str());
}
void test_send_error()
{
	sendError("bad");
	TEST_ASSERT_EQUAL_STRING("{\"t\":\"error\",\"msg\":\"bad\"}\n", capturedOutput.c_str());
}
void test_send_log()
{
	sendLog("hello");
	TEST_ASSERT_EQUAL_STRING("{\"t\":\"log\",\"msg\":\"hello\"}\n", capturedOutput.c_str());
}

// ── sendFrame respects streamEnabled ────────────────────────────────────────
void test_send_frame_stream_disabled()
{
	State s = {};
	s.streamEnabled = false;
	Frame f = {};
	f.id = 0x123;
	f.dlc = 2;
	f.data[0] = 0xAB;
	f.data[1] = 0xCD;
	sendFrame(f, "rx", BUS_VEHICLE, 100, s);
	TEST_ASSERT_EQUAL_STRING("", capturedOutput.c_str());
	TEST_ASSERT_EQUAL_UINT32(0, s.streamCount);
}
void test_send_frame_stream_enabled_increments_count()
{
	State s = {};
	s.streamEnabled = true;
	s.streamCount = 0;
	Frame f = {};
	f.id = 0x123;
	f.dlc = 1;
	f.data[0] = 0xAA;
	sendFrame(f, "rx", BUS_VEHICLE, 100, s);
	TEST_ASSERT_EQUAL_UINT32(1, s.streamCount);
	TEST_ASSERT_TRUE(capturedOutput.find("\"t\":\"frame\"") != std::string::npos);
	TEST_ASSERT_TRUE(capturedOutput.find("\"id\":291") != std::string::npos); // 0x123 == 291
	TEST_ASSERT_TRUE(capturedOutput.find("\"d\":\"AA\"") != std::string::npos);
}

// ── handleChar — character-by-character command buffer ─────────────────────
void test_handleChar_collects_until_newline()
{
	State s = {};
	char buf[SERIAL_CMD_BUFFER_SIZE];
	uint8_t len = 0;
	const char *cmd = "fsd:on\n";
	for (size_t i = 0; cmd[i]; i++)
		handleChar(buf, len, cmd[i], s);
	TEST_ASSERT_EQUAL(1, executedCount);
	TEST_ASSERT_EQUAL_STRING("fsd:on", lastExecuted.c_str());
	TEST_ASSERT_EQUAL_UINT8(0, len);
}
void test_handleChar_ignores_carriage_return()
{
	State s = {};
	char buf[SERIAL_CMD_BUFFER_SIZE];
	uint8_t len = 0;
	const char *cmd = "ok\r\n";
	for (size_t i = 0; cmd[i]; i++)
		handleChar(buf, len, cmd[i], s);
	TEST_ASSERT_EQUAL(1, executedCount);
	TEST_ASSERT_EQUAL_STRING("ok", lastExecuted.c_str());
}
void test_handleChar_rejects_invalid_chars_resets_buffer()
{
	State s = {};
	char buf[SERIAL_CMD_BUFFER_SIZE];
	uint8_t len = 0;
	handleChar(buf, len, 'a', s);
	handleChar(buf, len, 'b', s);
	handleChar(buf, len, '!', s); // invalid → reset
	TEST_ASSERT_EQUAL_UINT8(0, len);
	// newline now should NOT execute (buffer empty)
	handleChar(buf, len, '\n', s);
	TEST_ASSERT_EQUAL(0, executedCount);
}
void test_handleChar_overflow_blocks_execute()
{
	State s = {};
	char buf[SERIAL_CMD_BUFFER_SIZE];
	uint8_t len = 0;
	// fill past capacity
	for (int i = 0; i < SERIAL_CMD_BUFFER_SIZE + 5; i++)
		handleChar(buf, len, 'a', s);
	handleChar(buf, len, '\n', s);
	// executeCommand only runs when len < SERIAL_CMD_BUFFER_SIZE
	TEST_ASSERT_EQUAL(0, executedCount);
}
void test_handleChar_accepts_alphanumeric_punctuation()
{
	State s = {};
	char buf[SERIAL_CMD_BUFFER_SIZE];
	uint8_t len = 0;
	const char *cmd = "Power_AC-9:off\n";
	for (size_t i = 0; cmd[i]; i++)
		handleChar(buf, len, cmd[i], s);
	TEST_ASSERT_EQUAL(1, executedCount);
	TEST_ASSERT_EQUAL_STRING("Power_AC-9:off", lastExecuted.c_str());
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_json_line_str_field);
	RUN_TEST(test_json_line_multiple_fields);
	RUN_TEST(test_json_line_hex_field);
	RUN_TEST(test_json_line_nested_object);
	RUN_TEST(test_send_ack);
	RUN_TEST(test_send_error);
	RUN_TEST(test_send_log);
	RUN_TEST(test_send_frame_stream_disabled);
	RUN_TEST(test_send_frame_stream_enabled_increments_count);
	RUN_TEST(test_handleChar_collects_until_newline);
	RUN_TEST(test_handleChar_ignores_carriage_return);
	RUN_TEST(test_handleChar_rejects_invalid_chars_resets_buffer);
	RUN_TEST(test_handleChar_overflow_blocks_execute);
	RUN_TEST(test_handleChar_accepts_alphanumeric_punctuation);
	return UNITY_END();
}
