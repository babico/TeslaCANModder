#pragma once

/**
 * @file firmware/lib/interface/dispatch.h
 * @brief Central command dispatcher that routes string commands to feature handlers
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "messages.h"
#include "core/forward.h"
#include "interface/features.h"
#include "transport/can/recorder.h"

#if BOARD_ENABLE_BLE
#include "interface/ble/tesla_board.h"
#include "interface/ble/config.h"
#endif

/**
 * @brief Parse and execute a command string received from any transport (serial, WiFi, BLE).
 * @param cmd Null-terminated command string to dispatch
 * @param s Device state reference (read and mutated by feature handlers)
 * @param now Current uptime in milliseconds since boot
 */
void executeCommand(const char *cmd, State &s, unsigned long now)
{
	if (strcmp(cmd, "ping") == 0)
	{
		jsonLine().str("t", "pong").num("v", 1).end();
		return;
	}

	if (strcmp(cmd, "status:live:on") == 0)
	{
		statusLiveEnabled = true;
		sendAck(cmd);
		sendLog(F("Status live stream ON"));
		sendStatus(s, now);
		return;
	}

	if (strcmp(cmd, "status:live:off") == 0)
	{
		statusLiveEnabled = false;
		sendAck(cmd);
		sendLog(F("Status live stream OFF"));
		sendStatus(s, now);
		return;
	}

	if (strcmp(cmd, "status:live") == 0)
	{
		jsonLine()
			.str("t", "statusLive")
			.boolean("on", statusLiveEnabled)
			.num("intervalMs", (unsigned long)STATUS_LIVE_INTERVAL_MS)
			.end();
		return;
	}

	if (strcmp(cmd, "status:compact") == 0)
	{
		sendStatusCompact(s, now);
		return;
	}

	if (strcmp(cmd, "status:meta") == 0)
	{
		sendStatusMeta(s, now);
		return;
	}

	if (strcmp(cmd, "status:state") == 0)
	{
		sendStatusState(s);
		return;
	}

	if (strcmp(cmd, "status:features") == 0)
	{
		sendStatusFeatures(s);
		return;
	}

	if (strcmp(cmd, "status:can") == 0)
	{
		sendStatusCan(s);
		return;
	}

	if (strcmp(cmd, "status") == 0)
	{
		sendStatus(s, now);
		return;
	}

	if (strcmp(cmd, "apgate:on") == 0)
	{
		s.apInjectionGateEnabled = true;
		saveSettings(s);
		sendAck(cmd);
		sendLog(F("AP gate ON - writes will require AP/Park/Summon"));
		sendStatus(s, now);
		return;
	}

	if (strcmp(cmd, "apgate:off") == 0)
	{
		s.apInjectionGateEnabled = false;
		saveSettings(s);
		sendAck(cmd);
		sendLog(F("AP gate OFF - writes allowed without gate"));
		sendStatus(s, now);
		return;
	}

	if (strcmp(cmd, "apgate:status") == 0)
	{
		jsonLine()
			.str("t", "apgate")
			.boolean("enabled", s.apInjectionGateEnabled)
			.boolean("ap", s.apGateApActive)
			.boolean("park", s.apGateParked)
			.boolean("summon", s.apGateSummoning)
			.boolean("open", s.apGateOpen())
			.end();
		return;
	}

	if (executeStreamCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(s.streamEnabled ? F("Stream started") : F("Stream stopped"));
		sendStatus(s, now);
		return;
	}

	if (executeCanRawCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(s.rawCanListen ? F("Raw CAN mode enabled") : F("Filtered CAN mode"));
		sendStatus(s, now);
		return;
	}

	if (executeCanClockCmd(cmd, s))
	{
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

	if (executeFsdCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(s.fsdEnabled ? F("FSD enabled - saved to NVS") : F("FSD disabled - saved to NVS"));
		sendStatus(s, now);
		return;
	}

	if (strcmp(cmd, "drive:status") == 0)
	{
		jsonLine()
			.str("t", "drive")
			.boolean("enabled", dasDriveIsEnabled())
			.boolean("active", dasDriveIsActive())
			.num("steerDeg", (int)dasSteerAngle)
			.num("accelMinX100", (int)(dasAccelMin * 100.0f))
			.num("accelMaxX100", (int)(dasAccelMax * 100.0f))
			.num("speedKph", (int)dasSetSpeedKph)
			.num("speedLimitKph", (int)dasSpeedLimitKph)
			.num("speedCapKph", (int)dasSpeedCapKph)
			.num("speedCapMaxKph", (int)DAS_SPEED_CAP_MAX_KPH)
			.end();
		return;
	}

	if (executeDasCmd(cmd, s))
	{
		sendAck(cmd);
		if (strcmp(cmd, "drive:on") == 0)
			sendLog(F("Drive mode ON - saved"));
		else if (strcmp(cmd, "drive:off") == 0)
			sendLog(F("Drive mode OFF - saved"));
		else if (strncmp(cmd, "drive:speed:", 12) == 0)
			sendLog(F("Drive speed limit saved"));
		else if (strncmp(cmd, "drive:cap:", 10) == 0)
			sendLog(F("Drive speed CAP saved"));
		sendStatus(s, now);
		return;
	}

	if (executeFsdForceCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(s.fsdForceEnabled ? F("FSD Force ON - saved to NVS") : F("FSD Force OFF - saved to NVS"));
		sendStatus(s, now);
		return;
	}

	if (executeNagCmd(cmd, s))
	{
		sendAck(cmd);
		if (strncmp(cmd, "nag:mode:", 9) == 0)
			sendLog(F("Nag mode updated - saved"));
		else if (strncmp(cmd, "nag:bypass:", 11) == 0)
			sendLog(s.nagOrganicDriverBypass ? F("Nag organic driver bypass ON - saved")
											 : F("Nag organic driver bypass OFF - saved"));
		sendStatus(s, now);
		return;
	}

	if (executeProfileCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(s.profileOverride ? F("Profile pinned - saved") : F("Profile set to auto"));
		sendStatus(s, now);
		return;
	}

	if (executeOffsetCmd(cmd, s))
	{
		sendAck(cmd);
		if (s.detectedHW == 3 || s.variant == HW4)
			sendLog(s.speedOffset > 0 ? F("Offset updated - saved") : F("Offset disabled - saved"));
		else
			sendLog(s.offsetOverride ? F("Offset pinned - saved") : F("Offset set to auto"));
		sendStatus(s, now);
		return;
	}

	if (executeIsaChimeCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(s.isaChimeSuppress ? F("ISA chime suppressed - saved") : F("ISA chime original - saved"));
		sendStatus(s, now);
		return;
	}

	if (executeBanShieldCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(s.banShieldEnabled ? F("Ban Shield ON - telemetry monitoring active") : F("Ban Shield OFF"));
		sendStatus(s, now);
		return;
	}

	if (executeGtwShieldCmd(cmd, s))
	{
		sendAck(cmd);
		if (s.gtwShieldArmed)
			sendLog(F("GTW shield ARMED - blocking 0x7FF changes"));
		else
			sendLog(F("GTW shield disarmed"));
		sendStatus(s, now);
		return;
	}

	if (executeTlsscCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(s.tlsscRestore ? F("TLSSC restore ON - spoofing SELF_DRIVING tier") : F("TLSSC restore OFF"));
		sendStatus(s, now);
		return;
	}

	if (executeSummonInjectCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(s.summonInject ? F("Summon injection ON - saved") : F("Summon injection OFF - saved"));
		sendStatus(s, now);
		return;
	}

	if (executeSummonCmd(cmd, s))
	{
		sendAck(cmd);
		if (s.summonRemaining > 0)
			sendLog(F("Summon burst started (30 frames)"));
		else
			sendLog(F("Summon stopped"));
		sendStatus(s, now);
		return;
	}

	if (executeVariantCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(F("Variant changed - filters updated"));
		sendStatus(s, now);
		return;
	}

	if (executeAlcCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(s.alcAutoConfirmEnabled ? F("ALC auto-confirm ON") : F("ALC auto-confirm OFF"));
		sendStatus(s, now);
		return;
	}
	if (executeRegionSpoofCmd(cmd, s))
	{
		sendAck(cmd);
		if (s.regionSpoofCode == 0)
		{
			sendLog(F("Region spoof OFF"));
		}
		else
		{
			sendLog(F("Region spoofed - saved"));
		}
		sendStatus(s, now);
		return;
	}
	if (executeBmsCmd(cmd, s))
	{
		auto bmsCore = [&](JsonLineBuilder &out)
		{
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

		auto bmsExtended = [&](JsonLineBuilder &out)
		{
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
	if (executeWindowCmd(cmd, s) || executeSentryCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(cmd);
		sendStatus(s, now);
		return;
	}

	// Vehicle bus commands (climate, charge, drive, precondition, track mode)
	if (executePreconditionCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(s.preconditionEnabled ? F("Precondition ON - saved") : F("Precondition OFF - saved"));
		sendStatus(s, now);
		return;
	}

	if (executeTrackModeCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(s.trackModeEnabled ? F("Track mode ON - saved") : F("Track mode OFF - saved"));
		sendStatus(s, now);
		return;
	}

	if (executeClimateCmd(cmd, s) || executeChargeCmd(cmd, s) || executePedalCmd(cmd, s) || executeRegenCmd(cmd, s) ||
		executeStopCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(cmd);
		sendStatus(s, now);
		return;
	}

	// Commands below require the 0x273 control frame to be present
	if (!s.hasCtrl)
	{
		sendError(F("Waiting for 0x273 frame"));
		return;
	}

	// Non-legacy variant vehicle controls (mirror, lock, light, wiper, seat, display, power)
	if (s.variant != LEGACY &&
		(executeMirrorCmd(cmd, s) || executeLockCmd(cmd, s) || executeLightCmd(cmd, s) || executeWiperCmd(cmd, s) ||
		 executeSeatCmd(cmd, s) || executeDisplayCmd(cmd, s) || executePowerCmd(cmd, s)))
	{
		sendAck(cmd);
		sendLog(cmd);
		sendStatus(s, now);
		return;
	}

	// Trunk commands (frunk = vehicle bus, trunk/glovebox = body bus)
	if (s.variant != LEGACY && executeTrunkCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(cmd);
		sendStatus(s, now);
		return;
	}

	// TPMS pressure and temperature query
	if (executeTpmsCmd(cmd, s))
	{
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

	// Drive mode override: drivemode:off, drivemode:chill, drivemode:standard, drivemode:performance
	if (executeDriveModeCmd(cmd, s))
	{
		sendAck(cmd);
		saveSettings(s);
		sendLog(s.driveModeOverride == 0 ? F("Drive mode override OFF - saved")
										 : F("Drive mode override active - saved"));
		sendStatus(s, now);
		return;
	}

	// ECE R79 steering torque bypass toggle
	if (strcmp(cmd, "ecer79:on") == 0)
	{
		s.eceR79Bypass = true;
		saveSettings(s);
		sendAck(cmd);
		sendLog(F("ECE R79 bypass ON - saved"));
		sendStatus(s, now);
		return;
	}
	if (strcmp(cmd, "ecer79:off") == 0)
	{
		s.eceR79Bypass = false;
		saveSettings(s);
		sendAck(cmd);
		sendLog(F("ECE R79 bypass OFF - saved"));
		sendStatus(s, now);
		return;
	}

	// LHD mode: clears UI_drivingSide bit 41 on 0x3F8 (frame 1016)
	if (strcmp(cmd, "lhd:on") == 0)
	{
		s.lhdEnabled = true;
		saveSettings(s);
		sendAck(cmd);
		sendLog(F("LHD mode ON - saved (BETA, untested)"));
		sendStatus(s, now);
		return;
	}
	if (strcmp(cmd, "lhd:off") == 0)
	{
		s.lhdEnabled = false;
		saveSettings(s);
		sendAck(cmd);
		sendLog(F("LHD mode OFF - saved"));
		sendStatus(s, now);
		return;
	}

	// Assist nav-enable: sets bits 13+48+49 on 0x3F8 for nav-based FSD routing
	if (strcmp(cmd, "assist-nav:on") == 0)
	{
		s.assistNavEnable = true;
		saveSettings(s);
		sendAck(cmd);
		sendLog(F("Assist nav ON - nav-based FSD routing enabled (BETA)"));
		sendStatus(s, now);
		return;
	}
	if (strcmp(cmd, "assist-nav:off") == 0)
	{
		s.assistNavEnable = false;
		saveSettings(s);
		sendAck(cmd);
		sendLog(F("Assist nav OFF - saved"));
		sendStatus(s, now);
		return;
	}

	// Assist hands-off: sets bit 14 on 0x3F8 (UI_handsOnRequirementDisable)
	if (strcmp(cmd, "assist-hof:on") == 0)
	{
		s.assistHandsOff = true;
		saveSettings(s);
		sendAck(cmd);
		sendLog(F("Assist hands-off ON - hands-on requirement disabled (BETA)"));
		sendStatus(s, now);
		return;
	}
	if (strcmp(cmd, "assist-hof:off") == 0)
	{
		s.assistHandsOff = false;
		saveSettings(s);
		sendAck(cmd);
		sendLog(F("Assist hands-off OFF - saved"));
		sendStatus(s, now);
		return;
	}

	// Assist dev-mode: sets bit 5 on 0x3F8 (UI_dasDeveloper)
	if (strcmp(cmd, "assist-dev:on") == 0)
	{
		s.assistDevMode = true;
		saveSettings(s);
		sendAck(cmd);
		sendLog(F("Assist dev-mode ON - UI_dasDeveloper bit set (BETA)"));
		sendStatus(s, now);
		return;
	}
	if (strcmp(cmd, "assist-dev:off") == 0)
	{
		s.assistDevMode = false;
		saveSettings(s);
		sendAck(cmd);
		sendLog(F("Assist dev-mode OFF - saved"));
		sendStatus(s, now);
		return;
	}

	// Lane graph visualization: sets bit 45 on 0x3FD mux1
	if (strcmp(cmd, "lane-graph:on") == 0)
	{
		s.laneGraphEnable = true;
		saveSettings(s);
		sendAck(cmd);
		sendLog(F("Lane graph ON - saved (BETA)"));
		sendStatus(s, now);
		return;
	}
	if (strcmp(cmd, "lane-graph:off") == 0)
	{
		s.laneGraphEnable = false;
		saveSettings(s);
		sendAck(cmd);
		sendLog(F("Lane graph OFF - saved"));
		sendStatus(s, now);
		return;
	}

	// Assist telemetry-off: clears bit 43 on 0x3F8 (UI_enableTripTelemetry = 0)
	if (strcmp(cmd, "assist-tel:on") == 0)
	{
		s.assistTelemetryOff = true;
		saveSettings(s);
		sendAck(cmd);
		sendLog(F("Assist telemetry-off ON - trip data collection disabled (BETA)"));
		sendStatus(s, now);
		return;
	}
	if (strcmp(cmd, "assist-tel:off") == 0)
	{
		s.assistTelemetryOff = false;
		saveSettings(s);
		sendAck(cmd);
		sendLog(F("Assist telemetry-off OFF - trip telemetry restored"));
		sendStatus(s, now);
		return;
	}

	// AP-First mode: delays 0x3FD injection until DAS_autopilotState >= 2 (2026.14.x compat)
	if (strcmp(cmd, "ap-first:on") == 0)
	{
		s.apFirstEnabled = true;
		saveSettings(s);
		sendAck(cmd);
		sendLog(F("AP-First ON - injection delayed until AP is active (2026.14.x compat)"));
		sendStatus(s, now);
		return;
	}
	if (strcmp(cmd, "ap-first:off") == 0)
	{
		s.apFirstEnabled = false;
		saveSettings(s);
		sendAck(cmd);
		sendLog(F("AP-First OFF - injection starts immediately"));
		sendStatus(s, now);
		return;
	}

	// Enhanced Autopilot: sets bit46 on mux=1 to unlock EAP/Summon features
	if (strcmp(cmd, "eap:on") == 0)
	{
		s.enhancedAutopilot = true;
		saveSettings(s);
		sendAck(cmd);
		sendLog(F("Enhanced Autopilot ON - EAP/Summon unlocked"));
		sendStatus(s, now);
		return;
	}
	if (strcmp(cmd, "eap:off") == 0)
	{
		s.enhancedAutopilot = false;
		saveSettings(s);
		sendAck(cmd);
		sendLog(F("Enhanced Autopilot OFF - saved"));
		sendStatus(s, now);
		return;
	}

	// Emergency Vehicle Detection: bit59 on mux=0 (HW4 only)
	if (strcmp(cmd, "evd:on") == 0)
	{
		s.evdEnabled = true;
		saveSettings(s);
		sendAck(cmd);
		sendLog(F("EVD ON - emergency vehicle detection bit enabled (HW4)"));
		sendStatus(s, now);
		return;
	}
	if (strcmp(cmd, "evd:off") == 0)
	{
		s.evdEnabled = false;
		saveSettings(s);
		sendAck(cmd);
		sendLog(F("EVD OFF - saved"));
		sendStatus(s, now);
		return;
	}

	// Log ring dump: emit last N log entries as a JSON array
	if (strcmp(cmd, "log") == 0)
	{
		uint16_t count = logRingCount();
		JsonLineBuilder line = jsonLine();
		line.str("t", "log").num("count", count).raw(F(",\"entries\":["));
		for (uint16_t i = 0; i < count; i++)
		{
			if (i > 0)
				printStr(F(","));
			const LogEntry *e = logRingGet(i);
			if (!e)
				break;
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
	if (executeTurnSignalCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(cmd);
		sendStatus(s, now);
		return;
	}

	// Seatbelt emulation toggle
	if (executeSeatbeltCmd(cmd, s))
	{
		sendAck(cmd);
		saveSettings(s);
		sendLog(s.seatbeltEmulation ? F("Seatbelt emulation ON - saved") : F("Seatbelt emulation OFF - saved"));
		sendStatus(s, now);
		return;
	}

	// Air recirculation toggle
	if (executeAirRecircCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(cmd);
		sendStatus(s, now);
		return;
	}

	// Wiper persistence toggle (maintains wiper state across drive cycles)
	if (executeWiperPersistCmd(cmd, s))
	{
		sendAck(cmd);
		saveSettings(s);
		sendLog(s.wiperPersistEnabled ? F("Wiper persist ON - saved") : F("Wiper persist OFF - saved"));
		sendStatus(s, now);
		return;
	}

	// Mirror auto-fold toggle
	if (executeMirrorAutoFoldCmd(cmd, s))
	{
		sendAck(cmd);
		saveSettings(s);
		sendLog(s.mirrorAutoFoldEnabled ? F("Mirror auto-fold ON - saved") : F("Mirror auto-fold OFF - saved"));
		sendStatus(s, now);
		return;
	}

	// Powertrain telemetry query (speed, gear, motor temps, wheel speeds)
	if (executePowertrainCmd(cmd, s))
	{
		jsonLine()
			.str("t", "powertrain")
			.num("speed", (long)(s.vehicleSpeed * 100))
			.num("gear", s.gearState)
			.num("pedal", s.accelPedal)
			.num("brake", s.brakePedalState)
			.num("steer", (long)(s.steeringAngle * 10))
			.num("rpmR", s.rearMotorRpm)
			.num("rpmF", s.frontMotorRpm)
			.num("wsFL", (long)(s.wheelSpeedFL * 100))
			.num("wsFR", (long)(s.wheelSpeedFR * 100))
			.num("wsRL", (long)(s.wheelSpeedRL * 100))
			.num("wsRR", (long)(s.wheelSpeedRR * 100))
			.boolean("hasWs", s.hasWheelSpeeds)
			.num("rInvT", s.rearInvTemp)
			.num("rStatT", s.rearStatorTemp)
			.num("rHsT", s.rearHeatsinkTemp)
			.num("fInvT", s.frontInvTemp)
			.num("fStatT", s.frontStatorTemp)
			.num("fHsT", s.frontHeatsinkTemp)
			.boolean("hasMotorT", s.hasMotorTemps)
			.boolean("ok", s.hasPowertrain)
			.end();
		return;
	}

	// CAN simulation: simu:start, simu:stop
	if (executeCanSimCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(s.canSimEnabled ? F("CAN simulation started") : F("CAN simulation stopped"));
		sendStatus(s, now);
		return;
	}

	// Single-shot TX mode (disables automatic retransmission on MCP2515)
	if (executeSingleShotCmd(cmd, s))
	{
		sendAck(cmd);
		saveSettings(s);
		driverSetSingleShot(s.singleShotTx);
		sendLog(s.singleShotTx ? F("Single-shot TX ON - saved") : F("Single-shot TX OFF - saved"));
		sendStatus(s, now);
		return;
	}

	// Firmware version compatibility query
	if (executeFwCompatCmd(cmd, s))
	{
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

	// MQTT bridge configuration: mqtt:on/off, mqtt:broker:<host>, mqtt:port:<port>, mqtt:interval:<ms>
	if (executeMqttCmd(cmd, s))
	{
		sendAck(cmd);
		saveSettings(s);
		sendLog(s.mqttEnabled ? F("MQTT updated - saved") : F("MQTT OFF - saved"));
		sendStatus(s, now);
		return;
	}

	// Vehicle config query (model, year)
	if (executeVehicleConfigCmd(cmd, s))
	{
		jsonLine()
			.str("t", "vehicle")
			.num("model", s.vehicleModel)
			.num("year", s.vehicleYear)
			.boolean("ok", s.hasVehicleConfig)
			.end();
		return;
	}

	// Vehicle platform identity query (hardware gen, software version, FSD protocol)
	if (strcmp(cmd, "platform") == 0)
	{
		auto platformRoot = [&](JsonLineBuilder &out)
		{
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
		auto canHealthObject = [&](JsonLineBuilder::JsonObjectBuilder &obj)
		{
			for (uint8_t i = 0; i < BUS_MAX; i++)
			{
				char busKey[8];
				busKey[0] = 'b';
				busKey[1] = 'u';
				busKey[2] = 's';
				busKey[3] = (char)('0' + i);
				busKey[4] = '\0';

				obj.object(busKey, [&](JsonLineBuilder::JsonObjectBuilder &bus)
						   { bus.boolean("on", busActive(i)).boolean("det", mcpAvailable[i]); });
			}
		};

		jsonLine().str("t", "platform").merge(platformRoot).mergeObject("canHealth", canHealthObject).end();
		return;
	}

	// CAN Recorder: recorder:on, recorder:off, recorder:clear, recorder:status
	if (strncmp(cmd, "recorder:", 9) == 0)
	{
		const char *sub = cmd + 9;
		if (strcmp(sub, "on") == 0)
		{
			canRecorderStart(true);
			sendAck(cmd);
			sendLog(F("Recorder ON"));
			return;
		}
		if (strcmp(sub, "off") == 0)
		{
			canRecorderStop();
			sendAck(cmd);
			sendLog(F("Recorder OFF"));
			return;
		}
		if (strcmp(sub, "clear") == 0)
		{
			canRecorderReset();
			sendAck(cmd);
			sendLog(F("Recorder cleared"));
			return;
		}
		if (strcmp(sub, "status") == 0)
		{
			jsonLine()
				.str("t", "recorder")
				.boolean("enabled", canRecorderEnabled())
				.num("count", (int)canRecorderCount())
				.num("capacity", (int)canRecorderCapacity())
				.num("captured", (unsigned long)canRecorderCapturedTotal())
				.num("dropped", (unsigned long)canRecorderDroppedTotal())
				.num("lastCaptureMs", (unsigned long)canRecorderLastCaptureMs())
				.end();
			return;
		}
		sendError(F("Unknown recorder sub-command"));
		return;
	}

#if BOARD_ENABLE_BLE
	// BLE radio control: ble:on, ble:off, ble:name:<n>, ble:status
	if (strncmp(cmd, "ble:", 4) == 0 && strncmp(cmd, "ble:scan", 8) != 0)
	{
		const char *sub = cmd + 4;
		if (strcmp(sub, "on") == 0)
		{
			bleEnabledCfg = true;
			if (!bleIsReady())
				bleRestart();
			saveBleConfig();
			sendAck(cmd);
			sendLog(F("BLE radio ON - saved"));
			sendStatus(s, now);
			return;
		}
		if (strcmp(sub, "off") == 0)
		{
			bleEnabledCfg = false;
			if (bleIsReady())
				bleStop();
			saveBleConfig();
			sendAck(cmd);
			sendLog(F("BLE radio OFF - saved"));
			sendStatus(s, now);
			return;
		}
		if (strcmp(sub, "status") == 0)
		{
			jsonLine()
				.str("t", "ble")
				.boolean("enabled", bleIsReady())
				.boolean("connected", bleIsConnected())
				.str("deviceName", bleGetDeviceName())
				.end();
			return;
		}
		if (strncmp(sub, "name:", 5) == 0)
		{
			const char *name = sub + 5;
			size_t len = strlen(name);
			if (len == 0 || len > 32)
			{
				sendError(F("BLE name must be 1-32 chars"));
				return;
			}
			strncpy(bleNameCfg, name, sizeof(bleNameCfg) - 1);
			bleNameCfg[sizeof(bleNameCfg) - 1] = '\0'; // ensure null termination
			if (!bleSetDeviceName(bleNameCfg))
			{
				sendError(F("Failed to apply BLE name"));
				return;
			}
			saveBleConfig();
			sendAck(cmd);
			sendLog(F("BLE name updated - saved"));
			return;
		}
		sendError(F("Unknown ble sub-command"));
		return;
	}

	// Gamepad commands: scan, pair, unpair, on/off, status, bind, hold, axis, cancel
	if (strncmp(cmd, "gamepad:", 8) == 0)
	{
		const char *sub = cmd + 8;
		if (strcmp(sub, "scan") == 0 || strcmp(sub, "rescan") == 0)
		{
			gamepadStartScan();
			sendAck(cmd);
			return;
		}
		if (strcmp(sub, "unpair") == 0)
		{
			gamepadUnpair();
			sendAck(cmd);
			sendLog(F("Gamepad unpaired"));
			sendStatus(s, now);
			return;
		}
		if (strcmp(sub, "on") == 0)
		{
			gamepadSetEnabled(true);
			sendAck(cmd);
			sendLog(F("Gamepad enabled - saved"));
			sendStatus(s, now);
			return;
		}
		if (strcmp(sub, "off") == 0)
		{
			gamepadSetEnabled(false);
			sendAck(cmd);
			sendLog(F("Gamepad disabled - saved"));
			sendStatus(s, now);
			return;
		}
		if (strcmp(sub, "cancel") == 0)
		{
			gamepadCancel();
			sendAck(cmd);
			sendLog(F("Gamepad: DAS cancel burst sent"));
			return;
		}
		if (strcmp(sub, "status") == 0)
		{
			jsonLine()
				.str("t", "gamepad")
				.boolean("enabled", gpEnabled)
				.boolean("connected", gpConnected)
				.boolean("scanning", gpScanning)
				.str("addr", gpPairedAddr)
				.str("name", gamepadLastSeenName())
				.num("rssi", gamepadGetRssi())
				.num("battery", (int)gamepadGetBattery())
				.num("reconnFails", (int)gamepadReconnectFails())
				.num("buttons", (int)gpButtons)
				.end();
			return;
		}
		if (strncmp(sub, "pair:", 5) == 0)
		{
			if (gamepadSetPaired(sub + 5))
			{
				sendAck(cmd);
				sendLog(F("Gamepad paired - saved"));
			}
			else
			{
				sendError(F("Invalid address — use AA:BB:CC:DD:EE:FF"));
			}
			return;
		}
		if (strncmp(sub, "bind:", 5) == 0 || strncmp(sub, "hold:", 5) == 0)
		{
			bool isHold = (sub[0] == 'h');
			const char *rest = sub + 5;
			const char *colon = strchr(rest, ':');
			if (!colon)
			{
				sendError(F("Usage: gamepad:bind|hold:<n>:<cmd>"));
				return;
			}
			int idx = (int)strtol(rest, nullptr, 10);
			if (idx < 0 || idx >= GAMEPAD_BTN_COUNT)
			{
				sendError(F("Button index 0-15"));
				return;
			}
			if (isHold)
				gamepadSetBindingHold(idx, colon + 1);
			else
				gamepadSetBinding(idx, colon + 1);
			sendAck(cmd);
			return;
		}
		// Per-axis tuning: gamepad:axis:<n>:<dz|expo|inv>:<value>
		if (strncmp(sub, "axis:", 5) == 0)
		{
			const char *p = sub + 5;
			char *end;
			long n = strtol(p, &end, 10);
			if (n < 0 || n > 5 || *end != ':')
			{
				sendError(F("Usage: gamepad:axis:<0-5>:dz|expo|inv:<v>"));
				return;
			}
			const char *field = end + 1;
			const char *colon = strchr(field, ':');
			if (!colon)
			{
				sendError(F("Usage: gamepad:axis:<n>:dz|expo|inv:<v>"));
				return;
			}
			long v = strtol(colon + 1, nullptr, 10);
			uint8_t idx = (uint8_t)n;
			uint8_t dz = gpAxisDz[idx], expo = gpAxisExpo[idx];
			bool inv = (gpAxisInvMask & (1u << idx)) != 0;
			if (strncmp(field, "dz:", 3) == 0)
				dz = (uint8_t)(v < 0 ? 0 : (v > 50 ? 50 : v));
			else if (strncmp(field, "expo:", 5) == 0)
				expo = (uint8_t)(v < 0 ? 0 : (v > 100 ? 100 : v));
			else if (strncmp(field, "inv:", 4) == 0)
				inv = (v != 0);
			else
			{
				sendError(F("axis field must be dz|expo|inv"));
				return;
			}
			gamepadSetAxisTune(idx, dz, expo, inv);
			sendAck(cmd);
			return;
		}
		sendError(F("Unknown gamepad sub-command"));
		return;
	}

	// Tesla BLE protocol commands (delegated to Tesla:: namespace)
	if (strncmp(cmd, "tesla:", 6) == 0)
	{
		Tesla::executeTeslaCommand(cmd + 6);
		return;
	}
#endif

	sendError(F("Unknown command"));
}
