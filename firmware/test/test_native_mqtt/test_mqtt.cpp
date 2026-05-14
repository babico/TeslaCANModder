/** @file firmware/test/test_native_mqtt/test_mqtt.cpp
 *  @brief Unit tests for MQTT message formatting
 *  @author Tesla CAN Mod Contributors
 *  @license GPL-3.0
 */

#include <unity.h>
#include <cstring>

class __FlashStringHelper;
#define F(s) (reinterpret_cast<const __FlashStringHelper *>(s))

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

#include "core/types.h"

void saveSettings(const State &) {}
void sendLog(const char *) {}
void sendLog(const __FlashStringHelper *) {}

#include "feature/misc/mqtt_bridge.h"

void setUp() {}
void tearDown() {}


void test_mqtt_on()
{
	State s = {};
	bool ok = executeMqttCmd("mqtt:on", s);
	TEST_ASSERT_TRUE(ok);
	TEST_ASSERT_TRUE(s.mqttEnabled);
}

void test_mqtt_off()
{
	State s = {};
	s.mqttEnabled = true;
	bool ok = executeMqttCmd("mqtt:off", s);
	TEST_ASSERT_TRUE(ok);
	TEST_ASSERT_FALSE(s.mqttEnabled);
}

void test_mqtt_broker()
{
	State s = {};
	bool ok = executeMqttCmd("mqtt:broker:192.168.1.100", s);
	TEST_ASSERT_TRUE(ok);
	TEST_ASSERT_EQUAL_STRING("192.168.1.100", s.mqttHost);
}

void test_mqtt_port()
{
	State s = {};
	bool ok = executeMqttCmd("mqtt:port:8883", s);
	TEST_ASSERT_TRUE(ok);
	TEST_ASSERT_EQUAL(8883, s.mqttPort);
}

void test_mqtt_interval()
{
	State s = {};
	bool ok = executeMqttCmd("mqtt:interval:5000", s);
	TEST_ASSERT_TRUE(ok);
	TEST_ASSERT_EQUAL(5000, s.mqttInterval);
}

void test_mqtt_unrelated_returns_false()
{
	State s = {};
	bool ok = executeMqttCmd("fsd:on", s);
	TEST_ASSERT_FALSE(ok);
}

void test_mqtt_should_publish_interval()
{
	State s = {};
	s.mqttEnabled = true;
	s.mqttInterval = 2000;
	s.mqttLastPublishMs = 0;
	strcpy(s.mqttHost, "test.local");
	TEST_ASSERT_FALSE(mqttShouldPublish(s, 1000));
	TEST_ASSERT_TRUE(mqttShouldPublish(s, 2000));
	TEST_ASSERT_TRUE(mqttShouldPublish(s, 5000));
}

void test_mqtt_should_publish_disabled()
{
	State s = {};
	s.mqttEnabled = false;
	s.mqttInterval = 2000;
	s.mqttLastPublishMs = 0;
	TEST_ASSERT_FALSE(mqttShouldPublish(s, 5000));
}

void test_mqtt_broker_rejects_long_host()
{
	State s = {};
	char longHost[100] = "mqtt:broker:";
	for (int i = 12; i < 82; i++)
		longHost[i] = 'a';
	longHost[82] = '\0';
	bool ok = executeMqttCmd(longHost, s);
	TEST_ASSERT_FALSE(ok);
	TEST_ASSERT_EQUAL(0, strlen(s.mqttHost));
}

int main()
{
	UNITY_BEGIN();
	RUN_TEST(test_mqtt_on);
	RUN_TEST(test_mqtt_off);
	RUN_TEST(test_mqtt_broker);
	RUN_TEST(test_mqtt_port);
	RUN_TEST(test_mqtt_interval);
	RUN_TEST(test_mqtt_unrelated_returns_false);
	RUN_TEST(test_mqtt_should_publish_interval);
	RUN_TEST(test_mqtt_should_publish_disabled);
	RUN_TEST(test_mqtt_broker_rejects_long_host);
	return UNITY_END();
}

