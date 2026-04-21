#pragma once
#include <Arduino.h>
#include "core/config/uno.h"
#include "core/types.h"

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
#include "feature/auto_lane_change.h"
#include "feature/ban_shield.h"
#include "feature/profile.h"
#include "feature/offsets.h"
#include "feature/isa_chime.h"
#include "feature/summon.h"
#include "feature/variant.h"
#if !defined(BOARD_COMPACT_AVR)
#include "feature/bms.h"
#endif
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
#endif
#if BUS_BODY_ACTIVE
  #include "feature/window.h"
  #include "feature/sentry.h"
#endif
#if BUS_VEHICLE_ACTIVE || BUS_BODY_ACTIVE
  #include "feature/trunk.h"
#endif

#if BOARD_ENABLE_BT
  #include <SoftwareSerial.h>
  static SoftwareSerial btSerial(PIN_BT_RX, PIN_BT_TX);
#endif

static const uint8_t SERIAL_CMD_BUFFER_SIZE = 32;
static char usbBuf[SERIAL_CMD_BUFFER_SIZE];
static uint8_t usbLen = 0;
#if BOARD_ENABLE_BT
static char btBuf[SERIAL_CMD_BUFFER_SIZE];
static uint8_t btLen = 0;
#endif
static unsigned long lastStatusMs = 0;

// ── Output Helpers ───────────────────────────────────────────────────────────
void printStr(const char* s) {
  Serial.print(s);
#if BOARD_ENABLE_BT
  btSerial.print(s);
#endif
}

void printStr(const __FlashStringHelper* s) {
  Serial.print(s);
#if BOARD_ENABLE_BT
  btSerial.print(s);
#endif
}

void printNum(long n) {
  Serial.print(n);
#if BOARD_ENABLE_BT
  btSerial.print(n);
#endif
}

void printHex(uint8_t b) {
  if (b < 0x10) printStr("0");
  Serial.print(b, HEX);
#if BOARD_ENABLE_BT
  btSerial.print(b, HEX);
#endif
}

void printLn() {
  Serial.println();
#if BOARD_ENABLE_BT
  btSerial.println();
#endif
}

#include "io/serial/common.h"

// ── JSON Messages ────────────────────────────────────────────────────────────
void sendBoot(State& s) {
  Features f = s.features();
#if BUS_VEHICLE_ACTIVE || BUS_BODY_ACTIVE
  extern bool mcpAvailable[];
#endif

  jsonLine()
    .str("t", "boot")
    .object("meta", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.str("hw", BOARD_HW_NAME)
       .str("can", BOARD_CAN_NAME)
       .str("drv", BOARD_DRIVER_NAME)
       .str("variant", variantName(s.variant));
#if BOARD_ENABLE_BT
      o.str("cap", "usb+bluetooth");
#else
      o.str("cap", "usb");
#endif
      o.str("ready", "runtime-ready");
    })
    .object("connectivity", [&](JsonLineBuilder::JsonObjectBuilder& o) {
#if BOARD_ENABLE_BT
      o.boolean("btEnabled", true);
#else
      o.boolean("btEnabled", false);
#endif
    #if !defined(BOARD_COMPACT_AVR)
#if BUS_VEHICLE_ACTIVE
      o.boolean("vehicleOnline", mcpAvailable[1]);
#else
      o.boolean("vehicleOnline", false);
#endif
#if BUS_BODY_ACTIVE
      o.boolean("bodyOnline", mcpAvailable[2]);
#else
      o.boolean("bodyOnline", false);
#endif
#endif
      o.boolean("chassisOnline", s.chassisOnline)
       .boolean("standby", s.standby);
#if BOARD_ENABLE_BT
      o.str("btName", BOARD_BT_NAME);
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
#if !defined(BOARD_COMPACT_AVR)
       .num("gtwAutopilotTier", (int)s.gtwAutopilotTier)
#endif
       .boolean("rawCan", s.rawCanListen)
       .object("stream", [&](JsonLineBuilder::JsonObjectBuilder& stream) {
         stream.boolean("on", false).num("emitted", 0);
       });
    })
#if !defined(BOARD_COMPACT_AVR)
    .object("driverAssist", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.num("turnSignalLeft", s.turnSignalLeft ? 1 : 0)
       .num("turnSignalRight", s.turnSignalRight ? 1 : 0)
       .num("bsmLeftLevel", s.bsmLeftLevel)
       .num("bsmRightLevel", s.bsmRightLevel)
       .num("cruiseSetSpeed", (long)(s.cruiseSetSpeedKph * 10))
       .num("accSpeedLimit", (long)(s.accSpeedLimitKph * 10))
       .num("mapSpeedLimit", (long)(s.mapSpeedLimitKph * 10))
       .num("maxSpeed", (long)(s.maxSpeedKph * 10))
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
       .boolean("trunkOpen", s.trunkOpen);
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
    #endif
    .object("safety", [&](JsonLineBuilder::JsonObjectBuilder& o) {
    #if defined(BOARD_COMPACT_AVR)
      o.boolean("banShield", s.banShieldEnabled);
    #else
      o.boolean("banShield", s.banShieldEnabled)
       .num("banThreat", s.banThreatLevel)
       .num("banDetectCount", s.banDetectionCount);
    #endif
    })
    .object("can", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.num("clockReqMHz", s.canClockReqMHz).num("clockMHz", s.canClockMHz);
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
    .end();
}

void sendStatus(State& s, unsigned long now) {
  Features f = s.features();
#if BUS_VEHICLE_ACTIVE || BUS_BODY_ACTIVE
  extern bool mcpAvailable[];
#endif

  jsonLine()
    .str("t", "status")
    .object("meta", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.str("hw", BOARD_HW_NAME)
       .str("can", BOARD_CAN_NAME)
       .str("drv", BOARD_DRIVER_NAME)
       .str("variant", variantName(s.variant));
#if BOARD_ENABLE_BT
      o.str("cap", "usb+bluetooth");
#else
      o.str("cap", "usb");
#endif
      o.str("ready", "runtime-ready").num("up", now);
    })
    .object("connectivity", [&](JsonLineBuilder::JsonObjectBuilder& o) {
#if BOARD_ENABLE_BT
      o.boolean("btEnabled", true);
#else
      o.boolean("btEnabled", false);
#endif
    #if !defined(BOARD_COMPACT_AVR)
#if BUS_VEHICLE_ACTIVE
      o.boolean("vehicleOnline", mcpAvailable[1]);
#else
      o.boolean("vehicleOnline", false);
#endif
#if BUS_BODY_ACTIVE
      o.boolean("bodyOnline", mcpAvailable[2]);
#else
      o.boolean("bodyOnline", false);
#endif
#endif
      o.boolean("chassisOnline", s.chassisOnline)
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
#if !defined(BOARD_COMPACT_AVR)
       .num("gtwAutopilotTier", (int)s.gtwAutopilotTier)
#endif
       .boolean("rawCan", s.rawCanListen)
       .object("stream", [&](JsonLineBuilder::JsonObjectBuilder& stream) {
         stream.boolean("on", s.streamEnabled).num("emitted", s.streamCount);
       });
    })
#if !defined(BOARD_COMPACT_AVR)
    .object("driverAssist", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.num("turnSignalLeft", s.turnSignalLeft ? 1 : 0)
       .num("turnSignalRight", s.turnSignalRight ? 1 : 0)
       .num("bsmLeftLevel", s.bsmLeftLevel)
       .num("bsmRightLevel", s.bsmRightLevel)
       .num("cruiseSetSpeed", (long)(s.cruiseSetSpeedKph * 10))
       .num("accSpeedLimit", (long)(s.accSpeedLimitKph * 10))
       .num("mapSpeedLimit", (long)(s.mapSpeedLimitKph * 10))
       .num("maxSpeed", (long)(s.maxSpeedKph * 10))
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
       .boolean("trunkOpen", s.trunkOpen);
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
    #endif
    .object("safety", [&](JsonLineBuilder::JsonObjectBuilder& o) {
    #if defined(BOARD_COMPACT_AVR)
      o.boolean("banShield", s.banShieldEnabled);
    #else
      o.boolean("banShield", s.banShieldEnabled)
       .num("banThreat", s.banThreatLevel)
       .num("banDetectCount", s.banDetectionCount);
    #endif
    })
    .object("can", [&](JsonLineBuilder::JsonObjectBuilder& o) {
      o.num("clockReqMHz", s.canClockReqMHz).num("clockMHz", s.canClockMHz);
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
    .end();
}

// ── Command Parser ───────────────────────────────────────────────────────────
void executeCommand(const char* cmd, State& s, unsigned long now) {
  // System commands
  if (strcmp(cmd, "ping") == 0) {
    jsonLine().str("t", "pong").num("v", 1).end();
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
    sendLog(s.fsdEnabled ? F("FSD enabled - saved to EEPROM") : F("FSD disabled - saved to EEPROM"));
    sendStatus(s, now);
    return;
  }

  if (executeFsdForceCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.fsdForceEnabled ? F("FSD Force ON - saved to EEPROM") : F("FSD Force OFF - saved to EEPROM"));
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
  if (executeAlcCmd(cmd, s)) {
    sendAck(cmd);
    sendLog(s.alcAutoConfirmEnabled ? F("ALC auto-confirm ON") : F("ALC auto-confirm OFF"));
    sendStatus(s, now);
    return;
  }
  if (executeRegionSpoofCmd(cmd, s)) {
    sendAck(cmd);
    if (s.regionSpoofCode == 0) {
      sendLog(F("Region spoof OFF"));
    } else {
      sendLog(F("Region spoofed - saved"));
    }
    sendStatus(s, now);
    return;
  }

#if !defined(BOARD_COMPACT_AVR)
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
#endif

  // Vehicle bus commands (precondition, track mode, climate, charge, drive)
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

  sendError(F("Unknown command"));
}

// ── Serial API ───────────────────────────────────────────────────────────────
void serialInit(State& s) {
#if BOARD_ENABLE_BT
  btSerial.begin(BT_BAUD);
#endif
  delay(1000);
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {}
  sendLog(F(BOARD_READY_MSG));
  sendBoot(s);
}

void serialTick(State& s) {
  unsigned long now = millis();

  // Periodic status
  if (now - lastStatusMs >= STATUS_INTERVAL_MS) {
    lastStatusMs = now;
    sendStatus(s, now);
  }

  // USB commands
  while (Serial.available()) {
    char c = Serial.read();
    handleChar(usbBuf, usbLen, c, s);
  }

  // Bluetooth commands
#if BOARD_ENABLE_BT
  while (btSerial.available()) {
    char c = btSerial.read();
    handleChar(btBuf, btLen, c, s);
  }
#endif
}
