#pragma once

/**
 * @file firmware/lib/core/persist/esp32/board.h
 * @brief NVS-based settings persistence for ESP32, replacing AVR EEPROM storage
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <Preferences.h>
#include "core/types.h"

#define NVS_NAMESPACE "tcm"           // NVS namespace for all TeslaCANModder keys
#define NVS_KEY_MAGIC "magic"         // Key storing the settings magic byte
#define NVS_KEY_VERSION "ver"         // Key storing the settings schema version
#define NVS_SETTINGS_MAGIC 0xCA       // Magic byte identifying valid TCM settings
#define NVS_SETTINGS_VERSION 0x0E     // Current settings schema version (14)

static Preferences prefs;

/**
 * @brief Check whether the NVS settings store is corrupt
 * @return true if magic byte is present but does not match expected value
 */
inline bool nvsIsCorrupt()
{
	prefs.begin(NVS_NAMESPACE, true);
	uint8_t magic = prefs.getUChar(NVS_KEY_MAGIC, 0);
	prefs.end();
	return magic != 0 && magic != NVS_SETTINGS_MAGIC;
}

/**
 * @brief Erase all keys in the TCM NVS namespace
 */
inline void nvsReset()
{
	prefs.begin(NVS_NAMESPACE, false);
	prefs.clear();
	prefs.end();
}

/**
 * @brief Load all persisted settings from NVS into the runtime state
 * @param s Reference to the global State struct to populate
 * @return true if settings were loaded successfully, false if missing or version mismatch
 */
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
	s.speedProfile = prefs.getUChar("sp", 1);
	s.profileOverride = prefs.getUChar("spPin", 0);
	s.speedOffset = prefs.getUChar("offset", 0);
	s.offsetOverride = prefs.getUChar("offPin", 0);
	s.isaChimeSuppress = prefs.getUChar("isa", 0);
	s.summonInject = prefs.getUChar("sumInj", 0);
	// Nag suppression mode and organic-driver bypass flag (see NagMode in core/types.h)
	s.nagMode = (NagMode)prefs.getUChar("nagMode", (uint8_t)NAG_MODE_OFF);
	s.nagOrganicDriverBypass = prefs.getUChar("nagOrgDB", 0);
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

/**
 * @brief Persist all runtime settings from State into NVS flash
 * @param s Reference to the global State struct to save
 */
inline void saveSettings(const State &s)
{
	prefs.begin(NVS_NAMESPACE, false); // read-write
	prefs.putUChar(NVS_KEY_MAGIC, NVS_SETTINGS_MAGIC);
	prefs.putUChar(NVS_KEY_VERSION, NVS_SETTINGS_VERSION);
	prefs.putUChar("variant", (uint8_t)s.variant);
	prefs.putUChar("fsd", s.fsdEnabled ? 1 : 0);
	prefs.putUChar("ffsd", s.fsdForceEnabled ? 1 : 0);
	prefs.putUChar("sp", (uint8_t)s.speedProfile);
	prefs.putUChar("spPin", s.profileOverride ? 1 : 0);
	prefs.putUChar("offset", (uint8_t)s.speedOffset);
	prefs.putUChar("offPin", s.offsetOverride ? 1 : 0);
	prefs.putUChar("isa", s.isaChimeSuppress ? 1 : 0);
	prefs.putUChar("sumInj", s.summonInject ? 1 : 0);
	// Nag suppression mode and organic-driver bypass flag
	prefs.putUChar("nagMode", (uint8_t)s.nagMode);
	prefs.putUChar("nagOrgDB", s.nagOrganicDriverBypass ? 1 : 0);
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
