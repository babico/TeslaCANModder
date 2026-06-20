#pragma once

/**
 * @file firmware/lib/core/persist/native/board.h
 * @brief In-memory settings persistence for native (host) test builds
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <Preferences.h>
#include "core/types.h"
#include "core/persist/keys.h"

/**
 * @brief Load all persisted settings from the in-memory NVS into runtime state.
 *
 * Mirrors the ESP32 board implementation exactly — same keys, same defaults,
 * same return semantics.
 *
 * @param s Reference to the global State struct to populate.
 * @return true if settings were loaded, false if magic missing or zero.
 */
inline bool loadSettings(State &s)
{
	Preferences prefs;
	prefs.begin(NVS_NAMESPACE, true);
	uint8_t magic = prefs.getUChar(NVS_KEY_MAGIC, 0);
	uint8_t ver = prefs.getUChar(NVS_KEY_VERSION, 0);
	prefs.end();

	if (magic != NVS_SETTINGS_MAGIC || ver == 0)
		return false;

	prefs.begin(NVS_NAMESPACE, true);

	s.variant = (Variant)prefs.getUChar(PKEY_VARIANT, 0);
	s.fsdEnabled = prefs.getUChar(PKEY_FSD, 0);
	s.fsdForceEnabled = prefs.getUChar(PKEY_FSD_FORCE, 0);
	s.speedProfile = prefs.getUChar(PKEY_SPEED_PROF, 1);
	s.profileOverride = prefs.getUChar(PKEY_PROF_PIN, 0);
	s.speedOffset = prefs.getUChar(PKEY_SPEED_OFFSET, 0);
	s.offsetOverride = prefs.getUChar(PKEY_OFFSET_PIN, 0);
	s.isaChimeSuppress = prefs.getUChar(PKEY_ISA_CHIME, 0);
	s.summonInject = prefs.getUChar(PKEY_SUMMON_INJ, 0);
	s.nagMode = (NagMode)prefs.getUChar(PKEY_NAG_MODE, (uint8_t)NAG_MODE_OFF);
	s.nagOrganicDriverBypass = prefs.getUChar(PKEY_NAG_ORG_DB, 0);
	s.preconditionEnabled = prefs.getUChar(PKEY_PRECOND, 0);
	s.trackModeEnabled = prefs.getUChar(PKEY_TRACK_MODE, 0);
	s.variantAutoDetect = prefs.getUChar(PKEY_VAR_AUTO, 1);
	s.apInjectionGateEnabled = prefs.getUChar(PKEY_AP_GATE, 0);
	s.banShieldEnabled = prefs.getUChar(PKEY_BAN_SHIELD, 0);
	s.canClockReqMHz = prefs.getUChar(PKEY_CAN_CLOCK, 0);
	s.driveModeOverride = prefs.getUChar(PKEY_DRIVE_MODE, 0);
	s.eceR79Bypass = prefs.getUChar(PKEY_ECE_R79, 0);
	s.lhdEnabled = prefs.getUChar(PKEY_LHD, 0);
	s.assistNavEnable = prefs.getUChar(PKEY_ASSIST_NAV, 0);
	s.assistHandsOff = prefs.getUChar(PKEY_ASSIST_HOF, 0);
	s.assistDevMode = prefs.getUChar(PKEY_ASSIST_DEV, 0);
	s.laneGraphEnable = prefs.getUChar(PKEY_LANE_GRAPH, 0);
	s.assistTelemetryOff = prefs.getUChar(PKEY_ASSIST_TEL, 0);
	s.seatbeltEmulation = prefs.getUChar(PKEY_SEATBELT_E, 0);
	s.wiperPersistEnabled = prefs.getUChar(PKEY_WIPER_PERS, 0);
	s.savedWiperSpeed = prefs.getUChar(PKEY_WIPER_SPEED, 0);
	s.mirrorAutoFoldEnabled = prefs.getUChar(PKEY_MIRROR_AF, 0);
	s.singleShotTx = prefs.getUChar(PKEY_SINGLE_SHOT, 0);
	s.mqttEnabled = prefs.getUChar(PKEY_MQTT_EN, 0);
	s.mqttPort = prefs.getUShort(PKEY_MQTT_PORT, 1883);
	s.mqttInterval = prefs.getUShort(PKEY_MQTT_INTERVAL, 2000);
	size_t hostLen = prefs.getString(PKEY_MQTT_HOST, s.mqttHost, sizeof(s.mqttHost));
	if (hostLen == 0)
		s.mqttHost[0] = '\0';
	s.enhancedAutopilot = prefs.getUChar(PKEY_EAP, 0);
	s.evdEnabled = prefs.getUChar(PKEY_EVD, 0);
	s.tlsscRestore = prefs.getUChar(PKEY_TLSSC, 0);
	s.bleDistanceMode = (BleDistanceMode)prefs.getUChar(PKEY_BLE_DIST_MODE, (uint8_t)BLE_DISTANCE_FORMULA);
	s.bleDistanceFactor = prefs.getFloat(PKEY_BLE_DIST_FACTOR, 2.0f);
	s.bleDistanceCalOffset = prefs.getFloat(PKEY_BLE_DIST_OFFSET, 0.0f);
	s.bleDistanceCalibrated = prefs.getUChar(PKEY_BLE_DIST_CAL, 0);
	prefs.end();
	return true;
}

/**
 * @brief Persist all runtime settings from State into in-memory NVS.
 *
 * Mirrors the ESP32 board implementation exactly — same keys, same values.
 *
 * @param s Reference to the global State struct to save.
 */
inline void saveSettings(const State &s)
{
	Preferences prefs;
	prefs.begin(NVS_NAMESPACE, false);
	prefs.putUChar(NVS_KEY_MAGIC, NVS_SETTINGS_MAGIC);
	prefs.putUChar(NVS_KEY_VERSION, NVS_SETTINGS_VERSION);

	prefs.putUChar(PKEY_VARIANT, (uint8_t)s.variant);
	prefs.putUChar(PKEY_FSD, s.fsdEnabled ? 1 : 0);
	prefs.putUChar(PKEY_FSD_FORCE, s.fsdForceEnabled ? 1 : 0);
	prefs.putUChar(PKEY_SPEED_PROF, (uint8_t)s.speedProfile);
	prefs.putUChar(PKEY_PROF_PIN, s.profileOverride ? 1 : 0);
	prefs.putUChar(PKEY_SPEED_OFFSET, (uint8_t)s.speedOffset);
	prefs.putUChar(PKEY_OFFSET_PIN, s.offsetOverride ? 1 : 0);
	prefs.putUChar(PKEY_ISA_CHIME, s.isaChimeSuppress ? 1 : 0);
	prefs.putUChar(PKEY_SUMMON_INJ, s.summonInject ? 1 : 0);
	prefs.putUChar(PKEY_NAG_MODE, (uint8_t)s.nagMode);
	prefs.putUChar(PKEY_NAG_ORG_DB, s.nagOrganicDriverBypass ? 1 : 0);
	prefs.putUChar(PKEY_PRECOND, s.preconditionEnabled ? 1 : 0);
	prefs.putUChar(PKEY_TRACK_MODE, s.trackModeEnabled ? 1 : 0);
	prefs.putUChar(PKEY_VAR_AUTO, s.variantAutoDetect ? 1 : 0);
	prefs.putUChar(PKEY_AP_GATE, s.apInjectionGateEnabled ? 1 : 0);
	prefs.putUChar(PKEY_BAN_SHIELD, s.banShieldEnabled ? 1 : 0);
	prefs.putUChar(PKEY_CAN_CLOCK, s.canClockReqMHz);
	prefs.putUChar(PKEY_DRIVE_MODE, s.driveModeOverride);
	prefs.putUChar(PKEY_ECE_R79, s.eceR79Bypass ? 1 : 0);
	prefs.putUChar(PKEY_LHD, s.lhdEnabled ? 1 : 0);
	prefs.putUChar(PKEY_ASSIST_NAV, s.assistNavEnable ? 1 : 0);
	prefs.putUChar(PKEY_ASSIST_HOF, s.assistHandsOff ? 1 : 0);
	prefs.putUChar(PKEY_ASSIST_DEV, s.assistDevMode ? 1 : 0);
	prefs.putUChar(PKEY_LANE_GRAPH, s.laneGraphEnable ? 1 : 0);
	prefs.putUChar(PKEY_ASSIST_TEL, s.assistTelemetryOff ? 1 : 0);
	prefs.putUChar(PKEY_SEATBELT_E, s.seatbeltEmulation ? 1 : 0);
	prefs.putUChar(PKEY_WIPER_PERS, s.wiperPersistEnabled ? 1 : 0);
	prefs.putUChar(PKEY_WIPER_SPEED, s.savedWiperSpeed);
	prefs.putUChar(PKEY_MIRROR_AF, s.mirrorAutoFoldEnabled ? 1 : 0);
	prefs.putUChar(PKEY_SINGLE_SHOT, s.singleShotTx ? 1 : 0);
	prefs.putUChar(PKEY_MQTT_EN, s.mqttEnabled ? 1 : 0);
	prefs.putUShort(PKEY_MQTT_PORT, s.mqttPort);
	prefs.putUShort(PKEY_MQTT_INTERVAL, s.mqttInterval);
	prefs.putString(PKEY_MQTT_HOST, s.mqttHost);
	prefs.putUChar(PKEY_EAP, s.enhancedAutopilot ? 1 : 0);
	prefs.putUChar(PKEY_EVD, s.evdEnabled ? 1 : 0);
	prefs.putUChar(PKEY_TLSSC, s.tlsscRestore ? 1 : 0);
	prefs.putUChar(PKEY_BLE_DIST_MODE, (uint8_t)s.bleDistanceMode);
	prefs.putFloat(PKEY_BLE_DIST_FACTOR, s.bleDistanceFactor);
	prefs.putFloat(PKEY_BLE_DIST_OFFSET, s.bleDistanceCalOffset);
	prefs.putUChar(PKEY_BLE_DIST_CAL, s.bleDistanceCalibrated ? 1 : 0);
	prefs.end();
}
