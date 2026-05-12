/** @file firmware/test/test_native_wifi/test_wifi.cpp
 *  @brief Unit tests for WiFi transport initialization
 *  @author Tesla CAN Mod Contributors
 *  @license GPL-3.0
 */

#include <unity.h>
#include <cstring>
#include <string>

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 1
#define BOARD_ENABLE_BLE 0
#define BOARD_HW_NAME "ESP32S_DevKit"
#define BOARD_CAN_NAME "MCP2515_3x"
#define BOARD_DRIVER_NAME "arduino-mcp2515"

#include "core/types.h"
#include "vehicle/can/ids.h"

unsigned long millis()
{
	return 5000;
}


static std::string buildStateJson(State &s)
{
	Features feat = getFeatures(s.variant);
	std::string json = "{";
	json += "\"variant\":\"" + std::string(variantName(s.variant)) + "\"";
	json += ",\"fsd\":" + std::to_string(s.fsdEnabled ? 1 : 0);
	json += ",\"nagMode\":\"" + std::string(nagModeName(s.nagMode)) + "\"";
	json += ",\"nagOrgBypass\":" + std::to_string(s.nagOrganicDriverBypass ? 1 : 0);
	json += ",\"profile\":" + std::to_string(s.speedProfile);
	json += ",\"profilePin\":" + std::to_string(s.profileOverride ? 1 : 0);
	json += ",\"offset\":" + std::to_string(s.speedOffset);
	json += ",\"offsetPin\":" + std::to_string(s.offsetOverride ? 1 : 0);
	json += ",\"banShield\":" + std::to_string(s.banShieldEnabled ? 1 : 0);
	json += ",\"banThreat\":" + std::to_string(s.banThreatLevel);
	json += ",\"banDetectCount\":" + std::to_string(s.banDetectionCount);
	json += ",\"isaChime\":" + std::to_string(s.isaChimeSuppress ? 1 : 0);
	json += ",\"stream\":" + std::to_string(s.streamEnabled ? 1 : 0);
	json += ",\"rawCan\":" + std::to_string(s.rawCanListen ? 1 : 0);
	json += ",\"chassisOnline\":" + std::to_string(s.chassisOnline ? 1 : 0);
	json += ",\"standby\":" + std::to_string(s.standby ? 1 : 0);
	json += ",\"uptime\":" + std::to_string((long)millis());

	json += ",\"features\":{";
	json += "\"fsd\":" + std::to_string(feat.fsd ? 1 : 0);
	json += ",\"profile\":" + std::to_string(feat.profile ? 1 : 0);
	json += ",\"nag\":" + std::to_string(feat.nag ? 1 : 0);
	json += ",\"offset\":" + std::to_string(feat.offset ? 1 : 0);
	json += ",\"isaChime\":" + std::to_string(feat.isaChime ? 1 : 0);
	json += ",\"summon\":" + std::to_string(feat.summon ? 1 : 0);
	json += "}";

	json += ",\"hardware\":{";
	json += "\"board\":\"" BOARD_HW_NAME "\"";
	json += ",\"can\":\"" BOARD_CAN_NAME "\"";
	json += ",\"busChassis\":" + std::to_string(BUS_CHASSIS_ACTIVE);
	json += ",\"busVehicle\":" + std::to_string(BUS_VEHICLE_ACTIVE);
	json += ",\"busBody\":" + std::to_string(BUS_BODY_ACTIVE);
	json += ",\"ble\":false";
	json += ",\"wifi\":true";
	json += "}";

	json += "}";
	return json;
}

static bool isValidCommand(const char *cmd)
{
	if (!cmd || strlen(cmd) == 0 || strlen(cmd) > 31)
		return false;
	for (size_t i = 0; i < strlen(cmd); i++)
	{
		char c = cmd[i];
		bool valid = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == ':' ||
					 c == '-' || c == '_';
		if (!valid)
			return false;
	}
	return true;
}

static State makeState(Variant v = HW4)
{
	State s = {};
	s.variant = v;
	s.speedProfile = 1;
	return s;
}

void setUp() {}
void tearDown() {}


void test_wifi_json_contains_variant()
{
	State s = makeState(HW4);
	std::string json = buildStateJson(s);
	TEST_ASSERT_TRUE(json.find("\"variant\":\"hw4\"") != std::string::npos);
}

void test_wifi_json_contains_hw3_variant()
{
	State s = makeState(HW3);
	std::string json = buildStateJson(s);
	TEST_ASSERT_TRUE(json.find("\"variant\":\"hw3\"") != std::string::npos);
}

void test_wifi_json_fsd_enabled()
{
	State s = makeState();
	s.fsdEnabled = true;
	std::string json = buildStateJson(s);
	TEST_ASSERT_TRUE(json.find("\"fsd\":1") != std::string::npos);
}

void test_wifi_json_fsd_disabled()
{
	State s = makeState();
	s.fsdEnabled = false;
	std::string json = buildStateJson(s);
	TEST_ASSERT_TRUE(json.find("\"fsd\":0") != std::string::npos);
}

void test_wifi_json_contains_hardware_block()
{
	State s = makeState();
	std::string json = buildStateJson(s);
	TEST_ASSERT_TRUE(json.find("\"board\":\"ESP32S_DevKit\"") != std::string::npos);
	TEST_ASSERT_TRUE(json.find("\"busChassis\":1") != std::string::npos);
	TEST_ASSERT_TRUE(json.find("\"busVehicle\":1") != std::string::npos);
	TEST_ASSERT_TRUE(json.find("\"busBody\":1") != std::string::npos);
	TEST_ASSERT_TRUE(json.find("\"wifi\":true") != std::string::npos);
	TEST_ASSERT_TRUE(json.find("\"ble\":false") != std::string::npos);
}

void test_wifi_json_hw4_features()
{
	State s = makeState(HW4);
	std::string json = buildStateJson(s);
	TEST_ASSERT_TRUE(json.find("\"isaChime\":1") != std::string::npos);
	TEST_ASSERT_TRUE(json.find("\"summon\":1") != std::string::npos);
	TEST_ASSERT_TRUE(json.find("\"offset\":1") != std::string::npos);
}

void test_wifi_json_hw3_features()
{
	State s = makeState(HW3);
	std::string json = buildStateJson(s);
	TEST_ASSERT_TRUE(json.find("\"offset\":1") != std::string::npos);
	TEST_ASSERT_TRUE(json.find("\"summon\":1") != std::string::npos);
	TEST_ASSERT_TRUE(json.find("\"isaChime\":0") != std::string::npos);
}

void test_wifi_json_all_state_fields()
{
	State s = makeState();
	s.fsdEnabled = true;
	s.nagMode = NAG_MODE_ORGANIC;
	s.speedProfile = 5;
	s.profileOverride = true;
	s.speedOffset = 10;
	s.offsetOverride = true;
	s.isaChimeSuppress = true;
	s.streamEnabled = true;
	s.rawCanListen = true;
	s.chassisOnline = true;
	s.standby = false;

	std::string json = buildStateJson(s);
	TEST_ASSERT_TRUE(json.find("\"fsd\":1") != std::string::npos);
	TEST_ASSERT_TRUE(json.find("\"nag\":1") != std::string::npos);
	TEST_ASSERT_TRUE(json.find("\"profile\":5") != std::string::npos);
	TEST_ASSERT_TRUE(json.find("\"profilePin\":1") != std::string::npos);
	TEST_ASSERT_TRUE(json.find("\"offset\":10") != std::string::npos);
	TEST_ASSERT_TRUE(json.find("\"offsetPin\":1") != std::string::npos);
	TEST_ASSERT_TRUE(json.find("\"stream\":1") != std::string::npos);
	TEST_ASSERT_TRUE(json.find("\"rawCan\":1") != std::string::npos);
	TEST_ASSERT_TRUE(json.find("\"chassisOnline\":1") != std::string::npos);
	TEST_ASSERT_TRUE(json.find("\"standby\":0") != std::string::npos);
	TEST_ASSERT_TRUE(json.find("\"uptime\":5000") != std::string::npos);
}

void test_wifi_json_includes_banshield_fields()
{
	State s = makeState();
	s.banShieldEnabled = true;
	s.banThreatLevel = 3;
	s.banDetectionCount = 9;

	std::string json = buildStateJson(s);
	TEST_ASSERT_TRUE(json.find("\"banShield\":1") != std::string::npos);
	TEST_ASSERT_TRUE(json.find("\"banThreat\":3") != std::string::npos);
	TEST_ASSERT_TRUE(json.find("\"banDetectCount\":9") != std::string::npos);
}


void test_wifi_cmd_valid_simple()
{
	TEST_ASSERT_TRUE(isValidCommand("ping"));
	TEST_ASSERT_TRUE(isValidCommand("fsd:on"));
	TEST_ASSERT_TRUE(isValidCommand("variant:hw4"));
	TEST_ASSERT_TRUE(isValidCommand("isa-chime:off"));
	TEST_ASSERT_TRUE(isValidCommand("profile:5"));
}

void test_wifi_cmd_rejects_empty()
{
	TEST_ASSERT_FALSE(isValidCommand(""));
	TEST_ASSERT_FALSE(isValidCommand(nullptr));
}

void test_wifi_cmd_rejects_too_long()
{
	char cmd[64];
	memset(cmd, 'a', 63);
	cmd[63] = '\0';
	TEST_ASSERT_FALSE(isValidCommand(cmd));
}

void test_wifi_cmd_rejects_spaces()
{
	TEST_ASSERT_FALSE(isValidCommand("fsd on"));
}

void test_wifi_cmd_rejects_special_chars()
{
	TEST_ASSERT_FALSE(isValidCommand("fsd;on"));
	TEST_ASSERT_FALSE(isValidCommand("test<script>"));
	TEST_ASSERT_FALSE(isValidCommand("cmd&rm"));
	TEST_ASSERT_FALSE(isValidCommand("a=b"));
}

void test_wifi_cmd_accepts_underscore()
{
	TEST_ASSERT_TRUE(isValidCommand("my_command"));
}

void test_wifi_cmd_accepts_uppercase()
{
	TEST_ASSERT_TRUE(isValidCommand("FSD:ON"));
}

void test_wifi_cmd_accepts_numbers()
{
	TEST_ASSERT_TRUE(isValidCommand("profile:123"));
}


void test_wifi_json_apgate_enabled_field()
{
	State s = makeState();
	s.apInjectionGateEnabled = true;
	std::string json = "{";
	json += "\"apGateEnabled\":" + std::to_string(s.apInjectionGateEnabled ? 1 : 0);
	json += ",\"apGateAp\":" + std::to_string(s.apGateApActive ? 1 : 0);
	json += ",\"apGatePark\":" + std::to_string(s.apGateParked ? 1 : 0);
	json += ",\"apGateSummon\":" + std::to_string(s.apGateSummoning ? 1 : 0);
	json += ",\"apGateOpen\":" + std::to_string(s.apGateOpen() ? 1 : 0);
	json += "}";
	TEST_ASSERT_TRUE(json.find("\"apGateEnabled\":1") != std::string::npos);
}

void test_wifi_json_apgate_disabled_field()
{
	State s = makeState();
	s.apInjectionGateEnabled = false;
	std::string json = "{";
	json += "\"apGateEnabled\":" + std::to_string(s.apInjectionGateEnabled ? 1 : 0);
	json += "}";
	TEST_ASSERT_TRUE(json.find("\"apGateEnabled\":0") != std::string::npos);
}

void test_wifi_json_apgate_open_when_disabled()
{
	State s = makeState();
	s.apInjectionGateEnabled = false;
	s.apGateApActive = false;
	s.apGateParked = false;
	s.apGateSummoning = false;
	TEST_ASSERT_TRUE(s.apGateOpen());
}

void test_wifi_json_apgate_closed_when_enabled_no_signal()
{
	State s = makeState();
	s.apInjectionGateEnabled = true;
	s.apGateApActive = false;
	s.apGateParked = false;
	s.apGateSummoning = false;
	TEST_ASSERT_FALSE(s.apGateOpen());
}

void test_wifi_json_apgate_open_when_enabled_and_parked()
{
	State s = makeState();
	s.apInjectionGateEnabled = true;
	s.apGateApActive = false;
	s.apGateParked = true;
	s.apGateSummoning = false;
	TEST_ASSERT_TRUE(s.apGateOpen());
}

void test_wifi_cmd_apgate_on_is_valid()
{
	TEST_ASSERT_TRUE(isValidCommand("apgate:on"));
}

void test_wifi_cmd_apgate_off_is_valid()
{
	TEST_ASSERT_TRUE(isValidCommand("apgate:off"));
}

void test_wifi_cmd_apgate_status_is_valid()
{
	TEST_ASSERT_TRUE(isValidCommand("apgate:status"));
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_wifi_json_contains_variant);
	RUN_TEST(test_wifi_json_contains_hw3_variant);
	RUN_TEST(test_wifi_json_fsd_enabled);
	RUN_TEST(test_wifi_json_fsd_disabled);
	RUN_TEST(test_wifi_json_contains_hardware_block);
	RUN_TEST(test_wifi_json_hw4_features);
	RUN_TEST(test_wifi_json_hw3_features);
	RUN_TEST(test_wifi_json_all_state_fields);
	RUN_TEST(test_wifi_json_includes_banshield_fields);

	RUN_TEST(test_wifi_cmd_valid_simple);
	RUN_TEST(test_wifi_cmd_rejects_empty);
	RUN_TEST(test_wifi_cmd_rejects_too_long);
	RUN_TEST(test_wifi_cmd_rejects_spaces);
	RUN_TEST(test_wifi_cmd_rejects_special_chars);
	RUN_TEST(test_wifi_cmd_accepts_underscore);
	RUN_TEST(test_wifi_cmd_accepts_uppercase);
	RUN_TEST(test_wifi_cmd_accepts_numbers);

	RUN_TEST(test_wifi_json_apgate_enabled_field);
	RUN_TEST(test_wifi_json_apgate_disabled_field);
	RUN_TEST(test_wifi_json_apgate_open_when_disabled);
	RUN_TEST(test_wifi_json_apgate_closed_when_enabled_no_signal);
	RUN_TEST(test_wifi_json_apgate_open_when_enabled_and_parked);
	RUN_TEST(test_wifi_cmd_apgate_on_is_valid);
	RUN_TEST(test_wifi_cmd_apgate_off_is_valid);
	RUN_TEST(test_wifi_cmd_apgate_status_is_valid);

	return UNITY_END();
}

