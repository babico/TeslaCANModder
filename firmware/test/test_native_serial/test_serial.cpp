// ── ESP32 Serial & Command Tests ─────────────────────────────────────────────
// Tests handleChar, executeCommand, sendAck, sendError, and JSON output.
// Uses a captured output buffer instead of real Serial.

#include <unity.h>
#include <cstring>
#include <string>

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0
#define BOARD_HW_NAME "ESP32S_DevKit"
#define BOARD_CAN_NAME "MCP2515_3x"
#define BOARD_DRIVER_NAME "arduino-mcp2515"

// Provide types
#include "core/types.h"
#include "vehicle/can/ids.h"

// ── Fake Serial / Arduino ───────────────────────────────────────────────────
static std::string capturedOutput;

class FakeSerial
{
  public:
	void print(const char *s)
	{
		capturedOutput += s;
	}
	void print(long n)
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "%ld", n);
		capturedOutput += buf;
	}
	void print(uint8_t b, int base)
	{
		if (base == 16)
		{
			char buf[4];
			snprintf(buf, sizeof(buf), "%X", b);
			capturedOutput += buf;
		}
	}
	void println()
	{
		capturedOutput += "\n";
	}
	int available()
	{
		return 0;
	}
};

static FakeSerial Serial;

// F() macro for native — just return the string
#define __FlashStringHelper char
#define F(s) (s)

static unsigned long fake_millis_val = 1000;
unsigned long millis()
{
	return fake_millis_val;
}

// ── Stub out driver, persist, dispatch functions ────────────────────────────
bool mcpAvailable[BUS_MAX] = {true, true, true};
static int save_settings_calls = 0;

void driverSetFilters(const uint32_t *, uint8_t) {}
void driverSetBusFilters(uint8_t, const uint32_t *, uint8_t) {}
void driverSend(const Frame &, uint8_t = 0) {}
void saveSettings(const State &)
{
	save_settings_calls++;
}

// Stub the needed handler fns
void resetHW4LogFlags() {}
void resetHW3LogFlags() {}
void resetLegacyLogFlags() {}

// ── Stub command modules ────────────────────────────────────────────────────
// These return true if the command matches, and modify state

bool executeStreamCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "stream:on") == 0)
	{
		s.streamEnabled = true;
		return true;
	}
	if (strcmp(cmd, "stream:off") == 0)
	{
		s.streamEnabled = false;
		return true;
	}
	return false;
}

bool executeCanRawCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "can:raw:on") == 0)
	{
		s.rawCanListen = true;
		return true;
	}
	if (strcmp(cmd, "can:raw:off") == 0)
	{
		s.rawCanListen = false;
		return true;
	}
	return false;
}

bool executeFsdCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "fsd:on") == 0)
	{
		s.fsdEnabled = true;
		return true;
	}
	if (strcmp(cmd, "fsd:off") == 0)
	{
		s.fsdEnabled = false;
		return true;
	}
	return false;
}

bool executeNagCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "nag:on") == 0)
	{
		s.nagSuppress = true;
		return true;
	}
	if (strcmp(cmd, "nag:off") == 0)
	{
		s.nagSuppress = false;
		return true;
	}
	return false;
}

bool executeProfileCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "profile:", 8) == 0)
	{
		s.speedProfile = atoi(cmd + 8);
		s.profileOverride = true;
		return true;
	}
	return false;
}

bool executeOffsetCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "offset:", 7) == 0)
	{
		s.speedOffset = atoi(cmd + 7);
		s.offsetOverride = true;
		return true;
	}
	return false;
}

bool executeIsaChimeCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "isa-chime:off") == 0)
	{
		s.isaChimeSuppress = true;
		return true;
	}
	if (strcmp(cmd, "isa-chime:on") == 0)
	{
		s.isaChimeSuppress = false;
		return true;
	}
	return false;
}

bool executeSummonCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "summon:fwd") == 0)
	{
		s.summonDirection = SUMMON_FORWARD;
		s.summonMode = SUMMON_START;
		s.summonRemaining = 30;
		return true;
	}
	if (strcmp(cmd, "summon:stop") == 0)
	{
		s.summonRemaining = 0;
		return true;
	}
	return false;
}

bool executeVariantCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "variant:", 8) == 0)
	{
		return parseVariant(cmd + 8, s.variant);
	}
	return false;
}

bool executeWindowCmd(const char *, State &)
{
	return false;
}
bool executeSentryCmd(const char *, State &)
{
	return false;
}
bool executeClimateCmd(const char *, State &)
{
	return false;
}
bool executeChargeCmd(const char *, State &)
{
	return false;
}
bool executePedalCmd(const char *, State &)
{
	return false;
}
bool executeRegenCmd(const char *, State &)
{
	return false;
}
bool executeStopCmd(const char *, State &)
{
	return false;
}
bool executeVehicleCmd(const char *, State &)
{
	return false;
}

// ── Now define the serial functions we're testing ───────────────────────────
// (Inline replicas from serial/esp32.h, adapted for test env)

void printStr(const char *s)
{
	Serial.print(s);
}
void printNum(long n)
{
	Serial.print(n);
}
void printHex(uint8_t b)
{
	if (b < 0x10)
		printStr("0");
	Serial.print(b, 16);
}
void printLn()
{
	Serial.println();
}

void sendAck(const char *cmd)
{
	printStr("{\"t\":\"ack\",\"cmd\":\"");
	printStr(cmd);
	printStr("\"}");
	printLn();
}

void sendError(const char *msg)
{
	printStr("{\"t\":\"error\",\"msg\":\"");
	printStr(msg);
	printStr("\"}");
	printLn();
}

void sendLog(const char *msg)
{
	printStr("{\"t\":\"log\",\"msg\":\"");
	printStr(msg);
	printStr("\"}");
	printLn();
}

void sendStatus(State &s, unsigned long now)
{
	// Simplified version — just emit key fields for testing
	printStr("{\"t\":\"status\",\"variant\":\"");
	printStr(variantName(s.variant));
	printStr("\",\"fsd\":");
	printNum(s.fsdEnabled ? 1 : 0);
	printStr(",\"sp\":");
	printNum(s.speedProfile);
	printStr(",\"busChassis\":");
	printNum(BUS_CHASSIS_ACTIVE);
	printStr(",\"busVehicle\":");
	printNum(BUS_VEHICLE_ACTIVE);
	printStr(",\"busBody\":");
	printNum(BUS_BODY_ACTIVE);
	printStr(",\"up\":");
	printNum(now);
	printStr("}");
	printLn();
}

void executeCommand(const char *cmd, State &s, unsigned long now)
{
	if (strcmp(cmd, "ping") == 0)
	{
		printStr("{\"t\":\"pong\",\"v\":1}");
		printLn();
		return;
	}
	if (strcmp(cmd, "status") == 0)
	{
		sendStatus(s, now);
		return;
	}
	if (strcmp(cmd, "apgate:on") == 0)
	{
		s.apInjectionGateEnabled = true;
		saveSettings(s);
		sendAck(cmd);
		sendStatus(s, now);
		return;
	}
	if (strcmp(cmd, "apgate:off") == 0)
	{
		s.apInjectionGateEnabled = false;
		saveSettings(s);
		sendAck(cmd);
		sendStatus(s, now);
		return;
	}
	if (strcmp(cmd, "apgate:status") == 0)
	{
		printStr("{\"t\":\"apgate\",\"enabled\":");
		printNum(s.apInjectionGateEnabled ? 1 : 0);
		printStr(",\"ap\":");
		printNum(s.apGateApActive ? 1 : 0);
		printStr(",\"park\":");
		printNum(s.apGateParked ? 1 : 0);
		printStr(",\"summon\":");
		printNum(s.apGateSummoning ? 1 : 0);
		printStr(",\"open\":");
		printNum(s.apGateOpen() ? 1 : 0);
		printStr("}");
		printLn();
		return;
	}
	if (executeStreamCmd(cmd, s))
	{
		sendAck(cmd);
		return;
	}
	if (executeCanRawCmd(cmd, s))
	{
		sendAck(cmd);
		return;
	}
	if (executeFsdCmd(cmd, s))
	{
		sendAck(cmd);
		return;
	}
	if (executeNagCmd(cmd, s))
	{
		sendAck(cmd);
		return;
	}
	if (executeProfileCmd(cmd, s))
	{
		sendAck(cmd);
		return;
	}
	if (executeOffsetCmd(cmd, s))
	{
		sendAck(cmd);
		return;
	}
	if (executeIsaChimeCmd(cmd, s))
	{
		sendAck(cmd);
		return;
	}
	if (executeSummonCmd(cmd, s))
	{
		sendAck(cmd);
		return;
	}
	if (executeVariantCmd(cmd, s))
	{
		sendAck(cmd);
		return;
	}
	sendError("Unknown command");
}

void handleChar(char *buf, uint8_t &len, char c, State &s)
{
	if (c == '\r')
		return;
	if (c == '\n')
	{
		if (len > 0 && len < 32)
		{
			buf[len] = '\0';
			executeCommand(buf, s, millis());
		}
		len = 0;
		return;
	}
	bool valid =
		(c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == ':' || c == '-' || c == '_';
	if (!valid)
	{
		len = 0;
		return;
	}
	if (len < 31)
	{
		buf[len++] = c;
	}
	else
	{
		len = 32;
	}
}

// ── Helpers ─────────────────────────────────────────────────────────────────

static State makeState(Variant v = HW4)
{
	State s = {};
	s.variant = v;
	s.speedProfile = 1;
	return s;
}

void setUp()
{
	capturedOutput.clear();
	fake_millis_val = 1000;
	save_settings_calls = 0;
}
void tearDown() {}

// ═══════════════════════════════════════════════════════════════════════════════
// executeCommand — Basic Commands
// ═══════════════════════════════════════════════════════════════════════════════

void test_cmd_ping()
{
	State s = makeState();
	executeCommand("ping", s, 1000);
	TEST_ASSERT_TRUE(capturedOutput.find("\"t\":\"pong\"") != std::string::npos);
	TEST_ASSERT_TRUE(capturedOutput.find("\"v\":1") != std::string::npos);
}

void test_cmd_status()
{
	State s = makeState(HW4);
	executeCommand("status", s, 5000);
	TEST_ASSERT_TRUE(capturedOutput.find("\"t\":\"status\"") != std::string::npos);
	TEST_ASSERT_TRUE(capturedOutput.find("\"variant\":\"hw4\"") != std::string::npos);
	TEST_ASSERT_TRUE(capturedOutput.find("\"up\":5000") != std::string::npos);
}

void test_cmd_fsd_on()
{
	State s = makeState();
	executeCommand("fsd:on", s, 1000);
	TEST_ASSERT_TRUE(s.fsdEnabled);
	TEST_ASSERT_TRUE(capturedOutput.find("\"t\":\"ack\"") != std::string::npos);
	TEST_ASSERT_TRUE(capturedOutput.find("\"cmd\":\"fsd:on\"") != std::string::npos);
}

void test_cmd_stream_on_off()
{
	State s = makeState();
	executeCommand("stream:on", s, 1000);
	TEST_ASSERT_TRUE(s.streamEnabled);
	capturedOutput.clear();
	executeCommand("stream:off", s, 1000);
	TEST_ASSERT_FALSE(s.streamEnabled);
}

void test_cmd_variant_hw3()
{
	State s = makeState(HW4);
	executeCommand("variant:hw3", s, 1000);
	TEST_ASSERT_EQUAL(HW3, s.variant);
}

void test_cmd_variant_legacy()
{
	State s = makeState();
	executeCommand("variant:legacy", s, 1000);
	TEST_ASSERT_EQUAL(LEGACY, s.variant);
}

void test_cmd_profile()
{
	State s = makeState();
	executeCommand("profile:5", s, 1000);
	TEST_ASSERT_EQUAL(5, s.speedProfile);
	TEST_ASSERT_TRUE(s.profileOverride);
}

void test_cmd_offset()
{
	State s = makeState();
	executeCommand("offset:10", s, 1000);
	TEST_ASSERT_EQUAL(10, s.speedOffset);
	TEST_ASSERT_TRUE(s.offsetOverride);
}

void test_cmd_isa_chime_off()
{
	State s = makeState();
	executeCommand("isa-chime:off", s, 1000);
	TEST_ASSERT_TRUE(s.isaChimeSuppress);
}

void test_cmd_nag_on()
{
	State s = makeState();
	executeCommand("nag:on", s, 1000);
	TEST_ASSERT_TRUE(s.nagSuppress);
}

void test_cmd_unknown()
{
	State s = makeState();
	executeCommand("garbage", s, 1000);
	TEST_ASSERT_TRUE(capturedOutput.find("Unknown command") != std::string::npos);
}

void test_cmd_raw_can()
{
	State s = makeState();
	executeCommand("can:raw:on", s, 1000);
	TEST_ASSERT_TRUE(s.rawCanListen);
}

void test_cmd_apgate_on_sets_state_and_persists()
{
	State s = makeState();
	s.apInjectionGateEnabled = false;
	executeCommand("apgate:on", s, 1000);
	TEST_ASSERT_TRUE(s.apInjectionGateEnabled);
	TEST_ASSERT_EQUAL(1, save_settings_calls);
	TEST_ASSERT_TRUE(capturedOutput.find("\"cmd\":\"apgate:on\"") != std::string::npos);
}

void test_cmd_apgate_off_sets_state_and_persists()
{
	State s = makeState();
	s.apInjectionGateEnabled = true;
	executeCommand("apgate:off", s, 1000);
	TEST_ASSERT_FALSE(s.apInjectionGateEnabled);
	TEST_ASSERT_EQUAL(1, save_settings_calls);
	TEST_ASSERT_TRUE(capturedOutput.find("\"cmd\":\"apgate:off\"") != std::string::npos);
}

void test_cmd_apgate_status_reports_reason_flags()
{
	State s = makeState();
	s.apInjectionGateEnabled = true;
	s.apGateApActive = false;
	s.apGateParked = true;
	s.apGateSummoning = false;
	executeCommand("apgate:status", s, 1000);
	TEST_ASSERT_TRUE(capturedOutput.find("\"t\":\"apgate\"") != std::string::npos);
	TEST_ASSERT_TRUE(capturedOutput.find("\"enabled\":1") != std::string::npos);
	TEST_ASSERT_TRUE(capturedOutput.find("\"park\":1") != std::string::npos);
	TEST_ASSERT_TRUE(capturedOutput.find("\"open\":1") != std::string::npos);
}

// ═══════════════════════════════════════════════════════════════════════════════
// handleChar — Buffer Parsing
// ═══════════════════════════════════════════════════════════════════════════════

void test_handlechar_builds_command_on_newline()
{
	State s = makeState();
	char buf[32] = {};
	uint8_t len = 0;
	const char *input = "ping\n";
	for (size_t i = 0; i < strlen(input); i++)
		handleChar(buf, len, input[i], s);
	TEST_ASSERT_TRUE(capturedOutput.find("\"t\":\"pong\"") != std::string::npos);
	TEST_ASSERT_EQUAL(0, len);
}

void test_handlechar_ignores_cr()
{
	State s = makeState();
	char buf[32] = {};
	uint8_t len = 0;
	const char *input = "ping\r\n";
	for (size_t i = 0; i < strlen(input); i++)
		handleChar(buf, len, input[i], s);
	TEST_ASSERT_TRUE(capturedOutput.find("\"t\":\"pong\"") != std::string::npos);
}

void test_handlechar_rejects_invalid_chars()
{
	State s = makeState();
	char buf[32] = {};
	uint8_t len = 0;
	// Inject an invalid char (space) mid-command → should clear buffer
	handleChar(buf, len, 'p', s);
	handleChar(buf, len, 'i', s);
	TEST_ASSERT_EQUAL(2, len);
	handleChar(buf, len, ' ', s); // invalid
	TEST_ASSERT_EQUAL(0, len);
}

void test_handlechar_empty_line_does_nothing()
{
	State s = makeState();
	char buf[32] = {};
	uint8_t len = 0;
	handleChar(buf, len, '\n', s);
	TEST_ASSERT_TRUE(capturedOutput.empty());
}

void test_handlechar_overflow_ignored()
{
	State s = makeState();
	char buf[32] = {};
	uint8_t len = 0;
	// Fill buffer to 31 chars
	for (int i = 0; i < 35; i++)
		handleChar(buf, len, 'a', s);
	// len should be 32 (overflow marker)
	TEST_ASSERT_EQUAL(32, len);
	// Newline should NOT execute (len >= 32)
	handleChar(buf, len, '\n', s);
	TEST_ASSERT_TRUE(capturedOutput.empty());
}

// ═══════════════════════════════════════════════════════════════════════════════
// sendAck / sendError / sendLog format
// ═══════════════════════════════════════════════════════════════════════════════

void test_send_ack_format()
{
	sendAck("fsd:on");
	TEST_ASSERT_EQUAL_STRING("{\"t\":\"ack\",\"cmd\":\"fsd:on\"}\n", capturedOutput.c_str());
}

void test_send_error_format()
{
	sendError("bad input");
	TEST_ASSERT_EQUAL_STRING("{\"t\":\"error\",\"msg\":\"bad input\"}\n", capturedOutput.c_str());
}

void test_send_log_format()
{
	sendLog("hello");
	TEST_ASSERT_EQUAL_STRING("{\"t\":\"log\",\"msg\":\"hello\"}\n", capturedOutput.c_str());
}

int main()
{
	UNITY_BEGIN();

	// Commands
	RUN_TEST(test_cmd_ping);
	RUN_TEST(test_cmd_status);
	RUN_TEST(test_cmd_fsd_on);
	RUN_TEST(test_cmd_stream_on_off);
	RUN_TEST(test_cmd_variant_hw3);
	RUN_TEST(test_cmd_variant_legacy);
	RUN_TEST(test_cmd_profile);
	RUN_TEST(test_cmd_offset);
	RUN_TEST(test_cmd_isa_chime_off);
	RUN_TEST(test_cmd_nag_on);
	RUN_TEST(test_cmd_unknown);
	RUN_TEST(test_cmd_raw_can);
	RUN_TEST(test_cmd_apgate_on_sets_state_and_persists);
	RUN_TEST(test_cmd_apgate_off_sets_state_and_persists);
	RUN_TEST(test_cmd_apgate_status_reports_reason_flags);

	// handleChar
	RUN_TEST(test_handlechar_builds_command_on_newline);
	RUN_TEST(test_handlechar_ignores_cr);
	RUN_TEST(test_handlechar_rejects_invalid_chars);
	RUN_TEST(test_handlechar_empty_line_does_nothing);
	RUN_TEST(test_handlechar_overflow_ignored);

	// Message format
	RUN_TEST(test_send_ack_format);
	RUN_TEST(test_send_error_format);
	RUN_TEST(test_send_log_format);

	return UNITY_END();
}
