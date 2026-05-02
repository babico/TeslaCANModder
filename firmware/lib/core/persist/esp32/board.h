#pragma once
#include <Preferences.h>
#include "core/types.h"

// ── NVS Persistence (ESP32 replacement for AVR EEPROM) ──────────────────────
// Uses the ESP32 Preferences library (NVS flash) instead of EEPROM.
// Namespace: "tcm" (TeslaCANModder)

#define NVS_NAMESPACE "tcm"
#define NVS_KEY_MAGIC "magic"
#define NVS_KEY_VERSION "ver"
#define NVS_SETTINGS_MAGIC 0xCA
#define NVS_SETTINGS_VERSION 0x0D

static Preferences prefs;

// ── NVS Corruption Recovery ──────────────────────────────────────────────────
// Auto-detect corrupt NVS on boot. If magic matches but version doesn't,
// attempt migration. If magic is wrong, reset to defaults gracefully.
inline bool nvsIsCorrupt()
{
	prefs.begin(NVS_NAMESPACE, true);
	uint8_t magic = prefs.getUChar(NVS_KEY_MAGIC, 0);
	prefs.end();
	return magic != 0 && magic != NVS_SETTINGS_MAGIC;
}

inline void nvsReset()
{
	prefs.begin(NVS_NAMESPACE, false);
	prefs.clear();
	prefs.end();
}

inline bool loadSettings(State &s)
{
	prefs.begin(NVS_NAMESPACE, true); // read-only
	uint8_t magic = prefs.getUChar(NVS_KEY_MAGIC, 0);
	uint8_t ver = prefs.getUChar(NVS_KEY_VERSION, 0);
	if (magic != NVS_SETTINGS_MAGIC || ver != NVS_SETTINGS_VERSION)
	{
		prefs.end();
		// Auto-recover: if magic matches but version is old, clear and re-save
		if (magic == NVS_SETTINGS_MAGIC && ver > 0 && ver < NVS_SETTINGS_VERSION)
		{
			nvsReset();
		}
		return false;
	}

	s.variant = (Variant)prefs.getUChar("variant", 0);
	s.fsdEnabled = prefs.getUChar("fsd", 0);
	s.fsdForceEnabled = prefs.getUChar("ffsd", 0);
	s.nagSuppress = prefs.getUChar("nag", 0);
	s.speedProfile = prefs.getUChar("sp", 1);
	s.profileOverride = prefs.getUChar("spPin", 0);
	s.speedOffset = prefs.getUChar("offset", 0);
	s.offsetOverride = prefs.getUChar("offPin", 0);
	s.isaChimeSuppress = prefs.getUChar("isa", 0);
	s.summonInject = prefs.getUChar("sumInj", 0);
	s.nagKillerEnabled = prefs.getUChar("nagK", 0);
	s.nagKillerMode = (NagKillerMode)prefs.getUChar("nagKM", (uint8_t)NAG_KILLER_LEGACY);
	s.preconditionEnabled = prefs.getUChar("precond", 0);
	s.trackModeEnabled = prefs.getUChar("track", 0);
	s.variantAutoDetect = prefs.getUChar("vAuto", 1);
	s.apInjectionGateEnabled = prefs.getUChar("apGate", 0);
	s.banShieldEnabled = prefs.getUChar("banS", 0);
	s.canClockReqMHz = prefs.getUChar("clkMHz", 0);
	s.driveModeOverride = prefs.getUChar("drvM", 0);
	s.eceR79Bypass = prefs.getUChar("eceR79", 0);
	s.lhdEnabled = prefs.getUChar("lhd", 0);
	s.assistNavEnable = prefs.getUChar("assistNav", 0);
	s.assistHandsOff = prefs.getUChar("assistHof", 0);
	s.assistDevMode = prefs.getUChar("assistDev", 0);
	s.laneGraphEnable = prefs.getUChar("laneGraph", 0);
	s.assistTelemetryOff = prefs.getUChar("assistTel", 0);
	s.seatbeltEmulation = prefs.getUChar("seatbE", 0);
	s.wiperPersistEnabled = prefs.getUChar("wipP", 0);
	s.savedWiperSpeed = prefs.getUChar("wipS", 0);
	s.mirrorAutoFoldEnabled = prefs.getUChar("mirAF", 0);
	s.singleShotTx = prefs.getUChar("ssTx", 0);
	s.mqttEnabled = prefs.getUChar("mqttE", 0);
	s.mqttPort = prefs.getUShort("mqttP", 1883);
	s.mqttInterval = prefs.getUShort("mqttI", 2000);
	size_t hostLen = prefs.getString("mqttH", s.mqttHost, sizeof(s.mqttHost));
	if (hostLen == 0)
		s.mqttHost[0] = '\0';
	s.enhancedAutopilot = prefs.getUChar("eap", 0);
	s.evdEnabled = prefs.getUChar("evd", 0);
	s.tlsscRestore = prefs.getUChar("tlssc", 0);
	prefs.end();
	return true;
}

inline void saveSettings(const State &s)
{
	prefs.begin(NVS_NAMESPACE, false); // read-write
	prefs.putUChar(NVS_KEY_MAGIC, NVS_SETTINGS_MAGIC);
	prefs.putUChar(NVS_KEY_VERSION, NVS_SETTINGS_VERSION);
	prefs.putUChar("variant", (uint8_t)s.variant);
	prefs.putUChar("fsd", s.fsdEnabled ? 1 : 0);
	prefs.putUChar("ffsd", s.fsdForceEnabled ? 1 : 0);
	prefs.putUChar("nag", s.nagSuppress ? 1 : 0);
	prefs.putUChar("sp", (uint8_t)s.speedProfile);
	prefs.putUChar("spPin", s.profileOverride ? 1 : 0);
	prefs.putUChar("offset", (uint8_t)s.speedOffset);
	prefs.putUChar("offPin", s.offsetOverride ? 1 : 0);
	prefs.putUChar("isa", s.isaChimeSuppress ? 1 : 0);
	prefs.putUChar("sumInj", s.summonInject ? 1 : 0);
	prefs.putUChar("nagK", s.nagKillerEnabled ? 1 : 0);
	prefs.putUChar("nagKM", (uint8_t)s.nagKillerMode);
	prefs.putUChar("precond", s.preconditionEnabled ? 1 : 0);
	prefs.putUChar("track", s.trackModeEnabled ? 1 : 0);
	prefs.putUChar("vAuto", s.variantAutoDetect ? 1 : 0);
	prefs.putUChar("apGate", s.apInjectionGateEnabled ? 1 : 0);
	prefs.putUChar("banS", s.banShieldEnabled ? 1 : 0);
	prefs.putUChar("clkMHz", s.canClockReqMHz);
	prefs.putUChar("drvM", s.driveModeOverride);
	prefs.putUChar("eceR79", s.eceR79Bypass ? 1 : 0);
	prefs.putUChar("lhd", s.lhdEnabled ? 1 : 0);
	prefs.putUChar("assistNav", s.assistNavEnable ? 1 : 0);
	prefs.putUChar("assistHof", s.assistHandsOff ? 1 : 0);
	prefs.putUChar("assistDev", s.assistDevMode ? 1 : 0);
	prefs.putUChar("laneGraph", s.laneGraphEnable ? 1 : 0);
	prefs.putUChar("assistTel", s.assistTelemetryOff ? 1 : 0);
	prefs.putUChar("seatbE", s.seatbeltEmulation ? 1 : 0);
	prefs.putUChar("wipP", s.wiperPersistEnabled ? 1 : 0);
	prefs.putUChar("wipS", s.savedWiperSpeed);
	prefs.putUChar("mirAF", s.mirrorAutoFoldEnabled ? 1 : 0);
	prefs.putUChar("ssTx", s.singleShotTx ? 1 : 0);
	prefs.putUChar("mqttE", s.mqttEnabled ? 1 : 0);
	prefs.putUShort("mqttP", s.mqttPort);
	prefs.putUShort("mqttI", s.mqttInterval);
	prefs.putString("mqttH", s.mqttHost);
	prefs.putUChar("eap", s.enhancedAutopilot ? 1 : 0);
	prefs.putUChar("evd", s.evdEnabled ? 1 : 0);
	prefs.putUChar("tlssc", s.tlsscRestore ? 1 : 0);
	prefs.end();
}
