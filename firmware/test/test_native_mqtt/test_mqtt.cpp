// ── MQTT Bridge Tests ───────────────────────────────────────────────────────
// Tests MQTT command parsing and interval publishing logic.

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

// ── Stubs ────────────────────────────────────────────────────────────────────
void saveSettings(const State &) {}
void sendLog(const char *) {}
void sendLog(const __FlashStringHelper *) {}

#include "feature/mqtt_bridge.h"

void setUp() {}
void tearDown() {}

// ── Tests ───────────────────────────────────────────────────────────────────

void test_mqtt_on()
{
	State s = {};
	bool ok = execMqttCmd("mqtt:on", s);
	TEST_ASSERT_TRUE(ok);
	TEST_ASSERT_TRUE(s.mqttEnabled);
}

void test_mqtt_off()
{
	State s = {};
	s.mqttEnabled = true;
	bool ok = execMqttCmd("mqtt:off", s);
	TEST_ASSERT_TRUE(ok);
	TEST_ASSERT_FALSE(s.mqttEnabled);
}

void test_mqtt_broker()
{
	State s = {};
	bool ok = execMqttCmd("mqtt:broker:192.168.1.100", s);
	TEST_ASSERT_TRUE(ok);
	TEST_ASSERT_EQUAL_STRING("192.168.1.100", s.mqttHost);
}

void test_mqtt_port()
{
	State s = {};
	bool ok = execMqttCmd("mqtt:port:8883", s);
	TEST_ASSERT_TRUE(ok);
	TEST_ASSERT_EQUAL(8883, s.mqttPort);
}

void test_mqtt_interval()
{
	State s = {};
	bool ok = execMqttCmd("mqtt:interval:5000", s);
	TEST_ASSERT_TRUE(ok);
	TEST_ASSERT_EQUAL(5000, s.mqttInterval);
}

void test_mqtt_unrelated_returns_false()
{
	State s = {};
	bool ok = execMqttCmd("fsd:on", s);
	TEST_ASSERT_FALSE(ok);
}

void test_mqtt_should_publish_interval()
{
	State s = {};
	s.mqttEnabled = true;
	s.mqttInterval = 2000;
	s.mqttLastPublishMs = 0;
	strcpy(s.mqttHost, "test.local");
	// Before interval
	TEST_ASSERT_FALSE(mqttShouldPublish(s, 1000));
	// At interval
	TEST_ASSERT_TRUE(mqttShouldPublish(s, 2000));
	// After interval
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
	// 70-char host exceeds MQTT_HOST_MAX (63) — should be rejected
	char longHost[100] = "mqtt:broker:";
	for (int i = 12; i < 82; i++)
		longHost[i] = 'a';
	longHost[82] = '\0';
	bool ok = execMqttCmd(longHost, s);
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
