/** Shared types for TeslaCANModder firmware protocol. */

// ── Board Message Types (discriminated union on `t` field) ──────────────────

export interface BoardFeatures {
	fsd: boolean;
	fsdForce: boolean;
	offset: boolean;
	profile: boolean;
	nag: boolean;
	isaSpeedChime: boolean;
	summon: boolean;
	eap?: boolean;
	evd?: boolean;
	tlssc?: boolean;
}

type WireBool = boolean | number;
type CanHealthKey = "chassis" | "vehicle" | "body";
type CanHealthMessage = Partial<Record<CanHealthKey, { on: WireBool; det: WireBool }>>;
type CanHealthState = Partial<Record<CanHealthKey, { on: boolean; det: boolean }>>;
type CanBusMetrics = Partial<Record<CanHealthKey, number>>;

interface StatusMetaPayload {
	hw?: string;
	can?: string;
	drv?: string;
	variant?: string;
	cap?: string;
	ready?: string;
	up?: number;
}

interface StatusProfilePayload {
	value?: number;
	pinned?: WireBool;
}

interface StatusOffsetPayload {
	value?: number;
	pinned?: WireBool;
}

interface StatusStatePayload {
	fsd?: WireBool;
	fsdForce?: WireBool;
	nag?: WireBool;
	nagKiller?: WireBool;
	profile?: StatusProfilePayload | number;
	offset?: StatusOffsetPayload | number;
	precondition?: WireBool;
	trackMode?: WireBool;
	apGateEnabled?: WireBool;
	apGateOpen?: WireBool;
	apGateReason?: string;
	lhdEnabled?: WireBool;
	apFirstEnabled?: WireBool;
	laneGraphEnabled?: WireBool;
	assistNavEnable?: WireBool;
	assistHandsOff?: WireBool;
	assistDevMode?: WireBool;
	assistTelemetryOff?: WireBool;
	nagOrgBypass?: WireBool;
	rawCan?: WireBool;
}

interface StatusCanClockPayload {
	reqMHz?: number;
	activeMHz?: number;
}

interface StatusCanPayload {
	clock?: StatusCanClockPayload;
	health?: CanHealthMessage;
	nagEchoCount?: number;
	eapModCount?: number;
	txFailCount?: number;
	busOffCount?: number;
	frames?: CanBusMetrics;
	hz?: CanBusMetrics;
	hzMin?: CanBusMetrics;
	hzMax?: CanBusMetrics;
}

interface StatusCompactConnectivityPayload {
	bt?: WireBool;
	wifi?: WireBool;
	chassisOnline?: WireBool;
	vehicleOnline?: WireBool;
	bodyOnline?: WireBool;
	standby?: WireBool;
}

export interface BootMessage {
	t: "boot";
	variant?: string;
	hw?: string;
	drv?: string;
	features?: BoardFeatures;
	fsd?: number;
	fsdForce?: number;
	nag?: number;
	sp?: number;
	spPin?: number;
	offset?: number;
	offsetPin?: number;
	isaChime?: number;
	summonInject?: number;
	nagKiller?: number;
	nagKillerMode?: string;
	nagOrgBypass?: number;
	dasHandsOn?: number;
	turnSignalLeft?: number;
	turnSignalRight?: number;
	bsmLeftLevel?: number;
	bsmRightLevel?: number;
	doorFrontLeftOpen?: number;
	doorFrontRightOpen?: number;
	doorRearLeftOpen?: number;
	doorRearRightOpen?: number;
	driverDoorOpen?: number;
	anyDoorOpen?: number;
	frunkOpen?: number;
	trunkOpen?: number;
	cruiseSetSpeed?: number;
	accSpeedLimit?: number;
	mapSpeedLimit?: number;
	maxSpeed?: number;
	precondition?: number;
	trackMode?: number;
	apGateEnabled?: number;
	apGateOpen?: number;
	apGateReason?: string;
	detectedHW?: number;
	variantAutoDetect?: number;
	gtwAutopilotTier?: number;
	rawCan?: number;
	canClockReqMHz?: number;
	canClockMHz?: number;
	banShield?: number;
	banThreat?: number;
	banDetectCount?: number;
	gtwShieldArmed?: number;
	gtwShieldBlocks?: number;
	eap?: number;
	evd?: number;
	tlssc?: number;
	bmsNomFullPack?: number;
	bmsNomRemain?: number;
	bmsIdealRemain?: number;
	bmsCellVMax?: number;
	bmsCellVMin?: number;
	bmsMaxRegen?: number;
	bmsMaxDischarge?: number;
	hasEnhancedBms?: number;
	steeringMode?: number;
	hasSteeringMode?: number;
	driveMode?: number;
	currentDriveMode?: number;
	eceR79?: number;
	regionCode?: number;
	hasRegion?: number;
	cnLocked?: number;
	hasTpms?: number;
	regionSpoofCode?: number;
	alcAutoConfirm?: number;
	dasLaneChangeState?: number;
	seatbeltEmulation?: number;
	apFirst?: number;
	dasApState?: number;
	lhdEnabled?: number;
	laneGraphEnabled?: number;
	assistNavEnable?: number;
	assistHandsOff?: number;
	assistDevMode?: number;
	assistTelemetryOff?: number;
	wiperPersist?: number;
	mirrorAutoFold?: number;
	canSim?: number;
	hasPowertrain?: number;
	singleShot?: number;
	fwYear?: number;
	fwRelease?: number;
	fwMinor?: number;
	fwCompat?: number;
	hasFwVersion?: number;
	mqtt?: number;
	mqttConnected?: number;
	vehicleModel?: number;
	vehicleYear?: number;
	hasVehicleConfig?: number;
	platformModel?: number;
	platformHwGen?: number;
	platformSwYear?: number;
	platformSwWeek?: number;
	platformSwRelease?: number;
	platformFsdProto?: number;
	platformSwCompat?: number;
	platformResolved?: number;
	canHealth?: CanHealthMessage;
	canNagEchoCount?: number;
	canEapModCount?: number;
	canTxFailCount?: number;
	canBusOffCount?: number;
	canFrames?: CanBusMetrics;
	canHz?: CanBusMetrics;
	canHzMin?: CanBusMetrics;
	canHzMax?: CanBusMetrics;
	chassisOnline?: number;
	standby?: number;
	vehicleOnline?: number;
	bodyOnline?: number;
	busChassis?: number;
	busVehicle?: number;
	busBody?: number;
	// Button remapping
	btnMapLampShort?: string;
	btnMapLampLong?: string;
	btnMapLampDouble?: string;
	btnMapParkShort?: string;
	btnMapParkLong?: string;
	btnMapParkDouble?: string;
	hasBtnMap?: number;
	// Speed camera alert BLE
	speedAlert?: number;
	// GVRET TCP gateway
	gvret?: number;
	gvretPort?: number;
	gvretClients?: number;
	// ESP-NOW
	espNow?: number;
	espNowChannel?: number;
	espNowPeers?: number;
	// ScanMyTesla BT bridge
	scanMyTesla?: number;
	// ELM327 emulation
	elm327?: number;
	// Tesla BLE Vehicle Control (4.4)
	teslaBle?: number;
	teslaBleConnected?: number;
	teslaBleAuth?: number;
	// Home Assistant / ESPHome (4.5)
	homeAssistant?: number;
	haConnected?: number;
	haEntities?: number;
	haInterval?: number;
	// Encrypted BLE Multi-Device (4.6)
	bleEncrypt?: number;
	bleEncryptPaired?: number;
}

export interface StatusMessage {
	t: "status";
	variant?: string;
	hw?: string;
	drv?: string;
	up?: number;
	rate?: number;
	gtwAutopilotTier?: number;
	canClockReqMHz?: number;
	canClockMHz?: number;
	banShield?: number;
	banThreat?: number;
	banDetectCount?: number;
	gtwShieldArmed?: number;
	gtwShieldBlocks?: number;
	eap?: number;
	evd?: number;
	tlssc?: number;
	driveMode?: number;
	currentDriveMode?: number;
	eceR79?: number;
	regionCode?: number;
	hasRegion?: number;
	cnLocked?: number;
	hasTpms?: number;
	regionSpoofCode?: number;
	alcAutoConfirm?: number;
	dasLaneChangeState?: number;
	seatbeltEmulation?: number;
	apFirst?: number;
	dasApState?: number;
	lhdEnabled?: number;
	laneGraphEnabled?: number;
	assistNavEnable?: number;
	assistHandsOff?: number;
	assistDevMode?: number;
	assistTelemetryOff?: number;
	wiperPersist?: number;
	mirrorAutoFold?: number;
	canSim?: number;
	hasPowertrain?: number;
	singleShot?: number;
	fwYear?: number;
	fwRelease?: number;
	fwMinor?: number;
	fwCompat?: number;
	hasFwVersion?: number;
	mqtt?: number;
	mqttConnected?: number;
	vehicleModel?: number;
	vehicleYear?: number;
	hasVehicleConfig?: number;
	platformModel?: number;
	platformHwGen?: number;
	platformSwYear?: number;
	platformSwWeek?: number;
	platformSwRelease?: number;
	platformFsdProto?: number;
	platformSwCompat?: number;
	platformResolved?: number;
	canHealth?: CanHealthMessage;
	canNagEchoCount?: number;
	canEapModCount?: number;
	canTxFailCount?: number;
	canBusOffCount?: number;
	canFrames?: CanBusMetrics;
	canHz?: CanBusMetrics;
	canHzMin?: CanBusMetrics;
	canHzMax?: CanBusMetrics;
	bmsNomFullPack?: number;
	bmsNomRemain?: number;
	bmsIdealRemain?: number;
	bmsCellVMax?: number;
	bmsCellVMin?: number;
	bmsMaxRegen?: number;
	bmsMaxDischarge?: number;
	hasEnhancedBms?: number;
	steeringMode?: number;
	hasSteeringMode?: number;
	fsd?: number;
	fsdForce?: number;
	nag?: number;
	sp?: number;
	spPin?: number;
	offset?: number;
	offsetPin?: number;
	isaChime?: number;
	summonInject?: number;
	nagKiller?: number;
	nagKillerMode?: string;
	nagOrgBypass?: number;
	dasHandsOn?: number;
	turnSignalLeft?: number;
	turnSignalRight?: number;
	bsmLeftLevel?: number;
	bsmRightLevel?: number;
	doorFrontLeftOpen?: number;
	doorFrontRightOpen?: number;
	doorRearLeftOpen?: number;
	doorRearRightOpen?: number;
	driverDoorOpen?: number;
	anyDoorOpen?: number;
	frunkOpen?: number;
	trunkOpen?: number;
	cruiseSetSpeed?: number;
	accSpeedLimit?: number;
	mapSpeedLimit?: number;
	maxSpeed?: number;
	precondition?: number;
	trackMode?: number;
	apGateEnabled?: number;
	apGateOpen?: number;
	apGateReason?: string;
	detectedHW?: number;
	variantAutoDetect?: number;
	rawCan?: number;
	stream?: { on: WireBool; emitted: number };
	features?: BoardFeatures;
	chassisOnline?: number;
	standby?: number;
	vehicleOnline?: number;
	bodyOnline?: number;
	busChassis?: number;
	busVehicle?: number;
	busBody?: number;
	// Button remapping
	btnMapLampShort?: string;
	btnMapLampLong?: string;
	btnMapLampDouble?: string;
	btnMapParkShort?: string;
	btnMapParkLong?: string;
	btnMapParkDouble?: string;
	hasBtnMap?: number;
	// Speed camera alert BLE
	speedAlert?: number;
	// GVRET TCP gateway
	gvret?: number;
	gvretPort?: number;
	gvretClients?: number;
	// ESP-NOW
	espNow?: number;
	espNowChannel?: number;
	espNowPeers?: number;
	// ScanMyTesla BT bridge
	scanMyTesla?: number;
	// ELM327 emulation
	elm327?: number;
	// Tesla BLE Vehicle Control (4.4)
	teslaBle?: number;
	teslaBleConnected?: number;
	teslaBleAuth?: number;
	// Home Assistant / ESPHome (4.5)
	homeAssistant?: number;
	haConnected?: number;
	haEntities?: number;
	haInterval?: number;
	// Encrypted BLE Multi-Device (4.6)
	bleEncrypt?: number;
	bleEncryptPaired?: number;
}

export interface PlatformMessage {
	t: "platform";
	model: number;
	hwGen: number;
	swYear: number;
	swWeek: number;
	swRelease: number;
	fsdProto: number;
	swCompat: number;
	resolved: WireBool;
	canHealth: CanHealthMessage;
}

export interface StatusLiveMessage {
	t: "statusLive";
	on: WireBool;
	intervalMs: number;
}

export interface StatusMetaMessage extends StatusMetaPayload {
	t: "status_meta";
}

export interface StatusFeaturesMessage {
	t: "status_features";
	features?: BoardFeatures;
}

export interface StatusCanMessage {
	t: "status_can";
	clock?: StatusCanClockPayload;
	health?: CanHealthMessage;
	canClockReqMHz?: number;
	canClockMHz?: number;
	canHealth?: CanHealthMessage;
}

export interface StatusStateMessage {
	t: "status_state";
	state?: StatusStatePayload;
	fsd?: WireBool;
	fsdForce?: WireBool;
	nag?: WireBool;
	nagKiller?: WireBool;
	nagOrgBypass?: WireBool;
	rawCan?: WireBool;
	sp?: number;
	spPin?: WireBool;
	offset?: number;
	offsetPin?: WireBool;
	precondition?: WireBool;
	trackMode?: WireBool;
	apGateEnabled?: WireBool;
	apGateOpen?: WireBool;
	apGateReason?: string;
	lhdEnabled?: WireBool;
	apFirstEnabled?: WireBool;
	laneGraphEnabled?: WireBool;
	assistNavEnable?: WireBool;
	assistHandsOff?: WireBool;
	assistDevMode?: WireBool;
	assistTelemetryOff?: WireBool;
}

export interface StatusCompactMessage {
	t: "status_compact";
	meta?: StatusMetaPayload;
	connectivity?: StatusCompactConnectivityPayload;
	state?: StatusStatePayload;
	features?: BoardFeatures;
	can?: StatusCanPayload;
	stream?: { on: WireBool; emitted: number };
	rawCan?: WireBool;
	variant?: string;
	hw?: string;
	ready?: string;
	up?: number;
	chassisOnline?: WireBool;
	vehicleOnline?: WireBool;
	bodyOnline?: WireBool;
	standby?: WireBool;
	fsd?: WireBool;
	fsdForce?: WireBool;
	nag?: WireBool;
	sp?: number;
	offset?: number;
	canClockReqMHz?: number;
	canClockMHz?: number;
	canHealth?: CanHealthMessage;
}

export interface FrameMessage {
	t: "frame";
	id: number;
	dir: string;
	dlc: number;
	seq?: number;
	ms?: number;
	ext?: number;
	d: string;
	bus: number;
}

export interface AckMessage {
	t: "ack";
	cmd: string;
}

export interface ErrorMessage {
	t: "error";
	msg: string;
}

export interface LogMessage {
	t: "log";
	msg: string;
}

export interface PongMessage {
	t: "pong";
}

export interface BmsMessage {
	t: "bms";
	v: number; // voltage * 100
	a: number; // current * 10
	kw: number; // power * 10
	soc: number; // SoC * 10
	tMin: number; // temp min (°C)
	tMax: number; // temp max (°C)
	whkm: number; // Wh/km * 10
	nomFull?: number; // nominal full pack * 100 (kWh)
	nomRemain?: number; // nominal remaining * 100 (kWh)
	idealRemain?: number; // ideal remaining * 100 (kWh)
	cellVMax?: number; // cell voltage max * 1000 (V)
	cellVMin?: number; // cell voltage min * 1000 (V)
	maxRegen?: number; // max regen power * 100 (kW)
	maxDischarge?: number; // max discharge power * 100 (kW)
	enhanced?: number; // 1 = enhanced BMS data available
	ok: number; // 1 = data available
	// Expanded fields
	socUI?: number; // SoC UI * 10
	socMax?: number; // SoC max * 10
	socAvg?: number; // SoC avg * 10
	initFull?: number; // initial full pack * 10 (kWh)
	expRange?: number; // expected range * 10 (km)
	idealRange?: number; // ideal range * 10 (km)
	ratedCons?: number; // rated consumption * 10 (Wh/km)
	actSoc?: number; // actual SoC integer %
	useSoc?: number; // usable SoC integer %
	pwrDiss?: number; // power dissipation * 100 (kW)
	flowReq?: number; // flow request * 10 (LPM)
	coolTgt?: number; // cooling target * 10 (°C)
	heatTgt?: number; // heating target * 10 (°C)
	packTMin?: number; // pack temp min * 10 (°C)
	packTMax?: number; // pack temp max * 10 (°C)
	heatPwr?: number; // stationary heat power * 100 (kW)
	hvacBgt?: number; // HVAC budget * 100 (kW)
	precondOk?: number; // precondition allowed (0/1)
	heatWorth?: number; // heating worthwhile (0/1)
	contState?: number; // contactor state (0-7)
	hvState?: number; // HV bus state (0-7)
	minBusV?: number; // min bus voltage * 100 (V)
	maxBusV?: number; // max bus voltage * 100 (V)
	maxChgA?: number; // max charge current * 10 (A)
	maxDchA?: number; // max discharge current * 10 (A)
	expRemain?: number; // expected remaining * 100 (kWh)
	eBuf?: number; // energy buffer * 100 (kWh)
	eToChg?: number; // energy to charge complete * 100 (kWh)
	charged?: number; // fully charged (0/1)
	kwhDch?: number; // lifetime discharge total (kWh)
	kwhChg?: number; // lifetime charge total (kWh)
	acChg?: number; // AC charge total (kWh)
	dcChg?: number; // DC charge total (kWh)
	regen?: number; // regen total (kWh)
	drvDch?: number; // drive discharge total (kWh)
	chgTime?: number; // charge time to full * 100 (hours)
}

export interface TpmsMessage {
	t: "tpms";
	fl: number; // front-left pressure * 100 (bar)
	fr: number; // front-right pressure * 100 (bar)
	rl: number; // rear-left pressure * 100 (bar)
	rr: number; // rear-right pressure * 100 (bar)
	tfl: number; // front-left temp (°C)
	tfr: number; // front-right temp (°C)
	trl: number; // rear-left temp (°C)
	trr: number; // rear-right temp (°C)
	ok: number; // 1 = data available
}

export interface PowertrainMessage {
	t: "powertrain";
	speed: number; // vehicle speed * 100 (km/h)
	gear: number; // gear state (0=inv, 1=P, 2=R, 3=N, 4=D)
	pedal: number; // accelerator pedal % (0-100)
	brake: number; // brake pedal state (0=off, 1=applied, 2=hard)
	steer: number; // steering angle * 10 (degrees, + = right)
	rpmR: number; // rear motor RPM
	rpmF: number; // front motor RPM
	wsFL: number; // front-left wheel speed * 100 (km/h)
	wsFR: number; // front-right wheel speed * 100 (km/h)
	wsRL: number; // rear-left wheel speed * 100 (km/h)
	wsRR: number; // rear-right wheel speed * 100 (km/h)
	hasWs: number; // 1 = wheel speed data available
	rInvT: number; // rear inverter temperature °C
	rStatT: number; // rear stator temperature °C
	rHsT: number; // rear heatsink temperature °C
	fInvT: number; // front inverter temperature °C (dual-motor only)
	fStatT: number; // front stator temperature °C (dual-motor only)
	fHsT: number; // front heatsink temperature °C (dual-motor only)
	hasMotorT: number; // 1 = motor temperature data available
	ok: number; // 1 = data available
}

export interface FwCompatMessage {
	t: "fwcompat";
	year: number;
	release: number;
	minor: number;
	build: number;
	compat: number; // 0=UNKNOWN, 1=OK, 2=WARN, 3=FAIL
	ok: number; // 1 = data available
}

export interface VehicleConfigMessage {
	t: "vehicle";
	model: number; // 0=UNKNOWN, 1=MODEL_3, 2=MODEL_Y, 3=MODEL_S, 4=MODEL_X, 5=CYBERTRUCK
	year: number;
	ok: number; // 1 = data available
}

export type BoardMessage =
	| BootMessage
	| StatusMessage
	| PlatformMessage
	| StatusLiveMessage
	| StatusMetaMessage
	| StatusFeaturesMessage
	| StatusCanMessage
	| StatusStateMessage
	| StatusCompactMessage
	| FrameMessage
	| AckMessage
	| ErrorMessage
	| LogMessage
	| PongMessage
	| BmsMessage
	| TpmsMessage
	| PowertrainMessage
	| FwCompatMessage
	| VehicleConfigMessage;

// ── Board State ─────────────────────────────────────────────────────────────

export interface CanFrame {
	key: string;
	id: number;
	dir: string;
	bus: number;
	busName: string;
	seq?: number;
	dlc: number;
	data: string;
	ts: string;
}

export interface ConsoleMessage {
	id: number;
	type: "info" | "error";
	text: string;
	ts: string;
}

export interface BoardState {
	variant: string;
	hardware: string;
	driver: string;
	board: "arduino" | "esp32" | "unknown";
	uptime: number;
	rate: number;

	fsd: boolean;
	fsdForce: boolean;
	nag: boolean;
	profile: number;
	profilePinned: boolean;
	offset: number;
	offsetPinned: boolean;
	isaChime: boolean;
	summonInject: boolean;
	summonActive: boolean;

	nagKiller: boolean;
	nagKillerMode: string;
	nagOrgBypass: boolean;
	rawCan: boolean;
	dasHandsOn: number;
	turnSignalLeft: boolean;
	turnSignalRight: boolean;
	bsmLeftLevel: number;
	bsmRightLevel: number;
	doorFrontLeftOpen: boolean;
	doorFrontRightOpen: boolean;
	doorRearLeftOpen: boolean;
	doorRearRightOpen: boolean;
	driverDoorOpen: boolean;
	anyDoorOpen: boolean;
	frunkOpen: boolean;
	trunkOpen: boolean;
	cruiseSetSpeedKph: number;
	accSpeedLimitKph: number;
	mapSpeedLimitKph: number;
	maxSpeedKph: number;
	precondition: boolean;
	trackMode: boolean;
	apGateEnabled: boolean;
	apGateOpen: boolean;
	apGateReason: string;
	detectedHW: number;
	variantAutoDetect: boolean;
	gtwAutopilotTier: number;
	canClockReqMHz: number;
	canClockMHz: number;
	banShield: boolean;
	banThreat: number;
	banDetectCount: number;
	gtwShieldArmed: boolean;
	gtwShieldBlocks: number;
	enhancedAutopilot: boolean;
	evdEnabled: boolean;
	tlsscRestore: boolean;

	// TPMS tire pressure
	tpmsPressureFL: number;
	tpmsPressureFR: number;
	tpmsPressureRL: number;
	tpmsPressureRR: number;
	tpmsTempFL: number;
	tpmsTempFR: number;
	tpmsTempRL: number;
	tpmsTempRR: number;
	hasTpms: boolean;

	// Drive mode override
	driveMode: number;
	currentDriveMode: number;

	// Region detection
	regionCode: number;
	hasRegion: boolean;
	cnLocked: boolean;
	regionSpoofCode: number;

	// Auto Lane Change auto-confirm
	alcAutoConfirm: boolean;
	dasLaneChangeState: number;

	// ECE R79 bypass
	eceR79: boolean;

	// New vehicle features
	seatbeltEmulation: boolean;
	wiperPersist: boolean;
	mirrorAutoFold: boolean;
	canSim: boolean;

	// AP-First mode (2026.14.x compatibility)
	apFirstEnabled: boolean;
	dasApState: number;

	// LHD + Assist features
	lhdEnabled: boolean;
	laneGraphEnabled: boolean;
	assistNavEnable: boolean;
	assistHandsOff: boolean;
	assistDevMode: boolean;
	assistTelemetryOff: boolean;

	// Single-shot TX
	singleShot: boolean;

	// Button remapping (2.2)
	btnMapLampShort: string;
	btnMapLampLong: string;
	btnMapLampDouble: string;
	btnMapParkShort: string;
	btnMapParkLong: string;
	btnMapParkDouble: string;
	hasBtnMap: boolean;

	// Speed camera alert BLE (2.8)
	speedAlert: boolean;

	// GVRET TCP gateway (3.5)
	gvret: boolean;
	gvretPort: number;
	gvretClients: number;

	// ESP-NOW multi-device (4.1)
	espNow: boolean;
	espNowChannel: number;
	espNowPeers: number;

	// ScanMyTesla BT bridge (4.2)
	scanMyTesla: boolean;

	// ELM327 emulation (4.3)
	elm327: boolean;

	// Tesla BLE Vehicle Control (4.4)
	teslaBle: boolean;
	teslaBleConnected: boolean;
	teslaBleAuth: boolean;

	// Home Assistant / ESPHome (4.5)
	homeAssistant: boolean;
	haConnected: boolean;
	haEntities: number;
	haInterval: number;

	// Encrypted BLE Multi-Device (4.6)
	bleEncrypt: boolean;
	bleEncryptPaired: number;

	// Firmware version compatibility
	fwYear: number;
	fwRelease: number;
	fwMinor: number;
	fwCompat: number;
	hasFwVersion: boolean;

	// MQTT bridge
	mqtt: boolean;
	mqttConnected: boolean;

	// Vehicle configuration
	vehicleModel: number;
	vehicleYear: number;
	hasVehicleConfig: boolean;

	// Vehicle platform identity (Model → HW → SW)
	platformModel: number;
	platformHwGen: number;
	platformSwYear: number;
	platformSwWeek: number;
	platformSwRelease: number;
	platformFsdProto: number;
	platformSwCompat: number;
	platformResolved: boolean;

	// CAN bus health (per-bus MCP2515 status)
	canHealth: CanHealthState;

	// CAN diagnostic counters (firmware CanDiag aggregate)
	canNagEchoCount: number;
	canEapModCount: number;
	canTxFailCount: number;
	canBusOffCount: number;

	// CAN per-bus frame counts and rate (uint16_t * 10 on wire for hz* fields)
	canFrames: CanBusMetrics;
	canHz: CanBusMetrics;
	canHzMin: CanBusMetrics;
	canHzMax: CanBusMetrics;

	// Powertrain telemetry
	vehicleSpeed: number;
	gearState: number;
	accelPedal: number;
	brakePedalState: number;
	steeringAngle: number;
	rearMotorRpm: number;
	frontMotorRpm: number;
	hasPowertrain: boolean;

	// Wheel speeds (0x175, ChassisBus)
	wheelSpeedFL: number;
	wheelSpeedFR: number;
	wheelSpeedRL: number;
	wheelSpeedRR: number;
	hasWheelSpeeds: boolean;

	// Motor / inverter temperatures
	rearInvTemp: number;
	rearStatorTemp: number;
	rearHeatsinkTemp: number;
	frontInvTemp: number;
	frontStatorTemp: number;
	frontHeatsinkTemp: number;
	hasMotorTemps: boolean;

	// BMS battery telemetry
	bmsVoltage: number;
	bmsCurrent: number;
	bmsPower: number;
	bmsSoc: number;
	bmsTempMin: number;
	bmsTempMax: number;
	bmsWhPerKm: number;
	hasBms: boolean;

	// Enhanced BMS telemetry
	bmsNominalFullPack: number;
	bmsNominalRemaining: number;
	bmsIdealRemaining: number;
	bmsCellVoltageMax: number;
	bmsCellVoltageMin: number;
	bmsMaxRegenPower: number;
	bmsMaxDischargePower: number;
	hasEnhancedBms: boolean;

	// Expanded SoC
	bmsSocUI: number;
	bmsSocMax: number;
	bmsSocAvg: number;
	bmsInitialFullPack: number;

	// Range & consumption
	bmsExpectedRange: number;
	bmsIdealRange: number;
	bmsRatedConsumption: number;
	bmsActualSocInt: number;
	bmsUsableSocInt: number;

	// Thermal
	bmsPowerDissipation: number;
	bmsFlowRequest: number;
	bmsCoolTarget: number;
	bmsHeatTarget: number;
	bmsPackTMin: number;
	bmsPackTMax: number;
	bmsThermistorTMin: number;
	bmsThermistorTMax: number;
	bmsModelTMin: number;
	bmsModelTMax: number;

	// Power budget
	bmsStationaryHeatPower: number;
	bmsHvacPowerBudget: number;

	// BMS status
	bmsPrecondAllowed: boolean;
	bmsHeatingWorthwhile: boolean;
	bmsContactorState: number;
	bmsHvState: number;

	// Drive limits
	bmsMinBusVoltage: number;
	bmsMaxBusVoltage: number;
	bmsMaxChargeCurrent: number;
	bmsMaxDischargeCurrent: number;

	// Energy status mux=1
	bmsExpectedRemaining: number;
	bmsEnergyBuffer: number;
	bmsEnergyToCharge: number;
	bmsFullyCharged: boolean;

	// Lifetime counters
	bmsKwhDischargeTotal: number;
	bmsKwhChargeTotal: number;
	bmsAcChargeTotal: number;
	bmsDcChargeTotal: number;
	bmsRegenTotal: number;
	bmsDriveDischargeTotal: number;

	// Charge time
	bmsChargeTimeToFull: number;

	// Steering mode
	steeringMode: number;
	hasSteeringMode: boolean;

	chassisOnline: boolean;
	standby: boolean;
	vehicleOnline: boolean;
	bodyOnline: boolean;
	busChassis: boolean;
	busVehicle: boolean;
	busBody: boolean;
	statusLiveEnabled: boolean;
	statusLiveIntervalMs: number;

	streaming: boolean;
	frames: CanFrame[];
	frameCount: number;

	messages: ConsoleMessage[];

	features: BoardFeatures;
}

// ── Transport Types ─────────────────────────────────────────────────────────

export interface TransportEvents {
	onMessage: (msg: Record<string, unknown>) => void;
	onDisconnect: () => void;
	onError: (err: Error) => void;
}

export interface Transport {
	readonly type: "ble" | "serial";
	readonly connected: boolean;
	readonly deviceName: string | null;
	connect(): Promise<void>;
	disconnect(): Promise<void>;
	send(command: string): Promise<void>;
	setListeners(events: TransportEvents): void;
}

export interface ScannedDevice {
	id: string;
	name: string | null;
	rssi: number | null;
	serviceUuids?: string[];
}

// ── Parser Types ────────────────────────────────────────────────────────────

export interface ParsedEvent {
	type: "message" | "ignore" | "parse-error";
	message?: Record<string, unknown>;
	raw: string;
	reason?: string;
}
