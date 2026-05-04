#pragma once
#include "output.h"

inline const char* apGateReason(const State& s) {
  if (!s.apInjectionGateEnabled) return "disabled";
  if (s.apGateApActive) return "ap";
  if (s.apGateParked) return "park";
  if (s.apGateSummoning) return "summon";
  return "waiting";
}

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
      .boolean("apGateEnabled", s.apInjectionGateEnabled)
      .boolean("apGateOpen", s.apGateOpen())
      .str("apGateReason", apGateReason(s))
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
       .boolean("lhd", s.lhdEnabled)
       .boolean("assistNav", s.assistNavEnable)
       .boolean("assistHandsOff", s.assistHandsOff)
       .boolean("assistDev", s.assistDevMode)
       .boolean("laneGraph", s.laneGraphEnable)
       .boolean("assistTelOff", s.assistTelemetryOff)
       .boolean("apFirst", s.apFirstEnabled)
       .num("dasApState", s.dasApState)
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
       })
       .num("nagEchoCount", (long)s.canDiag.nagEchoCount)
       .num("eapModCount", (long)s.canDiag.eapModCount)
       .num("txFailCount", (long)s.canDiag.txFailCount)
       .num("busOffCount", (long)s.canDiag.busOffCount)
       .num("framesA", (long)s.canDiag.bus[0].frames)
       .num("framesB", (long)s.canDiag.bus[1].frames)
       .num("framesC", (long)s.canDiag.bus[2].frames)
       .num("hzA", (long)s.canDiag.bus[0].hz)
       .num("hzB", (long)s.canDiag.bus[1].hz)
       .num("hzC", (long)s.canDiag.bus[2].hz)
       .num("hzMinA", (long)(s.canDiag.bus[0].hzMin == 0xFFFF ? 0 : s.canDiag.bus[0].hzMin))
       .num("hzMinB", (long)(s.canDiag.bus[1].hzMin == 0xFFFF ? 0 : s.canDiag.bus[1].hzMin))
       .num("hzMinC", (long)(s.canDiag.bus[2].hzMin == 0xFFFF ? 0 : s.canDiag.bus[2].hzMin))
       .num("hzMaxA", (long)s.canDiag.bus[0].hzMax)
       .num("hzMaxB", (long)s.canDiag.bus[1].hzMax)
       .num("hzMaxC", (long)s.canDiag.bus[2].hzMax);
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
      .boolean("apGateEnabled", s.apInjectionGateEnabled)
      .boolean("apGateOpen", s.apGateOpen())
      .str("apGateReason", apGateReason(s))
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
       .boolean("lhd", s.lhdEnabled)
       .boolean("assistNav", s.assistNavEnable)
       .boolean("assistHandsOff", s.assistHandsOff)
       .boolean("assistDev", s.assistDevMode)
       .boolean("laneGraph", s.laneGraphEnable)
       .boolean("assistTelOff", s.assistTelemetryOff)
       .boolean("apFirst", s.apFirstEnabled)
       .num("dasApState", s.dasApState)
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
       })
       .num("nagEchoCount", (long)s.canDiag.nagEchoCount)
       .num("eapModCount", (long)s.canDiag.eapModCount)
       .num("txFailCount", (long)s.canDiag.txFailCount)
       .num("busOffCount", (long)s.canDiag.busOffCount)
       .num("framesA", (long)s.canDiag.bus[0].frames)
       .num("framesB", (long)s.canDiag.bus[1].frames)
       .num("framesC", (long)s.canDiag.bus[2].frames)
       .num("hzA", (long)s.canDiag.bus[0].hz)
       .num("hzB", (long)s.canDiag.bus[1].hz)
       .num("hzC", (long)s.canDiag.bus[2].hz)
       .num("hzMinA", (long)(s.canDiag.bus[0].hzMin == 0xFFFF ? 0 : s.canDiag.bus[0].hzMin))
       .num("hzMinB", (long)(s.canDiag.bus[1].hzMin == 0xFFFF ? 0 : s.canDiag.bus[1].hzMin))
       .num("hzMinC", (long)(s.canDiag.bus[2].hzMin == 0xFFFF ? 0 : s.canDiag.bus[2].hzMin))
       .num("hzMaxA", (long)s.canDiag.bus[0].hzMax)
       .num("hzMaxB", (long)s.canDiag.bus[1].hzMax)
       .num("hzMaxC", (long)s.canDiag.bus[2].hzMax);
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
      .boolean("txPaused", s.txPaused)
      .boolean("apGateEnabled", s.apInjectionGateEnabled)
      .boolean("apGateOpen", s.apGateOpen())
      .str("apGateReason", apGateReason(s));
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
      .boolean("trackMode", s.trackModeEnabled)
      .boolean("apGateEnabled", s.apInjectionGateEnabled)
      .boolean("apGateOpen", s.apGateOpen());
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
