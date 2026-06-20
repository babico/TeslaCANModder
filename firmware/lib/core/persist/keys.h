#pragma once

/**
 * @file firmware/lib/core/persist/keys.h
 * @brief Canonical NVS key names — single source of truth for all persisted fields
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

// ── NVS namespace and metadata ───────────────────────────────────────────
#define NVS_NAMESPACE "tcm"
#define NVS_KEY_MAGIC "magic"
#define NVS_KEY_VERSION "ver"
#define NVS_SETTINGS_MAGIC 0xCA
#define NVS_SETTINGS_VERSION 0x0E

// ── Feature toggle keys ───────────────────────────────────────────────────
#define PKEY_VARIANT "variant"
#define PKEY_FSD "fsd"
#define PKEY_FSD_FORCE "ffsd"
#define PKEY_SPEED_PROF "sp"
#define PKEY_PROF_PIN "spPin"
#define PKEY_SPEED_OFFSET "offset"
#define PKEY_OFFSET_PIN "offPin"
#define PKEY_ISA_CHIME "isa"
#define PKEY_SUMMON_INJ "sumInj"
#define PKEY_NAG_MODE "nagMode"
#define PKEY_NAG_ORG_DB "nagOrgDB"
#define PKEY_PRECOND "precond"
#define PKEY_TRACK_MODE "track"
#define PKEY_VAR_AUTO "vAuto"
#define PKEY_AP_GATE "apGate"
#define PKEY_BAN_SHIELD "banS"
#define PKEY_CAN_CLOCK "clkMHz"
#define PKEY_DRIVE_MODE "drvM"
#define PKEY_ECE_R79 "eceR79"
#define PKEY_LHD "lhd"
#define PKEY_ASSIST_NAV "assistNav"
#define PKEY_ASSIST_HOF "assistHof"
#define PKEY_ASSIST_DEV "assistDev"
#define PKEY_LANE_GRAPH "laneGraph"
#define PKEY_ASSIST_TEL "assistTel"
#define PKEY_SEATBELT_E "seatbE"
#define PKEY_WIPER_PERS "wipP"
#define PKEY_WIPER_SPEED "wipS"
#define PKEY_MIRROR_AF "mirAF"
#define PKEY_SINGLE_SHOT "ssTx"
#define PKEY_MQTT_EN "mqttE"
#define PKEY_MQTT_PORT "mqttP"
#define PKEY_MQTT_INTERVAL "mqttI"
#define PKEY_MQTT_HOST "mqttH"
#define PKEY_EAP "eap"
#define PKEY_EVD "evd"
#define PKEY_TLSSC "tlssc"
#define PKEY_BLE_DIST_MODE "bleDistM"
#define PKEY_BLE_DIST_FACTOR "bleDistF"
#define PKEY_BLE_DIST_OFFSET "bleDistO"
#define PKEY_BLE_DIST_CAL "bleDistC"
