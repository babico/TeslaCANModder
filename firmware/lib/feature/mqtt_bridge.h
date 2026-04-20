#pragma once
#include "core/types.h"
#include "infra/parse.h"

// ── 3.6 MQTT Telemetry Bridge ───────────────────────────────────────────────
// Publishes decoded CAN telemetry data to an MQTT broker for Home Assistant,
// Node-RED, or other IoT integrations.
//
// Topics published (prefix "tesla/"):
//   tesla/bms/voltage, tesla/bms/soc, tesla/bms/power, tesla/bms/temp
//   tesla/speed, tesla/gear, tesla/pedal, tesla/steering
//   tesla/tpms/fl, tesla/tpms/fr, tesla/tpms/rl, tesla/tpms/rr
//   tesla/status/fsd, tesla/status/variant, tesla/status/uptime
//
// Commands:
//   mqtt:on / mqtt:off         — enable/disable bridge
//   mqtt:broker:<host>         — set broker hostname (max 63 chars)
//   mqtt:port:<port>           — set broker port (1-65535)
//   mqtt:interval:<ms>         — publish interval (500-60000 ms)
//
// NVS keys: "mqttE", "mqttH" (host string), "mqttP" (port), "mqttI" (interval)

#define MQTT_DEFAULT_PORT       1883
#define MQTT_DEFAULT_INTERVAL   2000
#define MQTT_MIN_INTERVAL       500
#define MQTT_MAX_INTERVAL       60000
#define MQTT_HOST_MAX           63
#define MQTT_TOPIC_PREFIX       "tesla/"

inline bool execMqttCmd(const char* cmd, State& s) {
  if (strncmp(cmd, "mqtt:", 5) == 0 && parseBoolCmd(cmd + 5, s.mqttEnabled, s.mqttEnabled)) {
    return true;
  }
  if (strncmp(cmd, "mqtt:broker:", 12) == 0) {
    const char* host = cmd + 12;
    size_t len = strlen(host);
    if (len == 0 || len > MQTT_HOST_MAX) return false;
    // Validate hostname characters (alphanumeric, dash, dot, colon for IPv6)
    for (size_t i = 0; i < len; i++) {
      char c = host[i];
      if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '.' || c == ':')) {
        return false;
      }
    }
    strncpy(s.mqttHost, host, MQTT_HOST_MAX);
    s.mqttHost[MQTT_HOST_MAX] = '\0';
    return true;
  }
  if (strncmp(cmd, "mqtt:port:", 10) == 0) {
    int port = atoi(cmd + 10);
    if (port > 0 && port <= 65535) {
      s.mqttPort = (uint16_t)port;
      return true;
    }
    return false;
  }
  if (strncmp(cmd, "mqtt:interval:", 14) == 0) {
    int interval = atoi(cmd + 14);
    if (interval >= MQTT_MIN_INTERVAL && interval <= MQTT_MAX_INTERVAL) {
      s.mqttInterval = (uint16_t)interval;
      return true;
    }
    return false;
  }
  return false;
}

// Publish tick — called from main loop when MQTT is enabled
// Returns true if data was published this tick
inline bool mqttShouldPublish(const State& s, unsigned long now) {
  if (!s.mqttEnabled) return false;
  if (s.mqttHost[0] == '\0') return false;
  return (now - s.mqttLastPublishMs) >= s.mqttInterval;
}
