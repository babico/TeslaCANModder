#pragma once
#include "messages.h"

#if BOARD_ENABLE_BLE
#include "tesla/ble/esp32/tesla.h"
#endif

// ── Command Parser ───────────────────────────────────────────────────────────
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
		sendLog(s.nagSuppress ? F("Nag suppress ON - saved") : F("Nag suppress OFF - saved"));
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

	if (executeNagKillerCmd(cmd, s))
	{
		sendAck(cmd);
		if (strncmp(cmd, "nag:killer:mode:", 16) == 0)
		{
			sendLog(F("Nag killer mode updated - saved"));
		}
		else
		{
			sendLog(s.nagKillerEnabled ? F("Nag killer ON - saved") : F("Nag killer OFF - saved"));
		}
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
	if (execBmsCmd(cmd, s))
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
#if BUS_BODY_ACTIVE
	if (execWindowCmd(cmd, s) || execSentryCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(cmd);
		sendStatus(s, now);
		return;
	}
#endif

	// Vehicle bus commands (climate, charge, drive, precondition, track mode)
#if BUS_VEHICLE_ACTIVE
	if (execPreconditionCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(s.preconditionEnabled ? F("Precondition ON - saved") : F("Precondition OFF - saved"));
		sendStatus(s, now);
		return;
	}

	if (execTrackModeCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(s.trackModeEnabled ? F("Track mode ON - saved") : F("Track mode OFF - saved"));
		sendStatus(s, now);
		return;
	}

	if (execClimateCmd(cmd, s) || execChargeCmd(cmd, s) || execPedalCmd(cmd, s) || execRegenCmd(cmd, s) ||
		execStopCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(cmd);
		sendStatus(s, now);
		return;
	}

	if (!s.hasCtrl)
	{
		sendError(F("Waiting for 0x273 frame"));
		return;
	}

	if (s.variant != LEGACY &&
		(execMirrorCmd(cmd, s) || execLockCmd(cmd, s) || execLightCmd(cmd, s) || execWiperCmd(cmd, s) ||
		 execSeatCmd(cmd, s) || execDisplayCmd(cmd, s) || execPowerCmd(cmd, s)))
	{
		sendAck(cmd);
		sendLog(cmd);
		sendStatus(s, now);
		return;
	}
#endif

	// Trunk commands (frunk = vehicle bus, trunk/glovebox = body bus)
#if BUS_VEHICLE_ACTIVE || BUS_BODY_ACTIVE
	if (s.variant != LEGACY && execTrunkCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(cmd);
		sendStatus(s, now);
		return;
	}
#endif

	// TPMS query
	if (execTpmsCmd(cmd, s))
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

	// Drive mode override commands: drivemode:off, drivemode:chill, drivemode:standard, drivemode:performance
	if (executeDriveModeCmd(cmd, s))
	{
		sendAck(cmd);
		saveSettings(s);
		sendLog(s.driveModeOverride == 0 ? F("Drive mode override OFF - saved")
										 : F("Drive mode override active - saved"));
		sendStatus(s, now);
		return;
	}

	// ECE R79 bypass toggle: ecer79:on / ecer79:off
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

	// LHD (Left-Hand Drive) mode: lhd:on / lhd:off
	// Clears UI_drivingSide bit 41 on 0x3F8 (frame 1016) — source: ev-open-can-tools-plugins Beta/LHD.json
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

	// Assist nav-enable: assist-nav:on / assist-nav:off
	// Sets bits 13+48+49 on 0x3F8 (UI_driveOnMapsEnable, UI_hasDriveOnNav, UI_followNavRouteEnable)
	// Source: hypery11/flipper-tesla-fsd fsd_handler.c (assist_nav_enable)
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

	// Assist hands-off: assist-hof:on / assist-hof:off
	// Sets bit 14 on 0x3F8 (UI_handsOnRequirementDisable)
	// Source: hypery11/flipper-tesla-fsd fsd_handler.c (assist_hands_off)
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

	// Assist dev-mode: assist-dev:on / assist-dev:off
	// Sets bit 5 on 0x3F8 (UI_dasDeveloper)
	// Source: hypery11/flipper-tesla-fsd fsd_handler.c (assist_dev_mode)
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

	// Lane graph visualization: lane-graph:on / lane-graph:off
	// Sets bit 45 on 0x3FD mux1 (lane visualization on non-FSD tier)
	// Source: hypery11/flipper-tesla-fsd fsd_handler.c (assist_show_lane_graph)
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

	// Assist telemetry-off: assist-tel:on / assist-tel:off
	// Clears bit 43 on 0x3F8 (UI_enableTripTelemetry = 0 = trip data off)
	// Source: hypery11/flipper-tesla-fsd fsd_handler.c (assist_telemetry_off)
	// Note: "assist-tel:on" enables the toggle (telemetry disabled); "assist-tel:off" restores default
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

	// AP-First mode: ap-first:on / ap-first:off
	// Delays 0x3FD injection until DAS_autopilotState >= 2 (AP/TACC already running).
	// Required for Tesla 2026.14.x which blocks AP engagement if injection is already active.
	// Source: hypery11/flipper-tesla-fsd v2.14.0 — AP-First mode (confirmed ev-open-can-tools #43)
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

	// Enhanced Autopilot: eap:on / eap:off
	// Sets bit46 on mux=1 to unlock EAP/Summon features
	// Source: ev-open-can-tools + hypery11 enhanced_autopilot
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

	// Emergency Vehicle Detection: evd:on / evd:off (HW4 only, bit59 on mux=0)
	// Source: hypery11/flipper-tesla-fsd fsd_handler.c
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

	// Log ring dump: show last N log entries
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
#if BUS_VEHICLE_ACTIVE
	if (execTurnSignalCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(cmd);
		sendStatus(s, now);
		return;
	}
#endif

	// Seatbelt emulation: seatbelt:on, seatbelt:off
#if BUS_VEHICLE_ACTIVE
	if (execSeatbeltCmd(cmd, s))
	{
		sendAck(cmd);
		saveSettings(s);
		sendLog(s.seatbeltEmulation ? F("Seatbelt emulation ON - saved") : F("Seatbelt emulation OFF - saved"));
		sendStatus(s, now);
		return;
	}
#endif

	// Air recirculation: airecirc:on, airecirc:off
#if BUS_VEHICLE_ACTIVE
	if (execAirRecircCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(cmd);
		sendStatus(s, now);
		return;
	}
#endif

	// Wiper persistence: wiperpersist:on, wiperpersist:off
#if BUS_VEHICLE_ACTIVE
	if (execWiperPersistCmd(cmd, s))
	{
		sendAck(cmd);
		saveSettings(s);
		sendLog(s.wiperPersistEnabled ? F("Wiper persist ON - saved") : F("Wiper persist OFF - saved"));
		sendStatus(s, now);
		return;
	}
#endif

	// Mirror auto-fold: mirror:autofold:on, mirror:autofold:off
#if BUS_VEHICLE_ACTIVE
	if (execMirrorAutoFoldCmd(cmd, s))
	{
		sendAck(cmd);
		saveSettings(s);
		sendLog(s.mirrorAutoFoldEnabled ? F("Mirror auto-fold ON - saved") : F("Mirror auto-fold OFF - saved"));
		sendStatus(s, now);
		return;
	}
#endif

	// Powertrain telemetry query
#if BUS_VEHICLE_ACTIVE
	if (execPowertrainCmd(cmd, s))
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
#endif

	// CAN simulation: simu:start, simu:stop
	if (execCanSimCmd(cmd, s))
	{
		sendAck(cmd);
		sendLog(s.canSimEnabled ? F("CAN simulation started") : F("CAN simulation stopped"));
		sendStatus(s, now);
		return;
	}

	// Single-shot TX mode: singleshot:on, singleshot:off
	if (execSingleShotCmd(cmd, s))
	{
		sendAck(cmd);
		saveSettings(s);
		driverSetSingleShot(s.singleShotTx);
		sendLog(s.singleShotTx ? F("Single-shot TX ON - saved") : F("Single-shot TX OFF - saved"));
		sendStatus(s, now);
		return;
	}

	// Firmware version compatibility: fwcompat
	if (execFwCompatCmd(cmd, s))
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

#if BUS_VEHICLE_ACTIVE
	// MQTT bridge: mqtt:on/off, mqtt:broker:<host>, mqtt:port:<port>, mqtt:interval:<ms>
	if (execMqttCmd(cmd, s))
	{
		sendAck(cmd);
		saveSettings(s);
		sendLog(s.mqttEnabled ? F("MQTT updated - saved") : F("MQTT OFF - saved"));
		sendStatus(s, now);
		return;
	}
#endif

	// Vehicle config query: vehicle
	if (execVehicleConfigCmd(cmd, s))
	{
		jsonLine()
			.str("t", "vehicle")
			.num("model", s.vehicleModel)
			.num("year", s.vehicleYear)
			.boolean("ok", s.hasVehicleConfig)
			.end();
		return;
	}

	// Vehicle platform identity query: platform
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


#if BOARD_ENABLE_BLE
	if (strncmp(cmd, "tesla:", 6) == 0) {
		Tesla::executeTeslaCommand(cmd + 6);
		return;
	}
#endif

	sendError(F("Unknown command"));
}
