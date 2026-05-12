#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/mqtt_bridge.h
 * @brief MQTT telemetry bridge for publishing decoded CAN data to an MQTT broker
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/types.h"
#include "core/util/parse.h"

#define MQTT_DEFAULT_PORT 1883
#define MQTT_DEFAULT_INTERVAL 2000
#define MQTT_MIN_INTERVAL 500
#define MQTT_MAX_INTERVAL 60000
#define MQTT_HOST_MAX 63
#define MQTT_TOPIC_PREFIX "teslacanmodder/"

/**
 * @brief Execute an MQTT bridge command (enable/disable, set broker, port, interval).
 * @param cmd Raw command string starting with "mqtt:".
 * @param s Mutable reference to the global state.
 * @return True if the command was recognized and applied successfully.
 */
static bool executeMqttCmd(const char *cmd, State &s)
{
	if (strncmp(cmd, "mqtt:", 5) == 0 && parseBoolCmd(cmd + 5, s.mqttEnabled, s.mqttEnabled))
	{
		return true;
	}
	if (strncmp(cmd, "mqtt:broker:", 12) == 0)
	{
		const char *host = cmd + 12;
		size_t len = strlen(host);
		if (len == 0 || len > MQTT_HOST_MAX)
			return false;
		// Validate hostname: allow alphanumeric, dash, dot, colon (IPv6)
		for (size_t i = 0; i < len; i++)
		{
			char c = host[i];
			if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '.' ||
				  c == ':'))
			{
				return false;
			}
		}
		strncpy(s.mqttHost, host, MQTT_HOST_MAX);
		s.mqttHost[MQTT_HOST_MAX] = '\0';
		return true;
	}
	if (strncmp(cmd, "mqtt:port:", 10) == 0)
	{
		int port = atoi(cmd + 10);
		if (port > 0 && port <= 65535)
		{
			s.mqttPort = (uint16_t)port;
			return true;
		}
		return false;
	}
	if (strncmp(cmd, "mqtt:interval:", 14) == 0)
	{
		int interval = atoi(cmd + 14);
		if (interval >= MQTT_MIN_INTERVAL && interval <= MQTT_MAX_INTERVAL)
		{
			s.mqttInterval = (uint16_t)interval;
			return true;
		}
		return false;
	}
	return false;
}

/**
 * @brief Determine whether the MQTT bridge should publish data this tick.
 * @param s Const reference to the global state.
 * @param now Current timestamp in milliseconds.
 * @return True if the publish interval has elapsed and the bridge is configured.
 */
inline bool mqttShouldPublish(const State &s, unsigned long now)
{
	if (!s.mqttEnabled)
		return false;
	if (s.mqttHost[0] == '\0')
		return false;
	return (now - s.mqttLastPublishMs) >= s.mqttInterval;
}
