/** @file firmware/test/test_native_mqtt_contract/test_mqtt_contract.cpp
 *  @brief Contract test: MQTT telemetry is not advertised while unimplemented (C1-13)
 *  @author Tesla CAN Mod Contributors
 *  @license GPL-3.0
 *
 *  The firmware has no MQTT publish loop and never sets mqttConnected. The wire
 *  schema and command reference must not claim telemetry publishing.
 */

#include <unity.h>
#include <fstream>
#include <sstream>
#include <string>

void setUp() {}
void tearDown() {}

static std::string readFirstExisting(const char *a, const char *b)
{
	std::ifstream f(a);
	if (!f.is_open())
		f.open(b);
	std::ostringstream ss;
	ss << f.rdbuf();
	return ss.str();
}

/** @brief io.schema.json must not advertise an MQTT bridge feature or commands. */
void test_schema_does_not_advertise_mqtt_bridge()
{
	std::string schema = readFirstExisting("lib/io/schemas/io.schema.json", "firmware/lib/io/schemas/io.schema.json");
	TEST_ASSERT_FALSE(schema.empty());
	TEST_ASSERT_TRUE(schema.find("mqttBridge") == std::string::npos);
	TEST_ASSERT_TRUE(schema.find("mqtt:on") == std::string::npos);
}

/** @brief Command reference must state that MQTT publishing is not implemented. */
void test_docs_flag_mqtt_as_not_implemented()
{
	std::string docs = readFirstExisting("../docs/reference/commands.md", "docs/reference/commands.md");
	TEST_ASSERT_FALSE(docs.empty());
	TEST_ASSERT_TRUE(docs.find("not implemented") != std::string::npos);
	TEST_ASSERT_TRUE(docs.find("no publish loop") != std::string::npos);
}

int main()
{
	UNITY_BEGIN();
	RUN_TEST(test_schema_does_not_advertise_mqtt_bridge);
	RUN_TEST(test_docs_flag_mqtt_as_not_implemented);
	return UNITY_END();
}
