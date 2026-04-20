#pragma once
#include <Arduino.h>
#include "core/config/esp32.h"
#include "core/types.h"
#include "infra/can.h"

// Forward declarations for logging (used by handlers included below)
void sendLog(const char* msg);
void sendLog(const __FlashStringHelper* msg);
bool driverReinit();
void driverSetClockMHz(uint8_t mhz);
uint8_t driverGetClockReqMHz();
uint8_t driverGetClockMHz();

#include "infra/parse.h"
#include "feature/stream.h"
#include "feature/can_raw.h"
#include "feature/can_clock.h"
#include "feature/fsd.h"
#include "feature/nag.h"
#include "feature/ban_shield.h"
#include "feature/tlssc.h"
#include "feature/profile.h"
#include "feature/offsets.h"
#include "feature/isa_chime.h"
#include "feature/summon.h"
#include "feature/variant.h"
#include "feature/bms.h"
#include "feature/tpms.h"
#include "feature/drive_mode.h"
#include "feature/region.h"
#include "infra/log_ring.h"
#if BUS_VEHICLE_ACTIVE
  #include "feature/mirror.h"
  #include "feature/lock.h"
  #include "feature/light.h"
  #include "feature/wiper.h"
  #include "feature/seat.h"
  #include "feature/display.h"
  #include "feature/power.h"
  #include "feature/climate.h"
  #include "feature/charge.h"
  #include "feature/pedal.h"
  #include "feature/regen.h"
  #include "feature/stop.h"
  #include "feature/precondition.h"
  #include "feature/track_mode.h"
  #include "feature/turn_signal.h"
  #include "feature/seatbelt.h"
  #include "feature/air_recirc.h"
  #include "feature/wiper.h"
  #include "feature/mirror.h"
  #include "feature/powertrain.h"
  #include "feature/can_sim.h"
  #include "feature/single_shot.h"
  #include "feature/mqtt_bridge.h"
  #include "feature/fw_compat.h"
  #include "feature/vehicle_config.h"
#endif
#if BUS_BODY_ACTIVE
  #include "feature/window.h"
  #include "feature/sentry.h"
#endif
#if BUS_VEHICLE_ACTIVE || BUS_BODY_ACTIVE
  #include "feature/trunk.h"
#endif

#if BOARD_ENABLE_BLE
  #include "io/ble/esp32.h"
#endif

static const uint8_t SERIAL_CMD_BUFFER_SIZE = 32;
static char usbBuf[SERIAL_CMD_BUFFER_SIZE];
static uint8_t usbLen = 0;
#if BOARD_ENABLE_BLE
static char bleBuf[SERIAL_CMD_BUFFER_SIZE];
static uint8_t bleLen = 0;
#endif
static unsigned long lastStatusMs = 0;
static bool statusLiveEnabled = false;
static const unsigned long STATUS_LIVE_INTERVAL_MS = 250;

// ── Output Helpers ───────────────────────────────────────────────────────────
void printStr(const char* s) {
  Serial.print(s);
#if BOARD_ENABLE_BLE
  blePrint(s);
#endif
}

void printStr(const __FlashStringHelper* s) {
  Serial.print(s);
#if BOARD_ENABLE_BLE
  blePrint(s);
#endif
}

void printNum(long n) {
  Serial.print(n);
#if BOARD_ENABLE_BLE
  blePrintNum(n);
#endif
}

void printHex(uint8_t b) {
  if (b < 0x10) printStr("0");
  Serial.print(b, HEX);
#if BOARD_ENABLE_BLE
  blePrintHex(b);
#endif
}

void printLn() {
  Serial.println();
#if BOARD_ENABLE_BLE
  blePrintLn();
#endif
}

#include "io/serial/common.h"

// ── JSON Messages ────────────────────────────────────────────────────────────
void sendBoot(State& s) {
  Features f = getFeatures(s.variant);
  extern bool mcpAvailable[];

  jsonLine()
    .str("t", "boot")
    .object("meta", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.str("hw", BOARD_HW_NAME)
       .str("can", BOARD_CAN_NAME)
       .str("drv", BOARD_DRIVER_NAME)
       .str("variant", variantName(s.variant));
#if BOARD_ENABLE_WIFI && BOARD_ENABLE_BLE
      o.str("cap", "usb+wifi+ble");
#elif BOARD_ENABLE_WIFI
      o.str("cap", "usb+wifi");
#elif BOARD_ENABLE_BLE
      o.str("cap", "usb+ble");
#else
      o.str("cap", "usb");
#endif
      o.str("ready", "runtime-ready");
    })
    .object("connectivity", [&](JsonLineBuilder::JsonObjectBuilder& o) {
#if BOARD_ENABLE_BLE
      o.boolean("bleEnabled", true);
#else
      o.boolean("bleEnabled", false);
#endif
#if BOARD_ENABLE_WIFI
      o.boolean("wifiEnabled", true);
#else
      o.boolean("wifiEnabled", false);
#endif
      o.object("bus", [&](JsonLineBuilder::JsonObjectBuilder& bus) {
        bus.boolean("chassis", BUS_CHASSIS_ACTIVE)
           .boolean("vehicle", BUS_VEHICLE_ACTIVE)
           .boolean("body", BUS_BODY_ACTIVE);
      })
#if BUS_VEHICLE_ACTIVE
       .boolean("vehicleOnline", mcpAvailable[1])
#else
       .boolean("vehicleOnline", false)
#endif
#if BUS_BODY_ACTIVE
       .boolean("bodyOnline", mcpAvailable[2])
#else
       .boolean("bodyOnline", false)
#endif
       .boolean("chassisOnline", s.chassisOnline)
       .boolean("standby", s.standby);
#if BOARD_ENABLE_BLE
      o.str("bleName", BLE_DEVICE_NAME);
#endif
#if BOARD_ENABLE_WIFI
      o.object("wifi", [&](JsonLineBuilder::JsonObjectBuilder& wifi) {
        wifi.str("ssid", WIFI_AP_SSID).num("port", WIFI_REST_PORT);
      });
#endif
    })
    .object("state", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.boolean("fsd", s.fsdEnabled)
       .boolean("fsdForce", s.fsdForceEnabled)
       .boolean("nag", s.nagSuppress)
       .boolean("isaChime", s.isaChimeSuppress)
       .boolean("summonInject", s.summonInject)
       .boolean("nagKiller", s.nagKillerEnabled)
       .str("nagKillerMode", nagKillerModeName(s.nagKillerMode))
       .num("dasHandsOn", s.dasHandsOnState)
       .object("profile", [&](JsonLineBuilder::JsonObjectBuilder& p) {
         p.num("value", s.speedProfile).boolean("pinned", s.profileOverride);
       })
       .object("offset", [&](JsonLineBuilder::JsonObjectBuilder& off) {
         off.num("value", s.speedOffset).boolean("pinned", s.offsetOverride);
       })
       .boolean("precondition", s.preconditionEnabled)
       .boolean("trackMode", s.trackModeEnabled)
       .boolean("otaInProgress", s.otaInProgress)
       .boolean("txPaused", s.txPaused)
       .num("detectedHW", s.detectedHW)
       .boolean("variantAutoDetect", s.variantAutoDetect)
       .num("gtwAutopilotTier", (int)s.gtwAutopilotTier)
       .boolean("rawCan", s.rawCanListen)
       .object("stream", [&](JsonLineBuilder::JsonObjectBuilder& stream) {
         stream.boolean("on", false).num("emitted", 0);
       });
    })
    .object("driverAssist", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.num("turnSignalLeft", s.turnSignalLeft ? 1 : 0)
       .num("turnSignalRight", s.turnSignalRight ? 1 : 0)
       .num("bsmLeftLevel", s.bsmLeftLevel)
       .num("bsmRightLevel", s.bsmRightLevel)
       .num("cruiseSetSpeed", (long)(s.cruiseSetSpeedKph * 10))
       .num("accSpeedLimit", (long)(s.accSpeedLimitKph * 10))
       .num("mapSpeedLimit", (long)(s.mapSpeedLimitKph * 10))
       .num("maxSpeed", (long)(s.maxSpeedKph * 10))
       .num("driveMode", s.driveModeOverride)
       .num("currentDriveMode", s.currentDriveMode)
       .boolean("eceR79", s.eceR79Bypass)
       .boolean("seatbeltEmulation", s.seatbeltEmulation)
       .boolean("wiperPersist", s.wiperPersistEnabled)
       .boolean("mirrorAutoFold", s.mirrorAutoFoldEnabled)
       .boolean("canSim", s.canSimEnabled)
       .boolean("singleShot", s.singleShotTx)
       .num("steeringMode", s.steeringMode)
       .boolean("hasSteeringMode", s.hasSteeringMode);
    })
    .object("vehicle", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.boolean("doorFrontLeftOpen", s.doorFrontLeftOpen)
       .boolean("doorFrontRightOpen", s.doorFrontRightOpen)
       .boolean("doorRearLeftOpen", s.doorRearLeftOpen)
       .boolean("doorRearRightOpen", s.doorRearRightOpen)
       .boolean("driverDoorOpen", s.driverDoorOpen)
       .boolean("anyDoorOpen", s.anyDoorOpen)
       .boolean("frunkOpen", s.frunkOpen)
       .boolean("trunkOpen", s.trunkOpen)
       .num("regionCode", s.regionCode)
       .boolean("hasRegion", s.hasRegion)
       .boolean("cnLocked", s.chineseGatewayLocked)
       .boolean("rateLimit", s.rateLimitEnabled)
       .boolean("hasTpms", s.hasTpms)
       .boolean("hasPowertrain", s.hasPowertrain)
       .num("vehicleModel", s.vehicleModel)
       .num("vehicleYear", s.vehicleYear)
       .boolean("hasVehicleConfig", s.hasVehicleConfig);
    })
    .object("platform", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.num("model", s.platformModel)
       .num("hwGen", s.platformHwGen)
       .num("swYear", s.platformSwYear)
       .num("swWeek", s.platformSwWeek)
       .num("swRelease", s.platformSwRelease)
       .num("fsdProto", s.platformFsdProto)
       .num("swCompat", s.platformSwCompat)
       .boolean("resolved", s.platformResolved);
    })
    .object("firmware", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.num("year", s.fwYear)
       .num("release", s.fwRelease)
       .num("minor", s.fwMinor)
       .num("compat", s.fwCompat)
       .boolean("hasVersion", s.hasFwVersion)
       .boolean("mqtt", s.mqttEnabled)
       .boolean("mqttConnected", s.mqttConnected);
    })
    .object("battery", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.num("nomFullPack", (long)(s.bmsNominalFullPack * 100))
       .num("nomRemain", (long)(s.bmsNominalRemaining * 100))
       .num("idealRemain", (long)(s.bmsIdealRemaining * 100))
       .num("cellVMax", (long)(s.bmsCellVoltageMax * 1000))
       .num("cellVMin", (long)(s.bmsCellVoltageMin * 1000))
       .num("maxRegen", (long)(s.bmsMaxRegenPower * 100))
       .num("maxDischarge", (long)(s.bmsMaxDischargePower * 100))
       .boolean("hasEnhanced", s.hasEnhancedBms);
    })
    .object("safety", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.boolean("banShield", s.banShieldEnabled)
       .num("banThreat", s.banThreatLevel)
       .num("banDetectCount", s.banDetectionCount)
       .boolean("gtwShieldArmed", s.gtwShieldArmed)
       .num("gtwShieldBlocks", (long)s.gtwShieldBlocks);
    })
    .object("can", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.num("clockReqMHz", s.canClockReqMHz)
       .num("clockMHz", s.canClockMHz)
       .object("health", [&](JsonLineBuilder::JsonObjectBuilder& health) {
         for (uint8_t i = 0; i < BUS_MAX; i++) {
           char key[8];
           key[0] = 'b';
           key[1] = 'u';
           key[2] = 's';
           key[3] = (char)('0' + i);
           key[4] = '\0';
           health.object(key, [&](JsonLineBuilder::JsonObjectBuilder& bus) {
             bus.boolean("on", busActive(i)).boolean("det", mcpAvailable[i]);
           });
         }
       });
    })
    .object("features", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.boolean("fsd", f.fsd)
       .boolean("fsdForce", f.fsdForce)
       .boolean("profile", f.profile)
       .boolean("nag", f.nag)
       .boolean("offset", f.offset)
       .boolean("isaSpeedChime", f.isaChime)
       .boolean("summon", f.summon)
       .boolean("eap", s.enhancedAutopilot)
       .boolean("evd", s.evdEnabled)
       .boolean("tlssc", s.tlsscRestore);
    })
    .end();
}

void sendStatus(State& s, unsigned long now) {
  Features f = getFeatures(s.variant);
  extern bool mcpAvailable[];

  jsonLine()
    .str("t", "status")
    .object("meta", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.str("hw", BOARD_HW_NAME)
       .str("can", BOARD_CAN_NAME)
       .str("drv", BOARD_DRIVER_NAME)
       .str("variant", variantName(s.variant));
#if BOARD_ENABLE_WIFI && BOARD_ENABLE_BLE
      o.str("cap", "usb+wifi+ble");
#elif BOARD_ENABLE_WIFI
      o.str("cap", "usb+wifi");
#elif BOARD_ENABLE_BLE
      o.str("cap", "usb+ble");
#else
      o.str("cap", "usb");
#endif
      o.str("ready", "runtime-ready").num("up", now);
    })
    .object("connectivity", [&](JsonLineBuilder::JsonObjectBuilder& o) {
#if BOARD_ENABLE_BLE
      o.boolean("bleEnabled", true);
#else
      o.boolean("bleEnabled", false);
#endif
#if BOARD_ENABLE_WIFI
      o.boolean("wifiEnabled", true);
#else
      o.boolean("wifiEnabled", false);
#endif
      o.object("bus", [&](JsonLineBuilder::JsonObjectBuilder& bus) {
        bus.boolean("chassis", BUS_CHASSIS_ACTIVE)
           .boolean("vehicle", BUS_VEHICLE_ACTIVE)
           .boolean("body", BUS_BODY_ACTIVE);
      })
#if BUS_VEHICLE_ACTIVE
       .boolean("vehicleOnline", mcpAvailable[1])
#else
       .boolean("vehicleOnline", false)
#endif
#if BUS_BODY_ACTIVE
       .boolean("bodyOnline", mcpAvailable[2])
#else
       .boolean("bodyOnline", false)
#endif
       .boolean("chassisOnline", s.chassisOnline)
       .boolean("standby", s.standby);
    })
    .object("state", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.boolean("fsd", s.fsdEnabled)
       .boolean("fsdForce", s.fsdForceEnabled)
       .boolean("nag", s.nagSuppress)
       .boolean("isaChime", s.isaChimeSuppress)
       .boolean("summonInject", s.summonInject)
       .boolean("nagKiller", s.nagKillerEnabled)
       .str("nagKillerMode", nagKillerModeName(s.nagKillerMode))
       .num("dasHandsOn", s.dasHandsOnState)
       .object("profile", [&](JsonLineBuilder::JsonObjectBuilder& p) {
         p.num("value", s.speedProfile).boolean("pinned", s.profileOverride);
       })
       .object("offset", [&](JsonLineBuilder::JsonObjectBuilder& off) {
         off.num("value", s.speedOffset).boolean("pinned", s.offsetOverride);
       })
       .boolean("precondition", s.preconditionEnabled)
       .boolean("trackMode", s.trackModeEnabled)
       .boolean("otaInProgress", s.otaInProgress)
       .boolean("txPaused", s.txPaused)
       .num("detectedHW", s.detectedHW)
       .boolean("variantAutoDetect", s.variantAutoDetect)
       .num("gtwAutopilotTier", (int)s.gtwAutopilotTier)
       .boolean("rawCan", s.rawCanListen)
       .object("stream", [&](JsonLineBuilder::JsonObjectBuilder& stream) {
         stream.boolean("on", s.streamEnabled).num("emitted", s.streamCount);
       });
    })
    .object("driverAssist", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.num("turnSignalLeft", s.turnSignalLeft ? 1 : 0)
       .num("turnSignalRight", s.turnSignalRight ? 1 : 0)
       .num("bsmLeftLevel", s.bsmLeftLevel)
       .num("bsmRightLevel", s.bsmRightLevel)
       .num("cruiseSetSpeed", (long)(s.cruiseSetSpeedKph * 10))
       .num("accSpeedLimit", (long)(s.accSpeedLimitKph * 10))
       .num("mapSpeedLimit", (long)(s.mapSpeedLimitKph * 10))
       .num("maxSpeed", (long)(s.maxSpeedKph * 10))
       .num("driveMode", s.driveModeOverride)
       .num("currentDriveMode", s.currentDriveMode)
       .boolean("eceR79", s.eceR79Bypass)
       .boolean("seatbeltEmulation", s.seatbeltEmulation)
       .boolean("wiperPersist", s.wiperPersistEnabled)
       .boolean("mirrorAutoFold", s.mirrorAutoFoldEnabled)
       .boolean("canSim", s.canSimEnabled)
       .boolean("singleShot", s.singleShotTx)
       .num("steeringMode", s.steeringMode)
       .boolean("hasSteeringMode", s.hasSteeringMode);
    })
    .object("vehicle", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.boolean("doorFrontLeftOpen", s.doorFrontLeftOpen)
       .boolean("doorFrontRightOpen", s.doorFrontRightOpen)
       .boolean("doorRearLeftOpen", s.doorRearLeftOpen)
       .boolean("doorRearRightOpen", s.doorRearRightOpen)
       .boolean("driverDoorOpen", s.driverDoorOpen)
       .boolean("anyDoorOpen", s.anyDoorOpen)
       .boolean("frunkOpen", s.frunkOpen)
       .boolean("trunkOpen", s.trunkOpen)
       .num("regionCode", s.regionCode)
       .boolean("hasRegion", s.hasRegion)
       .boolean("cnLocked", s.chineseGatewayLocked)
       .boolean("rateLimit", s.rateLimitEnabled)
       .boolean("hasTpms", s.hasTpms)
       .boolean("hasPowertrain", s.hasPowertrain)
       .num("vehicleModel", s.vehicleModel)
       .num("vehicleYear", s.vehicleYear)
       .boolean("hasVehicleConfig", s.hasVehicleConfig);
    })
    .object("platform", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.num("model", s.platformModel)
       .num("hwGen", s.platformHwGen)
       .num("swYear", s.platformSwYear)
       .num("swWeek", s.platformSwWeek)
       .num("swRelease", s.platformSwRelease)
       .num("fsdProto", s.platformFsdProto)
       .num("swCompat", s.platformSwCompat)
       .boolean("resolved", s.platformResolved);
    })
    .object("firmware", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.num("year", s.fwYear)
       .num("release", s.fwRelease)
       .num("minor", s.fwMinor)
       .num("compat", s.fwCompat)
       .boolean("hasVersion", s.hasFwVersion)
       .boolean("mqtt", s.mqttEnabled)
       .boolean("mqttConnected", s.mqttConnected);
    })
    .object("battery", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.num("nomFullPack", (long)(s.bmsNominalFullPack * 100))
       .num("nomRemain", (long)(s.bmsNominalRemaining * 100))
       .num("idealRemain", (long)(s.bmsIdealRemaining * 100))
       .num("cellVMax", (long)(s.bmsCellVoltageMax * 1000))
       .num("cellVMin", (long)(s.bmsCellVoltageMin * 1000))
       .num("maxRegen", (long)(s.bmsMaxRegenPower * 100))
       .num("maxDischarge", (long)(s.bmsMaxDischargePower * 100))
       .boolean("hasEnhanced", s.hasEnhancedBms);
    })
    .object("safety", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.boolean("banShield", s.banShieldEnabled)
       .num("banThreat", s.banThreatLevel)
       .num("banDetectCount", s.banDetectionCount)
       .boolean("gtwShieldArmed", s.gtwShieldArmed)
       .num("gtwShieldBlocks", (long)s.gtwShieldBlocks);
    })
    .object("can", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.num("clockReqMHz", s.canClockReqMHz)
       .num("clockMHz", s.canClockMHz)
       .object("health", [&](JsonLineBuilder::JsonObjectBuilder& health) {
         for (uint8_t i = 0; i < BUS_MAX; i++) {
           char key[8];
           key[0] = 'b';
           key[1] = 'u';
           key[2] = 's';
           key[3] = (char)('0' + i);
           key[4] = '\0';
           health.object(key, [&](JsonLineBuilder::JsonObjectBuilder& bus) {
             bus.boolean("on", busActive(i)).boolean("det", mcpAvailable[i]);
           });
         }
       });
    })
    .object("features", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.boolean("fsd", f.fsd)
       .boolean("fsdForce", f.fsdForce)
       .boolean("profile", f.profile)
       .boolean("nag", f.nag)
       .boolean("offset", f.offset)
       .boolean("isaSpeedChime", f.isaChime)
       .boolean("summon", f.summon)
       .boolean("eap", s.enhancedAutopilot)
       .boolean("evd", s.evdEnabled)
       .boolean("tlssc", s.tlsscRestore);
    })
    .end();
}

void sendStatusMeta(State& s, unsigned long now) {
  JsonLineBuilder line = jsonLine();
  line.str("t", "status_meta")
      .str("hw", BOARD_HW_NAME)
      .str("can", BOARD_CAN_NAME)
      .str("drv", BOARD_DRIVER_NAME)
      .str("variant", variantName(s.variant));
#if BOARD_ENABLE_WIFI && BOARD_ENABLE_BLE
  line.str("cap", "usb+wifi+ble");
#elif BOARD_ENABLE_WIFI
  line.str("cap", "usb+wifi");
#elif BOARD_ENABLE_BLE
  line.str("cap", "usb+ble");
#else
  line.str("cap", "usb");
#endif
  line.str("ready", "runtime-ready").num("up", now).end();
}

void sendStatusFeatures(State& s) {
  Features f = getFeatures(s.variant);
  jsonLine()
    .str("t", "status_features")
    .object("features", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.boolean("fsd", f.fsd)
       .boolean("fsdForce", f.fsdForce)
       .boolean("profile", f.profile)
       .boolean("nag", f.nag)
       .boolean("offset", f.offset)
       .boolean("isaSpeedChime", f.isaChime)
       .boolean("summon", f.summon);
    })
    .end();
}

void sendStatusCan(State& s) {
  extern bool mcpAvailable[];
  jsonLine()
    .str("t", "status_can")
    .object("clock", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.num("reqMHz", s.canClockReqMHz)
       .num("activeMHz", s.canClockMHz);
    })
    .object("health", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      for (uint8_t i = 0; i < BUS_MAX; i++) {
        char key[8];
        key[0] = 'b';
        key[1] = 'u';
        key[2] = 's';
        key[3] = (char)('0' + i);
        key[4] = '\0';
        o.object(key, [&](JsonLineBuilder::JsonObjectBuilder& bus) {
          bus.boolean("on", busActive(i))
             .boolean("det", mcpAvailable[i]);
        });
      }
    })
    .end();
}

void sendStatusState(State& s) {
  jsonLine()
    .str("t", "status_state")
    .object("state", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.boolean("fsd", s.fsdEnabled)
       .boolean("fsdForce", s.fsdForceEnabled)
       .boolean("nag", s.nagSuppress)
       .boolean("nagKiller", s.nagKillerEnabled)
       .object("profile", [&](JsonLineBuilder::JsonObjectBuilder& p) {
         p.num("value", s.speedProfile).boolean("pinned", s.profileOverride);
       })
       .object("offset", [&](JsonLineBuilder::JsonObjectBuilder& off) {
         off.num("value", s.speedOffset).boolean("pinned", s.offsetOverride);
       })
       .boolean("precondition", s.preconditionEnabled)
       .boolean("trackMode", s.trackModeEnabled)
       .boolean("otaInProgress", s.otaInProgress)
       .boolean("txPaused", s.txPaused);
    })
    .end();
}

void sendStatusCompact(State& s, unsigned long now) {
  extern bool mcpAvailable[];
  Features f = getFeatures(s.variant);
  jsonLine()
    .str("t", "status_compact")
    .object("meta", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.str("hw", BOARD_HW_NAME)
       .str("variant", variantName(s.variant))
       .str("ready", "runtime-ready")
       .num("up", now);
    })
    .object("connectivity", [&](JsonLineBuilder::JsonObjectBuilder& o) {
#if BOARD_ENABLE_BLE
      o.boolean("bt", true);
#else
      o.boolean("bt", false);
#endif
#if BOARD_ENABLE_WIFI
      o.boolean("wifi", true);
#else
      o.boolean("wifi", false);
#endif
      o.boolean("chassisOnline", s.chassisOnline)
#if BUS_VEHICLE_ACTIVE
       .boolean("vehicleOnline", mcpAvailable[1])
#else
       .boolean("vehicleOnline", false)
#endif
#if BUS_BODY_ACTIVE
       .boolean("bodyOnline", mcpAvailable[2])
#else
       .boolean("bodyOnline", false)
#endif
       .boolean("standby", s.standby);
    })
    .object("state", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.boolean("fsd", s.fsdEnabled)
       .boolean("fsdForce", s.fsdForceEnabled)
       .boolean("nag", s.nagSuppress)
       .num("profile", s.speedProfile)
       .num("offset", s.speedOffset)
       .boolean("precondition", s.preconditionEnabled)
       .boolean("trackMode", s.trackModeEnabled);
    })
    .object("features", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.boolean("fsd", f.fsd)
       .boolean("fsdForce", f.fsdForce)
       .boolean("profile", f.profile)
       .boolean("nag", f.nag)
       .boolean("offset", f.offset)
       .boolean("isaSpeedChime", f.isaChime)
       .boolean("summon", f.summon);
    })
    .object("can", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.object("clock", [&](JsonLineBuilder::JsonObjectBuilder& clock) {
        clock.num("reqMHz", s.canClockReqMHz)
             .num("activeMHz", s.canClockMHz);
      });

      o.object("health", [&](JsonLineBuilder::JsonObjectBuilder& health) {
        for (uint8_t i = 0; i < BUS_MAX; i++) {
          char key[8];
          key[0] = 'b';
          key[1] = 'u';
          key[2] = 's';
          key[3] = (char)('0' + i);
          key[4] = '\0';
          health.object(key, [&](JsonLineBuilder::JsonObjectBuilder& bus) {
            bus.boolean("on", busActive(i))
               .boolean("det", mcpAvailable[i]);
          });
        }
      });
    })
    .object("stream", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.boolean("on", s.streamEnabled)
       .num("emitted", s.streamCount);
    })
    .boolean("rawCan", s.rawCanListen)
    .end();
}

// ── Command Parser ───────────────────────────────────────────────────────────
void executeCommand(const char* cmd, State& s, unsigned long now) {
  if (strcmp(cmd, "ping") == 0) {
    jsonLine().str("t", "pong").num("v", 1).end();
    return;
  }

  if (strcmp(cmd, "status:live:on") == 0) {
    statusLiveEnabled = true;
    sendAck(cmd);
    sendLog(F("Status live stream ON"));
    sendStatus(s, now);
    return;
  }

  if (strcmp(cmd, "status:live:off") == 0) {
    statusLiveEnabled = false;
    sendAck(cmd);
    sendLog(F("Status live stream OFF"));
    sendStatus(s, now);
    return;
  }

  if (strcmp(cmd, "status:live") == 0) {
    jsonLine()
      .str("t", "statusLive")
      .boolean("on", statusLiveEnabled)
      .num("intervalMs", (unsigned long)STATUS_LIVE_INTERVAL_MS)
      .end();
    return;
  }

  if (strcmp(cmd, "status:compact") == 0) {
    sendStatusCompact(s, now);
    return;
  }

  if (strcmp(cmd, "status:meta") == 0) {
    sendStatusMeta(s, now);
    return;
  }

  if (strcmp(cmd, "status:state") == 0) {
    sendStatusState(s);
    return;
  }

  if (strcmp(cmd, "status:features") == 0) {
    sendStatusFeatures(s);
    return;
  }

  if (strcmp(cmd, "status:can") == 0) {
    sendStatusCan(s);
    return;
  }

  if (strcmp(cmd, "status") == 0) {
    sendStatus(s, now);
    return;
  }

  if (executeStreamCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.streamEnabled ? F("Stream started") : F("Stream stopped"));
    sendStatus(s, now);
    return;
  }

  if (executeCanRawCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.rawCanListen ? F("Raw CAN mode enabled") : F("Filtered CAN mode"));
    sendStatus(s, now);
    return;
  }

  if (executeCanClockCmd(cmd, s)) {
    driverSetClockMHz(s.canClockReqMHz);
    bool ok = driverReinit();
    applyFilters(s);
    s.canClockReqMHz = driverGetClockReqMHz();
    s.canClockMHz = driverGetClockMHz();
    sendAck(cmd);
    sendLog(ok ? F("CAN clock profile applied") : F("CAN clock profile failed - check wiring/crystal"));
    sendStatus(s, now);
    return;
  }

  if (executeFsdCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.fsdEnabled ? F("FSD enabled - saved to NVS") : F("FSD disabled - saved to NVS"));
    sendStatus(s, now);
    return;
  }

  if (executeFsdForceCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.fsdForceEnabled ? F("FSD Force ON - saved to NVS") : F("FSD Force OFF - saved to NVS"));
    sendStatus(s, now);
    return;
  }

  if (executeNagCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.nagSuppress ? F("Nag suppress ON - saved") : F("Nag suppress OFF - saved"));
    sendStatus(s, now);
    return;
  }

  if (executeProfileCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.profileOverride ? F("Profile pinned - saved") : F("Profile set to auto"));
    sendStatus(s, now);
    return;
  }

  if (executeOffsetCmd(cmd, s)) {
    sendAck(cmd);
    if (s.detectedHW == 3 || s.variant == HW4)
      sendLog(s.speedOffset > 0 ? F("Offset updated - saved") : F("Offset disabled - saved"));
    else
      sendLog(s.offsetOverride ? F("Offset pinned - saved") : F("Offset set to auto"));
    sendStatus(s, now);
    return;
  }

  if (executeIsaChimeCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.isaChimeSuppress ? F("ISA chime suppressed - saved") : F("ISA chime original - saved"));
    sendStatus(s, now);
    return;
  }

  if (executeBanShieldCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.banShieldEnabled ? F("Ban Shield ON - telemetry monitoring active") : F("Ban Shield OFF"));
    sendStatus(s, now);
    return;
  }

  if (executeGtwShieldCmd(cmd, s)) {
    sendAck(cmd);
    if (s.gtwShieldArmed) sendLog(F("GTW shield ARMED - blocking 0x7FF changes"));
    else sendLog(F("GTW shield disarmed"));
    sendStatus(s, now);
    return;
  }

  if (executeTlsscCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.tlsscRestore ? F("TLSSC restore ON - spoofing SELF_DRIVING tier") : F("TLSSC restore OFF"));
    sendStatus(s, now);
    return;
  }

  if (executeSummonInjectCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.summonInject ? F("Summon injection ON - saved") : F("Summon injection OFF - saved"));
    sendStatus(s, now);
    return;
  }

  if (executeSummonCmd(cmd, s)) {
    sendAck(cmd);
    if (s.summonRemaining > 0)
      sendLog(F("Summon burst started (30 frames)"));
    else
      sendLog(F("Summon stopped"));
    sendStatus(s, now);
    return;
  }

  if (executeVariantCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(F("Variant changed - filters updated"));
    sendStatus(s, now);
    return;
  }

  if (executeNagKillerCmd(cmd, s)) {
    sendAck(cmd);
    if (strncmp(cmd, "nag:killer:mode:", 16) == 0) {
      sendLog(F("Nag killer mode updated - saved"));
    } else {
      sendLog(s.nagKillerEnabled ? F("Nag killer ON - saved") : F("Nag killer OFF - saved"));
    }
    sendStatus(s, now);
    return;
  }
  if (execBmsCmd(cmd, s)) {
    auto bmsCore = [&](JsonLineBuilder& out) {
      out.num("v", (long)(s.bmsVoltage * 100))
        .num("a", (long)(s.bmsCurrent * 10))
        .num("kw", (long)(s.bmsPower * 10))
        .num("soc", (long)(s.bmsSoc * 10))
        .num("tMin", s.bmsTempMin)
        .num("tMax", s.bmsTempMax)
        .num("whkm", (long)(s.bmsWhPerKm * 10))
        .num("nomFull", (long)(s.bmsNominalFullPack * 100))
        .num("nomRemain", (long)(s.bmsNominalRemaining * 100))
        .num("idealRemain", (long)(s.bmsIdealRemaining * 100))
        .num("cellVMax", (long)(s.bmsCellVoltageMax * 1000))
        .num("cellVMin", (long)(s.bmsCellVoltageMin * 1000))
        .num("maxRegen", (long)(s.bmsMaxRegenPower * 100))
        .num("maxDischarge", (long)(s.bmsMaxDischargePower * 100))
        .boolean("enhanced", s.hasEnhancedBms)
        .boolean("ok", s.hasBms);
    };

    auto bmsExtended = [&](JsonLineBuilder& out) {
      out.num("socUI", (long)(s.bmsSocUI * 10))
        .num("socMax", (long)(s.bmsSocMax * 10))
        .num("socAvg", (long)(s.bmsSocAvg * 10))
        .num("initFull", (long)(s.bmsInitialFullPack * 10))
        .num("expRange", (long)(s.bmsExpectedRange * 10))
        .num("idealRange", (long)(s.bmsIdealRange * 10))
        .num("ratedCons", (long)(s.bmsRatedConsumption * 10))
        .num("actSoc", s.bmsActualSocInt)
        .num("useSoc", s.bmsUsableSocInt)
        .num("pwrDiss", (long)(s.bmsPowerDissipation * 100))
        .num("flowReq", (long)(s.bmsFlowRequest * 10))
        .num("coolTgt", (long)(s.bmsCoolTarget * 10))
        .num("heatTgt", (long)(s.bmsHeatTarget * 10))
        .num("packTMin", (long)(s.bmsPackTMin * 10))
        .num("packTMax", (long)(s.bmsPackTMax * 10))
        .num("heatPwr", (long)(s.bmsStationaryHeatPower * 100))
        .num("hvacBgt", (long)(s.bmsHvacPowerBudget * 100))
        .boolean("precondOk", s.bmsPrecondAllowed)
        .boolean("heatWorth", s.bmsHeatingWorthwhile)
        .num("contState", s.bmsContactorState)
        .num("hvState", s.bmsHvState)
        .num("minBusV", (long)(s.bmsMinBusVoltage * 100))
        .num("maxBusV", (long)(s.bmsMaxBusVoltage * 100))
        .num("maxChgA", (long)(s.bmsMaxChargeCurrent * 10))
        .num("maxDchA", (long)(s.bmsMaxDischargeCurrent * 10))
        .num("expRemain", (long)(s.bmsExpectedRemaining * 100))
        .num("eBuf", (long)(s.bmsEnergyBuffer * 100))
        .num("eToChg", (long)(s.bmsEnergyToCharge * 100))
        .boolean("charged", s.bmsFullyCharged)
        .num("kwhDch", (long)(s.bmsKwhDischargeTotal))
        .num("kwhChg", (long)(s.bmsKwhChargeTotal))
        .num("acChg", (long)(s.bmsAcChargeTotal))
        .num("dcChg", (long)(s.bmsDcChargeTotal))
        .num("regen", (long)(s.bmsRegenTotal))
        .num("drvDch", (long)(s.bmsDriveDischargeTotal))
        .num("chgTime", (long)(s.bmsChargeTimeToFull * 100));
    };

    jsonLine().str("t", "bms").merge(bmsCore, bmsExtended).end();
    return;
  }

  // Body bus commands (window, sentry)
#if BUS_BODY_ACTIVE
  if (execWindowCmd(cmd, s) ||
      execSentryCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(cmd);
    sendStatus(s, now);
    return;
  }
#endif

  // Vehicle bus commands (climate, charge, drive, precondition, track mode)
#if BUS_VEHICLE_ACTIVE
  if (execPreconditionCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.preconditionEnabled ? F("Precondition ON - saved") : F("Precondition OFF - saved"));
    sendStatus(s, now);
    return;
  }

  if (execTrackModeCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.trackModeEnabled ? F("Track mode ON - saved") : F("Track mode OFF - saved"));
    sendStatus(s, now);
    return;
  }

  if (execClimateCmd(cmd, s) ||
      execChargeCmd(cmd, s) ||
      execPedalCmd(cmd, s) ||
      execRegenCmd(cmd, s) ||
      execStopCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(cmd);
    sendStatus(s, now);
    return;
  }

  if (!s.hasCtrl) {
    sendError(F("Waiting for 0x273 frame"));
    return;
  }

  if (s.variant != LEGACY && (
      execMirrorCmd(cmd, s) || execLockCmd(cmd, s) ||
      execLightCmd(cmd, s) || execWiperCmd(cmd, s) || execSeatCmd(cmd, s) ||
      execDisplayCmd(cmd, s) || execPowerCmd(cmd, s))) {
    sendAck(cmd);
    sendLog(cmd);
    sendStatus(s, now);
    return;
  }
#endif

  // Trunk commands (frunk = vehicle bus, trunk/glovebox = body bus)
#if BUS_VEHICLE_ACTIVE || BUS_BODY_ACTIVE
  if (s.variant != LEGACY && execTrunkCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(cmd);
    sendStatus(s, now);
    return;
  }
#endif

  // TPMS query
  if (execTpmsCmd(cmd, s)) {
    jsonLine()
      .str("t", "tpms")
      .num("fl", (long)(s.tpmsPressure[0] * 100))
      .num("fr", (long)(s.tpmsPressure[1] * 100))
      .num("rl", (long)(s.tpmsPressure[2] * 100))
      .num("rr", (long)(s.tpmsPressure[3] * 100))
      .num("tfl", s.tpmsTemp[0])
      .num("tfr", s.tpmsTemp[1])
      .num("trl", s.tpmsTemp[2])
      .num("trr", s.tpmsTemp[3])
      .boolean("ok", s.hasTpms)
      .end();
    return;
  }

  // Drive mode override commands: drivemode:off, drivemode:chill, drivemode:standard, drivemode:performance
  if (executeDriveModeCmd(cmd, s)) {
    sendAck(cmd);
    saveSettings(s);
    sendLog(s.driveModeOverride == 0 ? F("Drive mode override OFF - saved") : F("Drive mode override active - saved"));
    sendStatus(s, now);
    return;
  }

  // ECE R79 bypass toggle: ecer79:on / ecer79:off
  if (strcmp(cmd, "ecer79:on") == 0) {
    s.eceR79Bypass = true;
    saveSettings(s);
    sendAck(cmd);
    sendLog(F("ECE R79 bypass ON - saved"));
    sendStatus(s, now);
    return;
  }
  if (strcmp(cmd, "ecer79:off") == 0) {
    s.eceR79Bypass = false;
    saveSettings(s);
    sendAck(cmd);
    sendLog(F("ECE R79 bypass OFF - saved"));
    sendStatus(s, now);
    return;
  }

  // Enhanced Autopilot: eap:on / eap:off
  // Sets bit46 on mux=1 to unlock EAP/Summon features
  // Source: ev-open-can-tools + hypery11 enhanced_autopilot
  if (strcmp(cmd, "eap:on") == 0) {
    s.enhancedAutopilot = true;
    saveSettings(s);
    sendAck(cmd);
    sendLog(F("Enhanced Autopilot ON - EAP/Summon unlocked"));
    sendStatus(s, now);
    return;
  }
  if (strcmp(cmd, "eap:off") == 0) {
    s.enhancedAutopilot = false;
    saveSettings(s);
    sendAck(cmd);
    sendLog(F("Enhanced Autopilot OFF - saved"));
    sendStatus(s, now);
    return;
  }

  // Emergency Vehicle Detection: evd:on / evd:off (HW4 only, bit59 on mux=0)
  // Source: hypery11/flipper-tesla-fsd fsd_handler.c
  if (strcmp(cmd, "evd:on") == 0) {
    s.evdEnabled = true;
    saveSettings(s);
    sendAck(cmd);
    sendLog(F("EVD ON - emergency vehicle detection bit enabled (HW4)"));
    sendStatus(s, now);
    return;
  }
  if (strcmp(cmd, "evd:off") == 0) {
    s.evdEnabled = false;
    saveSettings(s);
    sendAck(cmd);
    sendLog(F("EVD OFF - saved"));
    sendStatus(s, now);
    return;
  }

  // Rate limiting toggle: ratelimit:on / ratelimit:off
  if (strcmp(cmd, "ratelimit:on") == 0) {
    s.rateLimitEnabled = true;
    saveSettings(s);
    sendAck(cmd);
    sendLog(F("Rate limiting ON - saved"));
    sendStatus(s, now);
    return;
  }
  if (strcmp(cmd, "ratelimit:off") == 0) {
    s.rateLimitEnabled = false;
    saveSettings(s);
    sendAck(cmd);
    sendLog(F("Rate limiting OFF - saved"));
    sendStatus(s, now);
    return;
  }

  // Log ring dump: show last N log entries
  if (strcmp(cmd, "log") == 0) {
    uint16_t count = logRingCount();
    JsonLineBuilder line = jsonLine();
    line.str("t", "log").num("count", count).raw(F(",\"entries\":["));
    for (uint16_t i = 0; i < count; i++) {
      if (i > 0) printStr(F(","));
      const LogEntry* e = logRingGet(i);
      if (!e) break;
      printStr(F("{\"ms\":"));
      printNum(e->timestamp);
      printStr(F(",\"m\":\""));
      printStr(e->msg);
      printStr(F("\"}"));
    }
    line.raw(F("]")).end();
    return;
  }

  // Turn signals: turn:left3, turn:right3, turn:hazard, turn:off
#if BUS_VEHICLE_ACTIVE
  if (execTurnSignalCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(cmd);
    sendStatus(s, now);
    return;
  }
#endif

  // Seatbelt emulation: seatbelt:on, seatbelt:off
#if BUS_VEHICLE_ACTIVE
  if (execSeatbeltCmd(cmd, s)) {
    sendAck(cmd);
    saveSettings(s);
    sendLog(s.seatbeltEmulation ? F("Seatbelt emulation ON - saved") : F("Seatbelt emulation OFF - saved"));
    sendStatus(s, now);
    return;
  }
#endif

  // Air recirculation: airecirc:on, airecirc:off
#if BUS_VEHICLE_ACTIVE
  if (execAirRecircCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(cmd);
    sendStatus(s, now);
    return;
  }
#endif

  // Wiper persistence: wiperpersist:on, wiperpersist:off
#if BUS_VEHICLE_ACTIVE
  if (execWiperPersistCmd(cmd, s)) {
    sendAck(cmd);
    saveSettings(s);
    sendLog(s.wiperPersistEnabled ? F("Wiper persist ON - saved") : F("Wiper persist OFF - saved"));
    sendStatus(s, now);
    return;
  }
#endif

  // Mirror auto-fold: mirror:autofold:on, mirror:autofold:off
#if BUS_VEHICLE_ACTIVE
  if (execMirrorAutoFoldCmd(cmd, s)) {
    sendAck(cmd);
    saveSettings(s);
    sendLog(s.mirrorAutoFoldEnabled ? F("Mirror auto-fold ON - saved") : F("Mirror auto-fold OFF - saved"));
    sendStatus(s, now);
    return;
  }
#endif

  // Powertrain telemetry query
#if BUS_VEHICLE_ACTIVE
  if (execPowertrainCmd(cmd, s)) {
    jsonLine()
      .str("t", "powertrain")
      .num("speed", (long)(s.vehicleSpeed * 100))
      .num("gear", s.gearState)
      .num("pedal", s.accelPedal)
      .num("steer", (long)(s.steeringAngle * 10))
      .num("rpmR", s.rearMotorRpm)
      .num("rpmF", s.frontMotorRpm)
      .boolean("ok", s.hasPowertrain)
      .end();
    return;
  }
#endif

  // CAN simulation: simu:start, simu:stop
  if (execCanSimCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.canSimEnabled ? F("CAN simulation started") : F("CAN simulation stopped"));
    sendStatus(s, now);
    return;
  }

  // Single-shot TX mode: singleshot:on, singleshot:off
  if (execSingleShotCmd(cmd, s)) {
    sendAck(cmd);
    saveSettings(s);
    driverSetSingleShot(s.singleShotTx);
    sendLog(s.singleShotTx ? F("Single-shot TX ON - saved") : F("Single-shot TX OFF - saved"));
    sendStatus(s, now);
    return;
  }

  // Firmware version compatibility: fwcompat
  if (execFwCompatCmd(cmd, s)) {
    jsonLine()
      .str("t", "fwcompat")
      .num("year", s.fwYear)
      .num("release", s.fwRelease)
      .num("minor", s.fwMinor)
      .num("build", s.fwBuild)
      .num("compat", s.fwCompat)
      .boolean("ok", s.hasFwVersion)
      .end();
    return;
  }

  // MQTT bridge: mqtt:on/off, mqtt:broker:<host>, mqtt:port:<port>, mqtt:interval:<ms>
  if (execMqttCmd(cmd, s)) {
    sendAck(cmd);
    saveSettings(s);
    sendLog(s.mqttEnabled ? F("MQTT updated - saved") : F("MQTT OFF - saved"));
    sendStatus(s, now);
    return;
  }

  // Vehicle config query: vehicle
  if (execVehicleConfigCmd(cmd, s)) {
    jsonLine()
      .str("t", "vehicle")
      .num("model", s.vehicleModel)
      .num("year", s.vehicleYear)
      .boolean("ok", s.hasVehicleConfig)
      .end();
    return;
  }

  // Vehicle platform identity query: platform
  if (strcmp(cmd, "platform") == 0) {
    auto platformRoot = [&](JsonLineBuilder& out) {
      out.num("model", s.platformModel)
        .num("hwGen", s.platformHwGen)
        .num("swYear", s.platformSwYear)
        .num("swWeek", s.platformSwWeek)
        .num("swRelease", s.platformSwRelease)
        .num("fsdProto", s.platformFsdProto)
        .num("swCompat", s.platformSwCompat)
        .boolean("resolved", s.platformResolved);
    };

    extern bool mcpAvailable[];
    auto canHealthObject = [&](JsonLineBuilder::JsonObjectBuilder& obj) {
      for (uint8_t i = 0; i < BUS_MAX; i++) {
        char busKey[8];
        busKey[0] = 'b';
        busKey[1] = 'u';
        busKey[2] = 's';
        busKey[3] = (char)('0' + i);
        busKey[4] = '\0';

        obj.object(busKey, [&](JsonLineBuilder::JsonObjectBuilder& bus) {
          bus.boolean("on", busActive(i))
             .boolean("det", mcpAvailable[i]);
        });
      }
    };

    jsonLine()
      .str("t", "platform")
      .merge(platformRoot)
      .mergeObject("canHealth", canHealthObject)
      .end();
    return;
  }

  sendError(F("Unknown command"));
}

// ── Serial API ───────────────────────────────────────────────────────────────
void serialInit(State& s) {
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {}

#if BOARD_ENABLE_BLE
  bleInit();
#endif

  sendLog(F(BOARD_READY_MSG));
  sendBoot(s);
}

void serialTick(State& s) {
  unsigned long now = millis();

  // Periodic status
  const unsigned long statusInterval = statusLiveEnabled ? STATUS_LIVE_INTERVAL_MS : STATUS_INTERVAL_MS;
  if (now - lastStatusMs >= statusInterval) {
    lastStatusMs = now;
    sendStatus(s, now);
  }

  // USB commands
  while (Serial.available()) {
    char c = Serial.read();
    handleChar(usbBuf, usbLen, c, s);
  }

  // BLE commands
#if BOARD_ENABLE_BLE
  while (bleAvailable()) {
    char c = bleRead();
    handleChar(bleBuf, bleLen, c, s);
  }
#endif
}
