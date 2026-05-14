#pragma once

/**
 * @file firmware/lib/client/api/routes.h
 * @brief REST API route handlers, CORS helpers, and JSON state builder for the WiFi server.
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "client/common/api_fwd.h"
#include "io/wifi/esp32/cmd.h"
#include "auth.h"
#include "core/can/recorder.h"
#include "vehicle/can/feature/das_drive.h"
#if BOARD_ENABLE_BLE
#include "client/gamepad/gamepad.h"
#endif

/**
 * @brief Build a comprehensive JSON representation of the current application state.
 *
 * Serializes vehicle status, feature flags, hardware info, CAN recorder stats,
 * and optional BLE/gamepad subsystem snapshots into a single JSON string suitable
 * for the GET /api/status response.
 *
 * @param s Global application state to serialize.
 * @return Serialized JSON string containing the full state snapshot.
 */
static String buildStateJson(State &s)
{
	JsonDocument doc;
	Features feat = getFeatures(s.variant);

	doc["variant"] = variantName(s.variant);
	doc["fsd"] = s.fsdEnabled;
	doc["nagMode"] = nagModeName(s.nagMode);
	doc["nagOrgBypass"] = s.nagOrganicDriverBypass;
	doc["dasHandsOn"] = s.dasHandsOnState;
	doc["profile"] = s.speedProfile;
	doc["profilePin"] = s.profileOverride;
	doc["offset"] = s.speedOffset;
	doc["offsetPin"] = s.offsetOverride;
	doc["isaChime"] = s.isaChimeSuppress;
	doc["stream"] = s.streamEnabled;
	doc["rawCan"] = s.rawCanListen;
	doc["chassisOnline"] = s.chassisOnline;
	doc["standby"] = s.standby;
	doc["uptime"] = millis();

	// BMS telemetry — serialized for dashboard telemetry cards
	if (s.hasBms)
	{
		doc["bmsVoltage"] = s.bmsVoltage;
		doc["bmsCurrent"] = s.bmsCurrent;
		doc["bmsPower"] = s.bmsPower;
		doc["bmsSoc"] = s.bmsSoc;
		doc["bmsTempMin"] = s.bmsTempMin;
		doc["bmsTempMax"] = s.bmsTempMax;
		doc["hasBms"] = 1;
	}
	if (s.hasEnhancedBms)
	{
		doc["bmsCellVoltageMax"] = s.bmsCellVoltageMax;
		doc["bmsCellVoltageMin"] = s.bmsCellVoltageMin;
		doc["bmsMaxRegenPower"] = s.bmsMaxRegenPower;
		doc["bmsMaxDischargePower"] = s.bmsMaxDischargePower;
		doc["bmsNominalFullPack"] = s.bmsNominalFullPack;
		doc["bmsNominalRemaining"] = s.bmsNominalRemaining;
		doc["bmsPackTMin"] = s.bmsPackTMin;
		doc["bmsPackTMax"] = s.bmsPackTMax;
		doc["bmsMaxDischargeCurrent"] = s.bmsMaxDischargeCurrent;
		doc["hasEnhancedBms"] = 1;
	}
	doc["driveMode"] = s.driveModeOverride;
	doc["currentDriveMode"] = s.currentDriveMode;
	doc["eceR79"] = s.eceR79Bypass;
	doc["regionCode"] = s.regionCode;
	doc["hasRegion"] = s.hasRegion;
	doc["cnLocked"] = s.chineseGatewayLocked;
	doc["hasTpms"] = s.hasTpms;
	doc["seatbeltEmulation"] = s.seatbeltEmulation;
	doc["wiperPersist"] = s.wiperPersistEnabled;
	doc["mirrorAutoFold"] = s.mirrorAutoFoldEnabled;
	doc["canSim"] = s.canSimEnabled;
	doc["hasPowertrain"] = s.hasPowertrain;
	doc["otaInProgress"] = s.otaInProgress;
	doc["txPaused"] = s.txPaused;
	doc["apGateEnabled"] = s.apInjectionGateEnabled;
	doc["apGateAp"] = s.apGateApActive;
	doc["apGatePark"] = s.apGateParked;
	doc["apGateSummon"] = s.apGateSummoning;
	doc["apGateOpen"] = s.apGateOpen();
	doc["enhancedAutopilot"] = s.enhancedAutopilot;
	doc["canRecorderEnabled"] = canRecorderEnabled();
	doc["canRecorderCount"] = canRecorderCount();
	doc["turnSignalLeft"] = s.turnSignalLeft;
	doc["turnSignalRight"] = s.turnSignalRight;
	doc["bsmLeftLevel"] = s.bsmLeftLevel;
	doc["bsmRightLevel"] = s.bsmRightLevel;
	doc["doorFrontLeftOpen"] = s.doorFrontLeftOpen;
	doc["doorFrontRightOpen"] = s.doorFrontRightOpen;
	doc["doorRearLeftOpen"] = s.doorRearLeftOpen;
	doc["doorRearRightOpen"] = s.doorRearRightOpen;
	doc["driverDoorOpen"] = s.driverDoorOpen;
	doc["anyDoorOpen"] = s.anyDoorOpen;
	doc["frunkOpen"] = s.frunkOpen;
	doc["trunkOpen"] = s.trunkOpen;
	doc["cruiseSetSpeed"] = (int)(s.cruiseSetSpeedKph * 10); // Fixed-point: kph * 10
	doc["accSpeedLimit"] = (int)(s.accSpeedLimitKph * 10);	 // Fixed-point: kph * 10
	doc["mapSpeedLimit"] = (int)(s.mapSpeedLimitKph * 10);	 // Fixed-point: kph * 10
	doc["maxSpeed"] = (int)(s.maxSpeedKph * 10);			 // Fixed-point: kph * 10
	doc["dasDriveEnabled"] = dasDriveIsEnabled();
	doc["dasSpeedLimitKph"] = (int)dasSpeedLimitKph;
	doc["dasSpeedCapKph"] = (int)dasSpeedCapKph;
	doc["dasSpeedCapMaxKph"] = (int)DAS_SPEED_CAP_MAX_KPH;

	if (s.hasTpms)
	{
		JsonObject tpms = doc["tpms"].to<JsonObject>();
		tpms["fl"] = (int)(s.tpmsPressure[0] * 100); // Pressure in centi-bar
		tpms["fr"] = (int)(s.tpmsPressure[1] * 100);
		tpms["rl"] = (int)(s.tpmsPressure[2] * 100);
		tpms["rr"] = (int)(s.tpmsPressure[3] * 100);
		tpms["tfl"] = s.tpmsTemp[0];
		tpms["tfr"] = s.tpmsTemp[1];
		tpms["trl"] = s.tpmsTemp[2];
		tpms["trr"] = s.tpmsTemp[3];
	}

	if (s.hasPowertrain)
	{
		JsonObject pt = doc["powertrain"].to<JsonObject>();
		pt["speed"] = (int)(s.vehicleSpeed * 100); // Fixed-point: speed * 100
		pt["gear"] = s.gearState;
		pt["pedal"] = s.accelPedal;
		pt["brake"] = s.brakePedalState;
		pt["steer"] = (int)(s.steeringAngle * 10); // Fixed-point: degrees * 10
		pt["rpmR"] = s.rearMotorRpm;
		pt["rpmF"] = s.frontMotorRpm;
	}
	if (s.hasWheelSpeeds)
	{
		JsonObject ws = doc["wheelSpeeds"].to<JsonObject>();
		ws["fl"] = (int)(s.wheelSpeedFL * 100); // Fixed-point: kph * 100
		ws["fr"] = (int)(s.wheelSpeedFR * 100);
		ws["rl"] = (int)(s.wheelSpeedRL * 100);
		ws["rr"] = (int)(s.wheelSpeedRR * 100);
	}
	if (s.hasMotorTemps)
	{
		JsonObject mt = doc["motorTemps"].to<JsonObject>();
		mt["rInv"] = s.rearInvTemp;
		mt["rStat"] = s.rearStatorTemp;
		mt["rHs"] = s.rearHeatsinkTemp;
		mt["fInv"] = s.frontInvTemp;
		mt["fStat"] = s.frontStatorTemp;
		mt["fHs"] = s.frontHeatsinkTemp;
	}

	JsonObject f = doc["features"].to<JsonObject>();
	f["fsd"] = feat.fsd;
	f["profile"] = feat.profile;
	f["nag"] = feat.nag;
	f["offset"] = feat.offset;
	f["isaChime"] = feat.isaChime;
	f["summon"] = feat.summon;

	JsonObject hw = doc["hardware"].to<JsonObject>();
	hw["board"] = BOARD_HW_NAME;
	hw["can"] = BOARD_CAN_NAME;
	hw["busChassis"] = (int)BUS_CHASSIS_ACTIVE;
	hw["busVehicle"] = (int)BUS_VEHICLE_ACTIVE;
	hw["busBody"] = (int)BUS_BODY_ACTIVE;
#if BOARD_ENABLE_BLE
	hw["ble"] = true;
#else
	hw["ble"] = false;
#endif
	hw["wifi"] = true;
	hw["ip"] = wifiCurrentIP();

	// Aggregated subsystem snapshots — replaces legacy per-subsystem status endpoints
	JsonObject rec = doc["recorder"].to<JsonObject>();
	rec["enabled"] = canRecorderEnabled();
	rec["count"] = canRecorderCount();
	rec["capacity"] = canRecorderCapacity();
	rec["captured"] = canRecorderCapturedTotal();
	rec["dropped"] = canRecorderDroppedTotal();
	rec["lastCaptureMs"] = canRecorderLastCaptureMs();

#if BOARD_ENABLE_BLE
	JsonObject ble = doc["ble"].to<JsonObject>();
	ble["enabled"] = bleIsReady();
	ble["connected"] = bleIsConnected();
	ble["deviceName"] = bleGetDeviceName();

	JsonObject gp = doc["gamepad"].to<JsonObject>();
	gp["enabled"] = gpEnabled;
	gp["connected"] = gpConnected;
	gp["scanning"] = gpScanning;
	gp["scanCount"] = gpDeviceCount;
	gp["pairedAddr"] = gpPairedAddr;
	gp["pairedName"] = gamepadLastSeenName();
	gp["rssi"] = (int)gamepadGetRssi();
	gp["battery"] = (int)gamepadGetBattery();
	JsonArray scanArr = gp["devices"].to<JsonArray>();
	for (uint8_t i = 0; i < gpDeviceCount; i++)
	{
		JsonObject d = scanArr.add<JsonObject>();
		d["addr"] = gpDevices[i].addr;
		d["name"] = gpDevices[i].name;
	}
	JsonArray bindArr = gp["bindings"].to<JsonArray>();
	for (uint8_t i = 0; i < GAMEPAD_BTN_COUNT; i++)
	{
		JsonObject b = bindArr.add<JsonObject>();
		b["index"] = i;
		b["button"] = kGpBtnName[i];
		b["command"] = gpBinding[i];
		b["hold"] = gpBindingHold[i];
	}
#endif

	String output;
	serializeJson(doc, output);
	return output;
}

/**
 * @brief Forward declaration for the serial command executor.
 *
 * Defined in serial/board.h; executes a wire command against the application state.
 *
 * @param cmd Null-terminated command string (e.g. "fsd:on", "recorder:clear").
 * @param s Global application state.
 * @param now Current timestamp in milliseconds (from millis()).
 */
void executeCommand(const char *cmd, State &s, unsigned long now);

/**
 * @brief Send standard CORS headers on the current HTTP response.
 *
 * Allows all origins, GET/POST/OPTIONS methods, and Content-Type + X-API-Key headers.
 */
static void handleCors()
{
	server.sendHeader("Access-Control-Allow-Origin", "*");
	server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
	server.sendHeader("Access-Control-Allow-Headers", "Content-Type, X-API-Key");
}

/**
 * @brief Send a JSON response with CORS headers.
 * @param code HTTP status code.
 * @param json Serialized JSON response body.
 */
static void sendJsonResponse(int code, const String &json)
{
	handleCors();
	server.send(code, "application/json", json);
}

/**
 * @brief Handle CORS preflight OPTIONS requests with a 204 No Content response.
 */
static void handleOptions()
{
	handleCors();
	server.send(204);
}

/**
 * @brief Serve the embedded HTML dashboard on the root path.
 */
static void handleRoot()
{
	server.send_P(200, "text/html", DASH_HTML);
}

/**
 * @brief Handle GET /api/status — returns the full application state as JSON.
 */
static void handleGetStatus()
{
	if (!restState)
	{
		sendJsonResponse(500, "{\"error\":\"not initialized\"}");
		return;
	}
	sendJsonResponse(200, buildStateJson(*restState));
}

/**
 * @brief Handle POST /api/command — parse and execute a wire command via REST.
 *
 * Expects a JSON body with a "cmd" field containing the command string.
 * Validates auth, parses JSON, checks command characters, attempts WiFi-specific
 * handling first, then falls back to the general command executor.
 */
static void handlePostCommand()
{
	if (!restState)
	{
		sendJsonResponse(500, "{\"error\":\"not initialized\"}");
		return;
	}
	if (!requireAuth())
		return;

	String body = server.arg("plain");
	if (body.length() == 0)
	{
		sendJsonResponse(400, "{\"error\":\"empty body\"}");
		return;
	}

	JsonDocument doc;
	DeserializationError err = deserializeJson(doc, body);
	if (err)
	{
		sendJsonResponse(400, "{\"error\":\"invalid json\"}");
		return;
	}

	const char *cmd = doc["cmd"];
	if (!cmd || strlen(cmd) == 0 || strlen(cmd) > 31)
	{
		sendJsonResponse(400, "{\"error\":\"missing or invalid cmd\"}");
		return;
	}

	// Whitelist allowed characters to prevent injection via command strings
	for (size_t i = 0; i < strlen(cmd); i++)
	{
		char c = cmd[i];
		bool valid = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == ':' ||
					 c == '-' || c == '_';
		if (!valid)
		{
			sendJsonResponse(400, "{\"error\":\"invalid characters in cmd\"}");
			return;
		}
	}

	// Try WiFi-specific command handler first (e.g. wifi:config, wifi:status)
	if (executeWifiCmd(cmd, doc))
		return;

	executeCommand(cmd, *restState, millis());

	// Return RpcResponse (Ack) — same contract as serial/BLE transports.
	// Clients needing updated state should follow up with GET /api/status.
	char ackJson[72];
	snprintf(ackJson, sizeof(ackJson), "{\"t\":\"ack\",\"cmd\":\"%s\"}", cmd);
	sendJsonResponse(200, String(ackJson));
}

/**
 * @brief Handle GET /api/ping — lightweight health check endpoint.
 */
static void handleGetPing()
{
	sendJsonResponse(200, "{\"t\":\"pong\",\"v\":1}");
}

/**
 * @brief Handle GET /api/disable — emergency kill switch for all active injections.
 *
 * Requires authentication. Disables FSD injection and clears summon remaining count.
 */
static void handleDisable()
{
	if (!restState)
	{
		sendJsonResponse(500, "{\"error\":\"not initialized\"}");
		return;
	}
	if (!requireAuth())
		return;
	restState->fsdEnabled = false;
	restState->summonRemaining = 0;
	sendJsonResponse(200, "{\"ok\":true,\"msg\":\"All injections disabled\"}");
}

/**
 * @brief Handle unmatched routes — returns a 404 JSON error with CORS headers.
 */
static void handleNotFound()
{
	handleCors();
	server.send(404, "application/json", "{\"error\":\"not found\"}");
}
