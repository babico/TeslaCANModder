/**
 * Shared board-state reducer for web and mobile.
 * Pure function: takes previous state + firmware message → new state.
 */

import type {
	BoardState,
	BootMessage,
	StatusMessage,
	PlatformMessage,
	StatusMetaMessage,
	StatusFeaturesMessage,
	StatusCanMessage,
	StatusStateMessage,
	StatusCompactMessage,
	FrameMessage,
	BmsMessage,
	BoardMessage,
	ConsoleMessage,
	CanFrame,
} from "./types.js";

// ── Constants ───────────────────────────────────────────────────────────────

export const BUS_NAMES = ["Chassis", "Vehicle", "Body"] as const;

// ── Helpers ─────────────────────────────────────────────────────────────────

export function detectBoard(hw: string): "arduino" | "esp32" | "unknown" {
	const lower = String(hw ?? "").toLowerCase();
	if (lower.includes("arduino") || lower.includes("uno")) return "arduino";
	if (lower.includes("esp32") || lower.includes("esp")) return "esp32";
	return "unknown";
}

/** Prepend a notification to state.messages (capped at 100). */
export function addNotification(
	state: BoardState,
	type: ConsoleMessage["type"],
	text: string,
	id: number,
	ts?: string,
): BoardState {
	const timestamp = ts ?? new Date().toLocaleTimeString();
	return {
		...state,
		messages: [{ id, type, text, ts: timestamp }, ...state.messages].slice(0, 100),
	};
}

function normalizeCanHealth(
	canHealth: PlatformMessage["canHealth"] | StatusMessage["canHealth"] | undefined,
): BoardState["canHealth"] | undefined {
	if (!canHealth) {
		return undefined;
	}

	return Object.fromEntries(
		Object.entries(canHealth).map(([key, value]) => [
			key,
			{ on: Boolean(value.on), det: Boolean(value.det) },
		]),
	) as BoardState["canHealth"];
}

function buildPlatformPatch(
	prev: BoardState,
	platform: {
		model?: number;
		hwGen?: number;
		swYear?: number;
		swWeek?: number;
		swRelease?: number;
		fsdProto?: number;
		swCompat?: number;
		resolved?: number | boolean;
		canHealth?: PlatformMessage["canHealth"] | StatusMessage["canHealth"];
	},
): Partial<BoardState> {
	return {
		platformModel: platform.model ?? prev.platformModel,
		platformHwGen: platform.hwGen ?? prev.platformHwGen,
		platformSwYear: platform.swYear ?? prev.platformSwYear,
		platformSwWeek: platform.swWeek ?? prev.platformSwWeek,
		platformSwRelease: platform.swRelease ?? prev.platformSwRelease,
		platformFsdProto: platform.fsdProto ?? prev.platformFsdProto,
		platformSwCompat: platform.swCompat ?? prev.platformSwCompat,
		platformResolved:
			platform.resolved !== undefined ? Boolean(platform.resolved) : prev.platformResolved,
		canHealth: normalizeCanHealth(platform.canHealth) ?? prev.canHealth,
	};
}

// ── Initial State ───────────────────────────────────────────────────────────

export const initialBoardState: BoardState = {
	variant: "hw4",
	hardware: "—",
	driver: "—",
	board: "unknown",
	uptime: 0,
	rate: 0,

	fsd: false,
	fsdForce: false,
	nag: false,
	profile: 1,
	profilePinned: false,
	offset: 0,
	offsetPinned: false,
	isaChime: false,
	summonInject: false,
	summonActive: false,

	nagKiller: false,
	nagKillerMode: "legacy",
	dasHandsOn: 0,
	turnSignalLeft: false,
	turnSignalRight: false,
	bsmLeftLevel: 0,
	bsmRightLevel: 0,
	doorFrontLeftOpen: false,
	doorFrontRightOpen: false,
	doorRearLeftOpen: false,
	doorRearRightOpen: false,
	driverDoorOpen: false,
	anyDoorOpen: false,
	frunkOpen: false,
	trunkOpen: false,
	cruiseSetSpeedKph: 0,
	accSpeedLimitKph: 0,
	mapSpeedLimitKph: 0,
	maxSpeedKph: 0,
	precondition: false,
	trackMode: false,
	apGateEnabled: false,
	apGateOpen: false,
	apGateReason: "waiting",
	detectedHW: 0,
	variantAutoDetect: true,
	gtwAutopilotTier: -1,
	canClockReqMHz: 8,
	canClockMHz: 8,
	banShield: false,
	banThreat: 0,
	banDetectCount: 0,
	gtwShieldArmed: false,
	gtwShieldBlocks: 0,
	enhancedAutopilot: false,
	evdEnabled: false,
	tlsscRestore: false,

	tpmsPressureFL: 0,
	tpmsPressureFR: 0,
	tpmsPressureRL: 0,
	tpmsPressureRR: 0,
	tpmsTempFL: 0,
	tpmsTempFR: 0,
	tpmsTempRL: 0,
	tpmsTempRR: 0,
	hasTpms: false,

	driveMode: 0,
	currentDriveMode: 0,

	regionCode: 0,
	regionSpoofCode: 0,
	hasRegion: false,
	cnLocked: false,

	eceR79: false,
	rateLimit: true,

	alcAutoConfirm: false,
	dasLaneChangeState: 0,

	seatbeltEmulation: false,
	apFirstEnabled: false,
	dasApState: 0,
	wiperPersist: false,
	mirrorAutoFold: false,
	canSim: false,

	singleShot: false,

	btnMapLampShort: "none",
	btnMapLampLong: "none",
	btnMapLampDouble: "none",
	btnMapParkShort: "none",
	btnMapParkLong: "none",
	btnMapParkDouble: "none",
	hasBtnMap: false,

	speedAlert: false,

	gvret: false,
	gvretPort: 23,
	gvretClients: 0,

	espNow: false,
	espNowChannel: 1,
	espNowPeers: 0,

	scanMyTesla: false,

	elm327: false,

	teslaBle: false,
	teslaBleConnected: false,
	teslaBleAuth: false,

	homeAssistant: false,
	haConnected: false,
	haEntities: 0,
	haInterval: 5000,

	bleEncrypt: false,
	bleEncryptPaired: 0,

	fwYear: 0,
	fwRelease: 0,
	fwMinor: 0,
	fwCompat: 0,
	hasFwVersion: false,

	mqtt: false,
	mqttConnected: false,

	vehicleModel: 0,
	vehicleYear: 0,
	hasVehicleConfig: false,

	platformModel: 0,
	platformHwGen: 0,
	platformSwYear: 0,
	platformSwWeek: 0,
	platformSwRelease: 0,
	platformFsdProto: 0,
	platformSwCompat: 0,
	platformResolved: false,

	canHealth: {},

	vehicleSpeed: 0,
	gearState: 0,
	accelPedal: 0,
	brakePedalState: 0,
	steeringAngle: 0,
	rearMotorRpm: 0,
	frontMotorRpm: 0,
	hasPowertrain: false,

	wheelSpeedFL: 0,
	wheelSpeedFR: 0,
	wheelSpeedRL: 0,
	wheelSpeedRR: 0,
	hasWheelSpeeds: false,

	rearInvTemp: 0,
	rearStatorTemp: 0,
	rearHeatsinkTemp: 0,
	frontInvTemp: 0,
	frontStatorTemp: 0,
	frontHeatsinkTemp: 0,
	hasMotorTemps: false,

	bmsVoltage: 0,
	bmsCurrent: 0,
	bmsPower: 0,
	bmsSoc: 0,
	bmsTempMin: 0,
	bmsTempMax: 0,
	bmsWhPerKm: 0,
	hasBms: false,

	bmsNominalFullPack: 0,
	bmsNominalRemaining: 0,
	bmsIdealRemaining: 0,
	bmsCellVoltageMax: 0,
	bmsCellVoltageMin: 0,
	bmsMaxRegenPower: 0,
	bmsMaxDischargePower: 0,
	hasEnhancedBms: false,

	bmsSocUI: 0,
	bmsSocMax: 0,
	bmsSocAvg: 0,
	bmsInitialFullPack: 0,

	bmsExpectedRange: 0,
	bmsIdealRange: 0,
	bmsRatedConsumption: 0,
	bmsActualSocInt: 0,
	bmsUsableSocInt: 0,

	bmsPowerDissipation: 0,
	bmsFlowRequest: 0,
	bmsCoolTarget: 0,
	bmsHeatTarget: 0,
	bmsPackTMin: 0,
	bmsPackTMax: 0,
	bmsThermistorTMin: 0,
	bmsThermistorTMax: 0,
	bmsModelTMin: 0,
	bmsModelTMax: 0,

	bmsStationaryHeatPower: 0,
	bmsHvacPowerBudget: 0,

	bmsPrecondAllowed: false,
	bmsHeatingWorthwhile: false,
	bmsContactorState: 0,
	bmsHvState: 0,

	bmsMinBusVoltage: 0,
	bmsMaxBusVoltage: 0,
	bmsMaxChargeCurrent: 0,
	bmsMaxDischargeCurrent: 0,

	bmsExpectedRemaining: 0,
	bmsEnergyBuffer: 0,
	bmsEnergyToCharge: 0,
	bmsFullyCharged: false,

	bmsKwhDischargeTotal: 0,
	bmsKwhChargeTotal: 0,
	bmsAcChargeTotal: 0,
	bmsDcChargeTotal: 0,
	bmsRegenTotal: 0,
	bmsDriveDischargeTotal: 0,

	bmsChargeTimeToFull: 0,

	steeringMode: 0,
	hasSteeringMode: false,

	chassisOnline: false,
	standby: false,
	vehicleOnline: false,
	bodyOnline: false,
	busChassis: true,
	busVehicle: false,
	busBody: false,
	statusLiveEnabled: false,
	statusLiveIntervalMs: 250,

	streaming: false,
	frames: [],
	frameCount: 0,

	messages: [],

	features: {
		fsd: true,
		fsdForce: true,
		profile: true,
		nag: true,
		offset: true,
		isaSpeedChime: true,
		summon: true,
	},
};

// ── Boot ────────────────────────────────────────────────────────────────────

function applyBoot(prev: BoardState, msg: BootMessage): BoardState {
	const hw = msg.hw || "—";
	return {
		...prev,
		variant: msg.variant || "hw4",
		hardware: hw,
		driver: msg.drv || "—",
		board: detectBoard(hw),
		features: msg.features || prev.features,
		fsd: msg.fsd !== undefined ? Boolean(msg.fsd) : prev.fsd,
		fsdForce: msg.fsdForce !== undefined ? Boolean(msg.fsdForce) : prev.fsdForce,
		nag: msg.nag !== undefined ? Boolean(msg.nag) : prev.nag,
		profile: msg.sp ?? prev.profile,
		profilePinned: msg.spPin !== undefined ? Boolean(msg.spPin) : prev.profilePinned,
		offset: msg.offset ?? prev.offset,
		offsetPinned: msg.offsetPin !== undefined ? Boolean(msg.offsetPin) : prev.offsetPinned,
		isaChime: msg.isaChime !== undefined ? Boolean(msg.isaChime) : prev.isaChime,
		summonInject:
			msg.summonInject !== undefined ? Boolean(msg.summonInject) : prev.summonInject,
		nagKiller: msg.nagKiller !== undefined ? Boolean(msg.nagKiller) : prev.nagKiller,
		nagKillerMode: msg.nagKillerMode || prev.nagKillerMode,
		dasHandsOn: msg.dasHandsOn ?? prev.dasHandsOn,
		turnSignalLeft:
			msg.turnSignalLeft !== undefined ? Boolean(msg.turnSignalLeft) : prev.turnSignalLeft,
		turnSignalRight:
			msg.turnSignalRight !== undefined ? Boolean(msg.turnSignalRight) : prev.turnSignalRight,
		bsmLeftLevel: msg.bsmLeftLevel !== undefined ? Number(msg.bsmLeftLevel) : prev.bsmLeftLevel,
		bsmRightLevel:
			msg.bsmRightLevel !== undefined ? Number(msg.bsmRightLevel) : prev.bsmRightLevel,
		doorFrontLeftOpen:
			msg.doorFrontLeftOpen !== undefined
				? Boolean(msg.doorFrontLeftOpen)
				: prev.doorFrontLeftOpen,
		doorFrontRightOpen:
			msg.doorFrontRightOpen !== undefined
				? Boolean(msg.doorFrontRightOpen)
				: prev.doorFrontRightOpen,
		doorRearLeftOpen:
			msg.doorRearLeftOpen !== undefined
				? Boolean(msg.doorRearLeftOpen)
				: prev.doorRearLeftOpen,
		doorRearRightOpen:
			msg.doorRearRightOpen !== undefined
				? Boolean(msg.doorRearRightOpen)
				: prev.doorRearRightOpen,
		driverDoorOpen:
			msg.driverDoorOpen !== undefined ? Boolean(msg.driverDoorOpen) : prev.driverDoorOpen,
		anyDoorOpen: msg.anyDoorOpen !== undefined ? Boolean(msg.anyDoorOpen) : prev.anyDoorOpen,
		frunkOpen: msg.frunkOpen !== undefined ? Boolean(msg.frunkOpen) : prev.frunkOpen,
		trunkOpen: msg.trunkOpen !== undefined ? Boolean(msg.trunkOpen) : prev.trunkOpen,
		cruiseSetSpeedKph:
			msg.cruiseSetSpeed !== undefined
				? Number(msg.cruiseSetSpeed) / 10
				: prev.cruiseSetSpeedKph,
		accSpeedLimitKph:
			msg.accSpeedLimit !== undefined
				? Number(msg.accSpeedLimit) / 10
				: prev.accSpeedLimitKph,
		mapSpeedLimitKph:
			msg.mapSpeedLimit !== undefined
				? Number(msg.mapSpeedLimit) / 10
				: prev.mapSpeedLimitKph,
		maxSpeedKph: msg.maxSpeed !== undefined ? Number(msg.maxSpeed) / 10 : prev.maxSpeedKph,
		precondition:
			msg.precondition !== undefined ? Boolean(msg.precondition) : prev.precondition,
		trackMode: msg.trackMode !== undefined ? Boolean(msg.trackMode) : prev.trackMode,
		apGateEnabled:
			msg.apGateEnabled !== undefined ? Boolean(msg.apGateEnabled) : prev.apGateEnabled,
		apGateOpen: msg.apGateOpen !== undefined ? Boolean(msg.apGateOpen) : prev.apGateOpen,
		apGateReason: msg.apGateReason ?? prev.apGateReason,
		detectedHW: msg.detectedHW ?? prev.detectedHW,
		variantAutoDetect:
			msg.variantAutoDetect !== undefined
				? Boolean(msg.variantAutoDetect)
				: prev.variantAutoDetect,
		gtwAutopilotTier:
			msg.gtwAutopilotTier !== undefined
				? Number(msg.gtwAutopilotTier)
				: prev.gtwAutopilotTier,
		canClockReqMHz:
			msg.canClockReqMHz !== undefined ? Number(msg.canClockReqMHz) : prev.canClockReqMHz,
		canClockMHz: msg.canClockMHz !== undefined ? Number(msg.canClockMHz) : prev.canClockMHz,
		banShield: msg.banShield !== undefined ? Boolean(msg.banShield) : prev.banShield,
		banThreat: msg.banThreat !== undefined ? Number(msg.banThreat) : prev.banThreat,
		banDetectCount:
			msg.banDetectCount !== undefined ? Number(msg.banDetectCount) : prev.banDetectCount,
		gtwShieldArmed:
			msg.gtwShieldArmed !== undefined ? Boolean(msg.gtwShieldArmed) : prev.gtwShieldArmed,
		gtwShieldBlocks:
			msg.gtwShieldBlocks !== undefined ? Number(msg.gtwShieldBlocks) : prev.gtwShieldBlocks,
		enhancedAutopilot: msg.eap !== undefined ? Boolean(msg.eap) : prev.enhancedAutopilot,
		evdEnabled: msg.evd !== undefined ? Boolean(msg.evd) : prev.evdEnabled,
		tlsscRestore: msg.tlssc !== undefined ? Boolean(msg.tlssc) : prev.tlsscRestore,
		driveMode: msg.driveMode ?? prev.driveMode,
		currentDriveMode: msg.currentDriveMode ?? prev.currentDriveMode,
		eceR79: msg.eceR79 !== undefined ? Boolean(msg.eceR79) : prev.eceR79,
		regionCode: msg.regionCode ?? prev.regionCode,
		hasRegion: msg.hasRegion !== undefined ? Boolean(msg.hasRegion) : prev.hasRegion,
		cnLocked: msg.cnLocked !== undefined ? Boolean(msg.cnLocked) : prev.cnLocked,
		rateLimit: msg.rateLimit !== undefined ? Boolean(msg.rateLimit) : prev.rateLimit,
		hasTpms: msg.hasTpms !== undefined ? Boolean(msg.hasTpms) : prev.hasTpms,
		regionSpoofCode: msg.regionSpoofCode ?? prev.regionSpoofCode,
		alcAutoConfirm:
			msg.alcAutoConfirm !== undefined ? Boolean(msg.alcAutoConfirm) : prev.alcAutoConfirm,
		dasLaneChangeState: msg.dasLaneChangeState ?? prev.dasLaneChangeState,
		seatbeltEmulation:
			msg.seatbeltEmulation !== undefined
				? Boolean(msg.seatbeltEmulation)
				: prev.seatbeltEmulation,
		apFirstEnabled: msg.apFirst !== undefined ? Boolean(msg.apFirst) : prev.apFirstEnabled,
		dasApState: msg.dasApState ?? prev.dasApState,
		wiperPersist:
			msg.wiperPersist !== undefined ? Boolean(msg.wiperPersist) : prev.wiperPersist,
		mirrorAutoFold:
			msg.mirrorAutoFold !== undefined ? Boolean(msg.mirrorAutoFold) : prev.mirrorAutoFold,
		canSim: msg.canSim !== undefined ? Boolean(msg.canSim) : prev.canSim,
		hasPowertrain:
			msg.hasPowertrain !== undefined ? Boolean(msg.hasPowertrain) : prev.hasPowertrain,
		singleShot: msg.singleShot !== undefined ? Boolean(msg.singleShot) : prev.singleShot,
		btnMapLampShort: msg.btnMapLampShort ?? prev.btnMapLampShort,
		btnMapLampLong: msg.btnMapLampLong ?? prev.btnMapLampLong,
		btnMapLampDouble: msg.btnMapLampDouble ?? prev.btnMapLampDouble,
		btnMapParkShort: msg.btnMapParkShort ?? prev.btnMapParkShort,
		btnMapParkLong: msg.btnMapParkLong ?? prev.btnMapParkLong,
		btnMapParkDouble: msg.btnMapParkDouble ?? prev.btnMapParkDouble,
		hasBtnMap: msg.hasBtnMap !== undefined ? Boolean(msg.hasBtnMap) : prev.hasBtnMap,
		speedAlert: msg.speedAlert !== undefined ? Boolean(msg.speedAlert) : prev.speedAlert,
		gvret: msg.gvret !== undefined ? Boolean(msg.gvret) : prev.gvret,
		gvretPort: msg.gvretPort ?? prev.gvretPort,
		gvretClients: msg.gvretClients ?? prev.gvretClients,
		espNow: msg.espNow !== undefined ? Boolean(msg.espNow) : prev.espNow,
		espNowChannel: msg.espNowChannel ?? prev.espNowChannel,
		espNowPeers: msg.espNowPeers ?? prev.espNowPeers,
		scanMyTesla: msg.scanMyTesla !== undefined ? Boolean(msg.scanMyTesla) : prev.scanMyTesla,
		elm327: msg.elm327 !== undefined ? Boolean(msg.elm327) : prev.elm327,
		teslaBle: msg.teslaBle !== undefined ? Boolean(msg.teslaBle) : prev.teslaBle,
		teslaBleConnected:
			msg.teslaBleConnected !== undefined
				? Boolean(msg.teslaBleConnected)
				: prev.teslaBleConnected,
		teslaBleAuth:
			msg.teslaBleAuth !== undefined ? Boolean(msg.teslaBleAuth) : prev.teslaBleAuth,
		homeAssistant:
			msg.homeAssistant !== undefined ? Boolean(msg.homeAssistant) : prev.homeAssistant,
		haConnected: msg.haConnected !== undefined ? Boolean(msg.haConnected) : prev.haConnected,
		haEntities: msg.haEntities ?? prev.haEntities,
		haInterval: msg.haInterval ?? prev.haInterval,
		bleEncrypt: msg.bleEncrypt !== undefined ? Boolean(msg.bleEncrypt) : prev.bleEncrypt,
		bleEncryptPaired: msg.bleEncryptPaired ?? prev.bleEncryptPaired,
		fwYear: msg.fwYear ?? prev.fwYear,
		fwRelease: msg.fwRelease ?? prev.fwRelease,
		fwMinor: msg.fwMinor ?? prev.fwMinor,
		fwCompat: msg.fwCompat ?? prev.fwCompat,
		hasFwVersion:
			msg.hasFwVersion !== undefined ? Boolean(msg.hasFwVersion) : prev.hasFwVersion,
		mqtt: msg.mqtt !== undefined ? Boolean(msg.mqtt) : prev.mqtt,
		mqttConnected:
			msg.mqttConnected !== undefined ? Boolean(msg.mqttConnected) : prev.mqttConnected,
		vehicleModel: msg.vehicleModel ?? prev.vehicleModel,
		vehicleYear: msg.vehicleYear ?? prev.vehicleYear,
		hasVehicleConfig:
			msg.hasVehicleConfig !== undefined
				? Boolean(msg.hasVehicleConfig)
				: prev.hasVehicleConfig,
		...buildPlatformPatch(prev, {
			model: msg.platformModel,
			hwGen: msg.platformHwGen,
			swYear: msg.platformSwYear,
			swWeek: msg.platformSwWeek,
			swRelease: msg.platformSwRelease,
			fsdProto: msg.platformFsdProto,
			swCompat: msg.platformSwCompat,
			resolved: msg.platformResolved,
			canHealth: msg.canHealth,
		}),
		bmsNominalFullPack:
			msg.bmsNomFullPack !== undefined
				? Number(msg.bmsNomFullPack) / 100
				: prev.bmsNominalFullPack,
		bmsNominalRemaining:
			msg.bmsNomRemain !== undefined
				? Number(msg.bmsNomRemain) / 100
				: prev.bmsNominalRemaining,
		bmsIdealRemaining:
			msg.bmsIdealRemain !== undefined
				? Number(msg.bmsIdealRemain) / 100
				: prev.bmsIdealRemaining,
		bmsCellVoltageMax:
			msg.bmsCellVMax !== undefined ? Number(msg.bmsCellVMax) / 1000 : prev.bmsCellVoltageMax,
		bmsCellVoltageMin:
			msg.bmsCellVMin !== undefined ? Number(msg.bmsCellVMin) / 1000 : prev.bmsCellVoltageMin,
		bmsMaxRegenPower:
			msg.bmsMaxRegen !== undefined ? Number(msg.bmsMaxRegen) / 100 : prev.bmsMaxRegenPower,
		bmsMaxDischargePower:
			msg.bmsMaxDischarge !== undefined
				? Number(msg.bmsMaxDischarge) / 100
				: prev.bmsMaxDischargePower,
		hasEnhancedBms:
			msg.hasEnhancedBms !== undefined ? Boolean(msg.hasEnhancedBms) : prev.hasEnhancedBms,
		steeringMode: msg.steeringMode !== undefined ? Number(msg.steeringMode) : prev.steeringMode,
		hasSteeringMode:
			msg.hasSteeringMode !== undefined ? Boolean(msg.hasSteeringMode) : prev.hasSteeringMode,
		chassisOnline:
			msg.chassisOnline !== undefined ? Boolean(msg.chassisOnline) : prev.chassisOnline,
		standby: msg.standby !== undefined ? Boolean(msg.standby) : prev.standby,
		vehicleOnline:
			msg.vehicleOnline !== undefined ? Boolean(msg.vehicleOnline) : prev.vehicleOnline,
		bodyOnline: msg.bodyOnline !== undefined ? Boolean(msg.bodyOnline) : prev.bodyOnline,
		busChassis: msg.busChassis !== undefined ? Boolean(msg.busChassis) : prev.busChassis,
		busVehicle: msg.busVehicle !== undefined ? Boolean(msg.busVehicle) : prev.busVehicle,
		busBody: msg.busBody !== undefined ? Boolean(msg.busBody) : prev.busBody,
	};
}

// ── Status ──────────────────────────────────────────────────────────────────

function applyStatus(prev: BoardState, msg: StatusMessage): BoardState {
	const hw = msg.hw || "";

	return {
		...prev,
		variant: msg.variant || prev.variant,
		hardware: hw || prev.hardware,
		driver: msg.drv || prev.driver,
		board: hw ? detectBoard(hw) : prev.board,
		uptime: msg.up ?? prev.uptime,
		rate: msg.rate ?? prev.rate,
		fsd: msg.fsd !== undefined ? Boolean(msg.fsd) : prev.fsd,
		fsdForce: msg.fsdForce !== undefined ? Boolean(msg.fsdForce) : prev.fsdForce,
		nag: msg.nag !== undefined ? Boolean(msg.nag) : prev.nag,
		profile: msg.sp ?? prev.profile,
		profilePinned: msg.spPin !== undefined ? Boolean(msg.spPin) : prev.profilePinned,
		offset: msg.offset ?? prev.offset,
		offsetPinned: msg.offsetPin !== undefined ? Boolean(msg.offsetPin) : prev.offsetPinned,
		isaChime: msg.isaChime !== undefined ? Boolean(msg.isaChime) : prev.isaChime,
		summonInject:
			msg.summonInject !== undefined ? Boolean(msg.summonInject) : prev.summonInject,
		nagKiller: msg.nagKiller !== undefined ? Boolean(msg.nagKiller) : prev.nagKiller,
		nagKillerMode: msg.nagKillerMode || prev.nagKillerMode,
		dasHandsOn: msg.dasHandsOn ?? prev.dasHandsOn,
		turnSignalLeft:
			msg.turnSignalLeft !== undefined ? Boolean(msg.turnSignalLeft) : prev.turnSignalLeft,
		turnSignalRight:
			msg.turnSignalRight !== undefined ? Boolean(msg.turnSignalRight) : prev.turnSignalRight,
		bsmLeftLevel: msg.bsmLeftLevel !== undefined ? Number(msg.bsmLeftLevel) : prev.bsmLeftLevel,
		bsmRightLevel:
			msg.bsmRightLevel !== undefined ? Number(msg.bsmRightLevel) : prev.bsmRightLevel,
		doorFrontLeftOpen:
			msg.doorFrontLeftOpen !== undefined
				? Boolean(msg.doorFrontLeftOpen)
				: prev.doorFrontLeftOpen,
		doorFrontRightOpen:
			msg.doorFrontRightOpen !== undefined
				? Boolean(msg.doorFrontRightOpen)
				: prev.doorFrontRightOpen,
		doorRearLeftOpen:
			msg.doorRearLeftOpen !== undefined
				? Boolean(msg.doorRearLeftOpen)
				: prev.doorRearLeftOpen,
		doorRearRightOpen:
			msg.doorRearRightOpen !== undefined
				? Boolean(msg.doorRearRightOpen)
				: prev.doorRearRightOpen,
		driverDoorOpen:
			msg.driverDoorOpen !== undefined ? Boolean(msg.driverDoorOpen) : prev.driverDoorOpen,
		anyDoorOpen: msg.anyDoorOpen !== undefined ? Boolean(msg.anyDoorOpen) : prev.anyDoorOpen,
		frunkOpen: msg.frunkOpen !== undefined ? Boolean(msg.frunkOpen) : prev.frunkOpen,
		trunkOpen: msg.trunkOpen !== undefined ? Boolean(msg.trunkOpen) : prev.trunkOpen,
		cruiseSetSpeedKph:
			msg.cruiseSetSpeed !== undefined
				? Number(msg.cruiseSetSpeed) / 10
				: prev.cruiseSetSpeedKph,
		accSpeedLimitKph:
			msg.accSpeedLimit !== undefined
				? Number(msg.accSpeedLimit) / 10
				: prev.accSpeedLimitKph,
		mapSpeedLimitKph:
			msg.mapSpeedLimit !== undefined
				? Number(msg.mapSpeedLimit) / 10
				: prev.mapSpeedLimitKph,
		maxSpeedKph: msg.maxSpeed !== undefined ? Number(msg.maxSpeed) / 10 : prev.maxSpeedKph,
		precondition:
			msg.precondition !== undefined ? Boolean(msg.precondition) : prev.precondition,
		trackMode: msg.trackMode !== undefined ? Boolean(msg.trackMode) : prev.trackMode,
		apGateEnabled:
			msg.apGateEnabled !== undefined ? Boolean(msg.apGateEnabled) : prev.apGateEnabled,
		apGateOpen: msg.apGateOpen !== undefined ? Boolean(msg.apGateOpen) : prev.apGateOpen,
		apGateReason: msg.apGateReason ?? prev.apGateReason,
		detectedHW: msg.detectedHW ?? prev.detectedHW,
		variantAutoDetect:
			msg.variantAutoDetect !== undefined
				? Boolean(msg.variantAutoDetect)
				: prev.variantAutoDetect,
		gtwAutopilotTier:
			msg.gtwAutopilotTier !== undefined
				? Number(msg.gtwAutopilotTier)
				: prev.gtwAutopilotTier,
		canClockReqMHz:
			msg.canClockReqMHz !== undefined ? Number(msg.canClockReqMHz) : prev.canClockReqMHz,
		canClockMHz: msg.canClockMHz !== undefined ? Number(msg.canClockMHz) : prev.canClockMHz,
		streaming: msg.stream !== undefined ? Boolean(msg.stream?.on) : prev.streaming,
		banShield: msg.banShield !== undefined ? Boolean(msg.banShield) : prev.banShield,
		banThreat: msg.banThreat !== undefined ? Number(msg.banThreat) : prev.banThreat,
		banDetectCount:
			msg.banDetectCount !== undefined ? Number(msg.banDetectCount) : prev.banDetectCount,
		gtwShieldArmed:
			msg.gtwShieldArmed !== undefined ? Boolean(msg.gtwShieldArmed) : prev.gtwShieldArmed,
		gtwShieldBlocks:
			msg.gtwShieldBlocks !== undefined ? Number(msg.gtwShieldBlocks) : prev.gtwShieldBlocks,
		enhancedAutopilot: msg.eap !== undefined ? Boolean(msg.eap) : prev.enhancedAutopilot,
		evdEnabled: msg.evd !== undefined ? Boolean(msg.evd) : prev.evdEnabled,
		tlsscRestore: msg.tlssc !== undefined ? Boolean(msg.tlssc) : prev.tlsscRestore,
		driveMode: msg.driveMode ?? prev.driveMode,
		currentDriveMode: msg.currentDriveMode ?? prev.currentDriveMode,
		eceR79: msg.eceR79 !== undefined ? Boolean(msg.eceR79) : prev.eceR79,
		regionCode: msg.regionCode ?? prev.regionCode,
		hasRegion: msg.hasRegion !== undefined ? Boolean(msg.hasRegion) : prev.hasRegion,
		cnLocked: msg.cnLocked !== undefined ? Boolean(msg.cnLocked) : prev.cnLocked,
		rateLimit: msg.rateLimit !== undefined ? Boolean(msg.rateLimit) : prev.rateLimit,
		hasTpms: msg.hasTpms !== undefined ? Boolean(msg.hasTpms) : prev.hasTpms,
		regionSpoofCode: msg.regionSpoofCode ?? prev.regionSpoofCode,
		alcAutoConfirm:
			msg.alcAutoConfirm !== undefined ? Boolean(msg.alcAutoConfirm) : prev.alcAutoConfirm,
		dasLaneChangeState: msg.dasLaneChangeState ?? prev.dasLaneChangeState,
		seatbeltEmulation:
			msg.seatbeltEmulation !== undefined
				? Boolean(msg.seatbeltEmulation)
				: prev.seatbeltEmulation,
		apFirstEnabled: msg.apFirst !== undefined ? Boolean(msg.apFirst) : prev.apFirstEnabled,
		dasApState: msg.dasApState ?? prev.dasApState,
		wiperPersist:
			msg.wiperPersist !== undefined ? Boolean(msg.wiperPersist) : prev.wiperPersist,
		mirrorAutoFold:
			msg.mirrorAutoFold !== undefined ? Boolean(msg.mirrorAutoFold) : prev.mirrorAutoFold,
		canSim: msg.canSim !== undefined ? Boolean(msg.canSim) : prev.canSim,
		hasPowertrain:
			msg.hasPowertrain !== undefined ? Boolean(msg.hasPowertrain) : prev.hasPowertrain,
		singleShot: msg.singleShot !== undefined ? Boolean(msg.singleShot) : prev.singleShot,
		btnMapLampShort: msg.btnMapLampShort ?? prev.btnMapLampShort,
		btnMapLampLong: msg.btnMapLampLong ?? prev.btnMapLampLong,
		btnMapLampDouble: msg.btnMapLampDouble ?? prev.btnMapLampDouble,
		btnMapParkShort: msg.btnMapParkShort ?? prev.btnMapParkShort,
		btnMapParkLong: msg.btnMapParkLong ?? prev.btnMapParkLong,
		btnMapParkDouble: msg.btnMapParkDouble ?? prev.btnMapParkDouble,
		hasBtnMap: msg.hasBtnMap !== undefined ? Boolean(msg.hasBtnMap) : prev.hasBtnMap,
		speedAlert: msg.speedAlert !== undefined ? Boolean(msg.speedAlert) : prev.speedAlert,
		gvret: msg.gvret !== undefined ? Boolean(msg.gvret) : prev.gvret,
		gvretPort: msg.gvretPort ?? prev.gvretPort,
		gvretClients: msg.gvretClients ?? prev.gvretClients,
		espNow: msg.espNow !== undefined ? Boolean(msg.espNow) : prev.espNow,
		espNowChannel: msg.espNowChannel ?? prev.espNowChannel,
		espNowPeers: msg.espNowPeers ?? prev.espNowPeers,
		scanMyTesla: msg.scanMyTesla !== undefined ? Boolean(msg.scanMyTesla) : prev.scanMyTesla,
		elm327: msg.elm327 !== undefined ? Boolean(msg.elm327) : prev.elm327,
		teslaBle: msg.teslaBle !== undefined ? Boolean(msg.teslaBle) : prev.teslaBle,
		teslaBleConnected:
			msg.teslaBleConnected !== undefined
				? Boolean(msg.teslaBleConnected)
				: prev.teslaBleConnected,
		teslaBleAuth:
			msg.teslaBleAuth !== undefined ? Boolean(msg.teslaBleAuth) : prev.teslaBleAuth,
		homeAssistant:
			msg.homeAssistant !== undefined ? Boolean(msg.homeAssistant) : prev.homeAssistant,
		haConnected: msg.haConnected !== undefined ? Boolean(msg.haConnected) : prev.haConnected,
		haEntities: msg.haEntities ?? prev.haEntities,
		haInterval: msg.haInterval ?? prev.haInterval,
		bleEncrypt: msg.bleEncrypt !== undefined ? Boolean(msg.bleEncrypt) : prev.bleEncrypt,
		bleEncryptPaired: msg.bleEncryptPaired ?? prev.bleEncryptPaired,
		fwYear: msg.fwYear ?? prev.fwYear,
		fwRelease: msg.fwRelease ?? prev.fwRelease,
		fwMinor: msg.fwMinor ?? prev.fwMinor,
		fwCompat: msg.fwCompat ?? prev.fwCompat,
		hasFwVersion:
			msg.hasFwVersion !== undefined ? Boolean(msg.hasFwVersion) : prev.hasFwVersion,
		mqtt: msg.mqtt !== undefined ? Boolean(msg.mqtt) : prev.mqtt,
		mqttConnected:
			msg.mqttConnected !== undefined ? Boolean(msg.mqttConnected) : prev.mqttConnected,
		vehicleModel: msg.vehicleModel ?? prev.vehicleModel,
		vehicleYear: msg.vehicleYear ?? prev.vehicleYear,
		hasVehicleConfig:
			msg.hasVehicleConfig !== undefined
				? Boolean(msg.hasVehicleConfig)
				: prev.hasVehicleConfig,
		...buildPlatformPatch(prev, {
			model: msg.platformModel,
			hwGen: msg.platformHwGen,
			swYear: msg.platformSwYear,
			swWeek: msg.platformSwWeek,
			swRelease: msg.platformSwRelease,
			fsdProto: msg.platformFsdProto,
			swCompat: msg.platformSwCompat,
			resolved: msg.platformResolved,
			canHealth: msg.canHealth,
		}),
		bmsNominalFullPack:
			msg.bmsNomFullPack !== undefined
				? Number(msg.bmsNomFullPack) / 100
				: prev.bmsNominalFullPack,
		bmsNominalRemaining:
			msg.bmsNomRemain !== undefined
				? Number(msg.bmsNomRemain) / 100
				: prev.bmsNominalRemaining,
		bmsIdealRemaining:
			msg.bmsIdealRemain !== undefined
				? Number(msg.bmsIdealRemain) / 100
				: prev.bmsIdealRemaining,
		bmsCellVoltageMax:
			msg.bmsCellVMax !== undefined ? Number(msg.bmsCellVMax) / 1000 : prev.bmsCellVoltageMax,
		bmsCellVoltageMin:
			msg.bmsCellVMin !== undefined ? Number(msg.bmsCellVMin) / 1000 : prev.bmsCellVoltageMin,
		bmsMaxRegenPower:
			msg.bmsMaxRegen !== undefined ? Number(msg.bmsMaxRegen) / 100 : prev.bmsMaxRegenPower,
		bmsMaxDischargePower:
			msg.bmsMaxDischarge !== undefined
				? Number(msg.bmsMaxDischarge) / 100
				: prev.bmsMaxDischargePower,
		hasEnhancedBms:
			msg.hasEnhancedBms !== undefined ? Boolean(msg.hasEnhancedBms) : prev.hasEnhancedBms,
		steeringMode: msg.steeringMode !== undefined ? Number(msg.steeringMode) : prev.steeringMode,
		hasSteeringMode:
			msg.hasSteeringMode !== undefined ? Boolean(msg.hasSteeringMode) : prev.hasSteeringMode,
		features: msg.features || prev.features,
		chassisOnline:
			msg.chassisOnline !== undefined ? Boolean(msg.chassisOnline) : prev.chassisOnline,
		standby: msg.standby !== undefined ? Boolean(msg.standby) : prev.standby,
		vehicleOnline:
			msg.vehicleOnline !== undefined ? Boolean(msg.vehicleOnline) : prev.vehicleOnline,
		bodyOnline: msg.bodyOnline !== undefined ? Boolean(msg.bodyOnline) : prev.bodyOnline,
		busChassis: msg.busChassis !== undefined ? Boolean(msg.busChassis) : prev.busChassis,
		busVehicle: msg.busVehicle !== undefined ? Boolean(msg.busVehicle) : prev.busVehicle,
		busBody: msg.busBody !== undefined ? Boolean(msg.busBody) : prev.busBody,
	};
}

// ── Frame ───────────────────────────────────────────────────────────────────

function applyFrame(prev: BoardState, msg: FrameMessage, ts: string): BoardState {
	const bus = msg.bus ?? 0;
	const frame: CanFrame = {
		key: `${msg.seq}-${msg.id}`,
		id: msg.id,
		dir: msg.dir,
		bus,
		busName: BUS_NAMES[bus] ?? `Bus${bus}`,
		seq: msg.seq,
		dlc: msg.dlc,
		data: msg.d || "",
		ts,
	};
	return {
		...prev,
		frames: [frame, ...prev.frames].slice(0, 100),
		frameCount: prev.frameCount + 1,
	};
}

// ── BMS ─────────────────────────────────────────────────────────────────────

function applyBms(prev: BoardState, msg: BmsMessage): BoardState {
	return {
		...prev,
		bmsVoltage: msg.v / 100,
		bmsCurrent: msg.a / 10,
		bmsPower: msg.kw / 10,
		bmsSoc: msg.soc / 10,
		bmsTempMin: msg.tMin,
		bmsTempMax: msg.tMax,
		bmsWhPerKm: msg.whkm / 10,
		bmsNominalFullPack: msg.nomFull !== undefined ? msg.nomFull / 100 : prev.bmsNominalFullPack,
		bmsNominalRemaining:
			msg.nomRemain !== undefined ? msg.nomRemain / 100 : prev.bmsNominalRemaining,
		bmsIdealRemaining:
			msg.idealRemain !== undefined ? msg.idealRemain / 100 : prev.bmsIdealRemaining,
		bmsCellVoltageMax:
			msg.cellVMax !== undefined ? msg.cellVMax / 1000 : prev.bmsCellVoltageMax,
		bmsCellVoltageMin:
			msg.cellVMin !== undefined ? msg.cellVMin / 1000 : prev.bmsCellVoltageMin,
		bmsMaxRegenPower: msg.maxRegen !== undefined ? msg.maxRegen / 100 : prev.bmsMaxRegenPower,
		bmsMaxDischargePower:
			msg.maxDischarge !== undefined ? msg.maxDischarge / 100 : prev.bmsMaxDischargePower,
		hasEnhancedBms: msg.enhanced !== undefined ? Boolean(msg.enhanced) : prev.hasEnhancedBms,
		hasBms: Boolean(msg.ok),
		// Expanded fields
		bmsSocUI: msg.socUI !== undefined ? msg.socUI / 10 : prev.bmsSocUI,
		bmsSocMax: msg.socMax !== undefined ? msg.socMax / 10 : prev.bmsSocMax,
		bmsSocAvg: msg.socAvg !== undefined ? msg.socAvg / 10 : prev.bmsSocAvg,
		bmsInitialFullPack:
			msg.initFull !== undefined ? msg.initFull / 10 : prev.bmsInitialFullPack,
		bmsExpectedRange: msg.expRange !== undefined ? msg.expRange / 10 : prev.bmsExpectedRange,
		bmsIdealRange: msg.idealRange !== undefined ? msg.idealRange / 10 : prev.bmsIdealRange,
		bmsRatedConsumption:
			msg.ratedCons !== undefined ? msg.ratedCons / 10 : prev.bmsRatedConsumption,
		bmsActualSocInt: msg.actSoc ?? prev.bmsActualSocInt,
		bmsUsableSocInt: msg.useSoc ?? prev.bmsUsableSocInt,
		bmsPowerDissipation:
			msg.pwrDiss !== undefined ? msg.pwrDiss / 100 : prev.bmsPowerDissipation,
		bmsFlowRequest: msg.flowReq !== undefined ? msg.flowReq / 10 : prev.bmsFlowRequest,
		bmsCoolTarget: msg.coolTgt !== undefined ? msg.coolTgt / 10 : prev.bmsCoolTarget,
		bmsHeatTarget: msg.heatTgt !== undefined ? msg.heatTgt / 10 : prev.bmsHeatTarget,
		bmsPackTMin: msg.packTMin !== undefined ? msg.packTMin / 10 : prev.bmsPackTMin,
		bmsPackTMax: msg.packTMax !== undefined ? msg.packTMax / 10 : prev.bmsPackTMax,
		bmsStationaryHeatPower:
			msg.heatPwr !== undefined ? msg.heatPwr / 100 : prev.bmsStationaryHeatPower,
		bmsHvacPowerBudget: msg.hvacBgt !== undefined ? msg.hvacBgt / 100 : prev.bmsHvacPowerBudget,
		bmsPrecondAllowed:
			msg.precondOk !== undefined ? Boolean(msg.precondOk) : prev.bmsPrecondAllowed,
		bmsHeatingWorthwhile:
			msg.heatWorth !== undefined ? Boolean(msg.heatWorth) : prev.bmsHeatingWorthwhile,
		bmsContactorState: msg.contState ?? prev.bmsContactorState,
		bmsHvState: msg.hvState ?? prev.bmsHvState,
		bmsMinBusVoltage: msg.minBusV !== undefined ? msg.minBusV / 100 : prev.bmsMinBusVoltage,
		bmsMaxBusVoltage: msg.maxBusV !== undefined ? msg.maxBusV / 100 : prev.bmsMaxBusVoltage,
		bmsMaxChargeCurrent:
			msg.maxChgA !== undefined ? msg.maxChgA / 10 : prev.bmsMaxChargeCurrent,
		bmsMaxDischargeCurrent:
			msg.maxDchA !== undefined ? msg.maxDchA / 10 : prev.bmsMaxDischargeCurrent,
		bmsExpectedRemaining:
			msg.expRemain !== undefined ? msg.expRemain / 100 : prev.bmsExpectedRemaining,
		bmsEnergyBuffer: msg.eBuf !== undefined ? msg.eBuf / 100 : prev.bmsEnergyBuffer,
		bmsEnergyToCharge: msg.eToChg !== undefined ? msg.eToChg / 100 : prev.bmsEnergyToCharge,
		bmsFullyCharged: msg.charged !== undefined ? Boolean(msg.charged) : prev.bmsFullyCharged,
		bmsKwhDischargeTotal: msg.kwhDch ?? prev.bmsKwhDischargeTotal,
		bmsKwhChargeTotal: msg.kwhChg ?? prev.bmsKwhChargeTotal,
		bmsAcChargeTotal: msg.acChg ?? prev.bmsAcChargeTotal,
		bmsDcChargeTotal: msg.dcChg ?? prev.bmsDcChargeTotal,
		bmsRegenTotal: msg.regen ?? prev.bmsRegenTotal,
		bmsDriveDischargeTotal: msg.drvDch ?? prev.bmsDriveDischargeTotal,
		bmsChargeTimeToFull:
			msg.chgTime !== undefined ? msg.chgTime / 100 : prev.bmsChargeTimeToFull,
	};
}

function applyPlatform(prev: BoardState, msg: PlatformMessage): BoardState {
	return {
		...prev,
		...buildPlatformPatch(prev, {
			model: msg.model,
			hwGen: msg.hwGen,
			swYear: msg.swYear,
			swWeek: msg.swWeek,
			swRelease: msg.swRelease,
			fsdProto: msg.fsdProto,
			swCompat: msg.swCompat,
			resolved: msg.resolved,
			canHealth: msg.canHealth,
		}),
	};
}

function applyStatusMeta(prev: BoardState, msg: StatusMetaMessage): BoardState {
	const hw = msg.hw ?? prev.hardware;
	return {
		...prev,
		variant: msg.variant ?? prev.variant,
		hardware: hw,
		driver: msg.drv ?? prev.driver,
		board: msg.hw ? detectBoard(msg.hw) : prev.board,
		uptime: msg.up ?? prev.uptime,
	};
}

function applyStatusFeatures(prev: BoardState, msg: StatusFeaturesMessage): BoardState {
	return {
		...prev,
		features: msg.features ?? prev.features,
	};
}

function applyStatusCan(prev: BoardState, msg: StatusCanMessage): BoardState {
	const canHealth = normalizeCanHealth(msg.canHealth ?? msg.health);
	return {
		...prev,
		canClockReqMHz: msg.canClockReqMHz ?? msg.clock?.reqMHz ?? prev.canClockReqMHz,
		canClockMHz: msg.canClockMHz ?? msg.clock?.activeMHz ?? prev.canClockMHz,
		canHealth: canHealth ?? prev.canHealth,
	};
}

function applyStatusState(prev: BoardState, msg: StatusStateMessage): BoardState {
	const state = msg.state;
	const profile = typeof state?.profile === "number" ? state.profile : undefined;
	const profilePinned =
		typeof state?.profile === "object" && state.profile !== null
			? state.profile.pinned
			: undefined;
	const offset = typeof state?.offset === "number" ? state.offset : undefined;
	const offsetPinned =
		typeof state?.offset === "object" && state.offset !== null
			? state.offset.pinned
			: undefined;

	return {
		...prev,
		fsd:
			msg.fsd !== undefined
				? Boolean(msg.fsd)
				: state?.fsd !== undefined
					? Boolean(state.fsd)
					: prev.fsd,
		fsdForce:
			msg.fsdForce !== undefined
				? Boolean(msg.fsdForce)
				: state?.fsdForce !== undefined
					? Boolean(state.fsdForce)
					: prev.fsdForce,
		nag:
			msg.nag !== undefined
				? Boolean(msg.nag)
				: state?.nag !== undefined
					? Boolean(state.nag)
					: prev.nag,
		nagKiller:
			msg.nagKiller !== undefined
				? Boolean(msg.nagKiller)
				: state?.nagKiller !== undefined
					? Boolean(state.nagKiller)
					: prev.nagKiller,
		profile: msg.sp ?? profile ?? prev.profile,
		profilePinned:
			msg.spPin !== undefined
				? Boolean(msg.spPin)
				: profilePinned !== undefined
					? Boolean(profilePinned)
					: prev.profilePinned,
		offset: msg.offset ?? offset ?? prev.offset,
		offsetPinned:
			msg.offsetPin !== undefined
				? Boolean(msg.offsetPin)
				: offsetPinned !== undefined
					? Boolean(offsetPinned)
					: prev.offsetPinned,
		precondition:
			msg.precondition !== undefined
				? Boolean(msg.precondition)
				: state?.precondition !== undefined
					? Boolean(state.precondition)
					: prev.precondition,
		trackMode:
			msg.trackMode !== undefined
				? Boolean(msg.trackMode)
				: state?.trackMode !== undefined
					? Boolean(state.trackMode)
					: prev.trackMode,
		apGateEnabled:
			msg.apGateEnabled !== undefined
				? Boolean(msg.apGateEnabled)
				: state?.apGateEnabled !== undefined
					? Boolean(state.apGateEnabled)
					: prev.apGateEnabled,
		apGateOpen:
			msg.apGateOpen !== undefined
				? Boolean(msg.apGateOpen)
				: state?.apGateOpen !== undefined
					? Boolean(state.apGateOpen)
					: prev.apGateOpen,
		apGateReason:
			msg.apGateReason ??
			(typeof state?.apGateReason === "string" ? state.apGateReason : undefined) ??
			prev.apGateReason,
	};
}

function applyStatusLive(prev: BoardState, on: number | boolean, intervalMs: number): BoardState {
	return {
		...prev,
		statusLiveEnabled: Boolean(on),
		statusLiveIntervalMs: intervalMs,
	};
}

function applyStatusCompact(prev: BoardState, msg: StatusCompactMessage): BoardState {
	const hw = msg.hw ?? msg.meta?.hw ?? prev.hardware;
	const variant = msg.variant ?? msg.meta?.variant ?? prev.variant;
	const canHealth = normalizeCanHealth(msg.canHealth ?? msg.can?.health);
	const features = msg.features ?? prev.features;
	const streaming = msg.stream !== undefined ? Boolean(msg.stream?.on) : prev.streaming;

	return {
		...prev,
		variant,
		hardware: hw,
		board: hw !== prev.hardware ? detectBoard(hw) : prev.board,
		uptime: msg.up ?? msg.meta?.up ?? prev.uptime,
		chassisOnline:
			msg.chassisOnline !== undefined
				? Boolean(msg.chassisOnline)
				: msg.connectivity?.chassisOnline !== undefined
					? Boolean(msg.connectivity.chassisOnline)
					: prev.chassisOnline,
		vehicleOnline:
			msg.vehicleOnline !== undefined
				? Boolean(msg.vehicleOnline)
				: msg.connectivity?.vehicleOnline !== undefined
					? Boolean(msg.connectivity.vehicleOnline)
					: prev.vehicleOnline,
		bodyOnline:
			msg.bodyOnline !== undefined
				? Boolean(msg.bodyOnline)
				: msg.connectivity?.bodyOnline !== undefined
					? Boolean(msg.connectivity.bodyOnline)
					: prev.bodyOnline,
		standby:
			msg.standby !== undefined
				? Boolean(msg.standby)
				: msg.connectivity?.standby !== undefined
					? Boolean(msg.connectivity.standby)
					: prev.standby,
		fsd:
			msg.fsd !== undefined
				? Boolean(msg.fsd)
				: msg.state?.fsd !== undefined
					? Boolean(msg.state.fsd)
					: prev.fsd,
		fsdForce:
			msg.fsdForce !== undefined
				? Boolean(msg.fsdForce)
				: msg.state?.fsdForce !== undefined
					? Boolean(msg.state.fsdForce)
					: prev.fsdForce,
		nag:
			msg.nag !== undefined
				? Boolean(msg.nag)
				: msg.state?.nag !== undefined
					? Boolean(msg.state.nag)
					: prev.nag,
		profile:
			msg.sp ??
			(typeof msg.state?.profile === "number" ? msg.state.profile : undefined) ??
			prev.profile,
		offset:
			msg.offset ??
			(typeof msg.state?.offset === "number" ? msg.state.offset : undefined) ??
			prev.offset,
		precondition:
			msg.state?.precondition !== undefined
				? Boolean(msg.state.precondition)
				: prev.precondition,
		trackMode:
			msg.state?.trackMode !== undefined ? Boolean(msg.state.trackMode) : prev.trackMode,
		apGateEnabled:
			msg.state?.apGateEnabled !== undefined
				? Boolean(msg.state.apGateEnabled)
				: prev.apGateEnabled,
		apGateOpen:
			msg.state?.apGateOpen !== undefined ? Boolean(msg.state.apGateOpen) : prev.apGateOpen,
		apGateReason: msg.state?.apGateReason ?? prev.apGateReason,
		features,
		canClockReqMHz: msg.canClockReqMHz ?? msg.can?.clock?.reqMHz ?? prev.canClockReqMHz,
		canClockMHz: msg.canClockMHz ?? msg.can?.clock?.activeMHz ?? prev.canClockMHz,
		canHealth: canHealth ?? prev.canHealth,
		streaming,
	};
}

// ── Main Reducer ────────────────────────────────────────────────────────────

/**
 * Reduce a firmware message into a new BoardState.
 * @param prev     Current state
 * @param msg      Typed firmware message (caller casts from Record<string,unknown>)
 * @param nextId   Function returning the next unique message ID
 * @param ts       Optional timestamp string (defaults to current time)
 */
export function reduceBoardMessage(
	prev: BoardState,
	msg: BoardMessage,
	nextId: () => number,
	ts?: string,
): BoardState {
	const timestamp = ts ?? new Date().toLocaleTimeString();

	if (msg.t === "boot") {
		const updated = applyBoot(prev, msg);
		return addNotification(updated, "info", `Board connected: ${msg.hw}`, nextId(), timestamp);
	}

	if (msg.t === "status") {
		return applyStatus(prev, msg);
	}

	if (msg.t === "platform") {
		return applyPlatform(prev, msg);
	}

	if (msg.t === "status_meta") {
		return applyStatusMeta(prev, msg);
	}

	if (msg.t === "status_features") {
		return applyStatusFeatures(prev, msg);
	}

	if (msg.t === "status_can") {
		return applyStatusCan(prev, msg);
	}

	if (msg.t === "status_state") {
		return applyStatusState(prev, msg);
	}

	if (msg.t === "status_compact") {
		return applyStatusCompact(prev, msg);
	}

	if (msg.t === "statusLive") {
		return applyStatusLive(prev, msg.on, msg.intervalMs);
	}

	if (msg.t === "frame") {
		return applyFrame(prev, msg, timestamp);
	}

	if (msg.t === "ack") {
		return addNotification(prev, "info", `OK ${msg.cmd}`, nextId(), timestamp);
	}

	if (msg.t === "error") {
		return addNotification(prev, "error", msg.msg || "Error", nextId(), timestamp);
	}

	if (msg.t === "log") {
		const text = msg.msg || "Log";
		let state = addNotification(prev, "info", text, nextId(), timestamp);
		if (text.includes("Summon burst started")) {
			state = { ...state, summonActive: true };
		} else if (text.includes("Summon burst complete") || text.includes("Summon stopped")) {
			state = { ...state, summonActive: false };
		}
		return state;
	}

	if (msg.t === "pong") {
		return addNotification(prev, "info", "Pong received", nextId(), timestamp);
	}

	if (msg.t === "bms") {
		return applyBms(prev, msg);
	}

	if (msg.t === "tpms") {
		return {
			...prev,
			tpmsPressureFL: msg.fl / 100,
			tpmsPressureFR: msg.fr / 100,
			tpmsPressureRL: msg.rl / 100,
			tpmsPressureRR: msg.rr / 100,
			tpmsTempFL: msg.tfl,
			tpmsTempFR: msg.tfr,
			tpmsTempRL: msg.trl,
			tpmsTempRR: msg.trr,
			hasTpms: Boolean(msg.ok),
		};
	}

	if (msg.t === "powertrain") {
		return {
			...prev,
			vehicleSpeed: msg.speed / 100,
			gearState: msg.gear,
			accelPedal: msg.pedal,
			brakePedalState: msg.brake,
			steeringAngle: msg.steer / 10,
			rearMotorRpm: msg.rpmR,
			frontMotorRpm: msg.rpmF,
			wheelSpeedFL: msg.wsFL / 100,
			wheelSpeedFR: msg.wsFR / 100,
			wheelSpeedRL: msg.wsRL / 100,
			wheelSpeedRR: msg.wsRR / 100,
			hasWheelSpeeds: Boolean(msg.hasWs),
			rearInvTemp: msg.rInvT,
			rearStatorTemp: msg.rStatT,
			rearHeatsinkTemp: msg.rHsT,
			frontInvTemp: msg.fInvT,
			frontStatorTemp: msg.fStatT,
			frontHeatsinkTemp: msg.fHsT,
			hasMotorTemps: Boolean(msg.hasMotorT),
			hasPowertrain: Boolean(msg.ok),
		};
	}

	if (msg.t === "fwcompat") {
		return {
			...prev,
			fwYear: msg.year,
			fwRelease: msg.release,
			fwMinor: msg.minor,
			fwCompat: msg.compat,
			hasFwVersion: Boolean(msg.ok),
		};
	}

	if (msg.t === "vehicle") {
		return {
			...prev,
			vehicleModel: msg.model,
			vehicleYear: msg.year,
			hasVehicleConfig: Boolean(msg.ok),
		};
	}

	return prev;
}
