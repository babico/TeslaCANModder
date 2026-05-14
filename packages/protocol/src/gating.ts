import { commands } from "./commands.js";
import type { BoardState } from "./types.js";

/**
 * Returned by getCommandGate to indicate whether a command can be executed.
 */
export interface CommandGate {
	/** Whether the command is available */
	available: boolean;
	/** Human-readable reason if unavailable, null if available */
	reason: string | null;
}

/**
 * Type-safe command names derived from the commands registry.
 */
export type CommandName = keyof typeof commands;

/**
 * Gate requirements for a specific command.
 * Each gate specifies a condition and the message if it fails.
 */
interface Gate {
	check: (state: BoardState) => boolean;
	message: string;
}

/**
 * Command gating rules matrix.
 * Maps command names to arrays of gate checks that must all pass.
 * If a command is not listed, it has no restrictions (always available).
 */
const COMMAND_GATES: Record<string, Gate[]> = {
	// System / Info commands (no gates)
	// ping, status, variant, fwCompat, vehicle, platform, log all have no restrictions

	// FSD control (requires chassis bus)
	fsd: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	fsdForce: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],

	// Nag (requires chassis bus)
	nag: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],

	// Speed profile (requires chassis + feature)
	profile: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
		{
			check: (s) => s.features.profile,
			message: "Speed Profile feature not supported by this firmware",
		},
	],
	profileAuto: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
		{
			check: (s) => s.features.profile,
			message: "Speed Profile feature not supported by this firmware",
		},
	],
	profileLock: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
		{
			check: (s) => s.features.profile,
			message: "Speed Profile feature not supported by this firmware",
		},
	],
	profileUnlock: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
		{
			check: (s) => s.features.profile,
			message: "Speed Profile feature not supported by this firmware",
		},
	],

	// Speed offset (requires chassis + feature)
	offset: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
		{
			check: (s) => s.features.offset,
			message: "Speed Offset feature not supported by this firmware",
		},
	],
	offsetAuto: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
		{
			check: (s) => s.features.offset,
			message: "Speed Offset feature not supported by this firmware",
		},
	],
	offsetOff: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
		{
			check: (s) => s.features.offset,
			message: "Speed Offset feature not supported by this firmware",
		},
	],

	// ISA Chime (HW4, requires chassis)
	isaChime: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
		{
			check: (s) => s.features.isaSpeedChime,
			message: "ISA Speed Chime feature not supported by this firmware",
		},
	],

	// Summon (requires chassis + feature)
	summonInject: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
		{
			check: (s) => s.features.summon,
			message: "Summon feature not supported by this firmware",
		},
	],
	summon: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
		{
			check: (s) => s.features.summon,
			message: "Summon feature not supported by this firmware",
		},
	],
	summonForward: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
		{
			check: (s) => s.features.summon,
			message: "Summon feature not supported by this firmware",
		},
	],
	summonReverse: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
		{
			check: (s) => s.features.summon,
			message: "Summon feature not supported by this firmware",
		},
	],
	summonStop: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
		{
			check: (s) => s.features.summon,
			message: "Summon feature not supported by this firmware",
		},
	],

	// Streaming / CAN access (requires chassis)
	stream: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	rawCan: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	canClockAuto: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	canClock: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],

	// BMS (requires hardware support)
	bms: [
		{
			check: (s) => s.hasBms !== false,
			message: "BMS hardware not available or not enabled",
		},
	],

	// Ban Shield (requires chassis)
	banShield: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	gtwShieldArm: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	gtwShieldDisarm: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	gtwShieldReset: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	tlssc: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	eap: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	evd: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],

	// Preconditioning (requires chassis)
	precondition: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],

	// Track Mode (requires chassis)
	trackMode: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],

	// Mirror (requires body bus)
	mirrorFold: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	mirrorUnfold: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	mirrorHeat: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	mirrorAutofold: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	mirrorDip: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	mirrorAutoFold: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
		{
			check: (s) => s.mirrorAutoFold,
			message: "Mirror auto-fold feature not enabled",
		},
	],

	// Lock/Unlock (requires chassis)
	lock: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	unlock: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	lockChild: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],

	// Horn (requires chassis)
	horn: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],

	// Trunk/Frunk (requires body)
	frunkOpen: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	frunkClose: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	trunkOpen: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	trunkClose: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	glovebox: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],

	// Lights (requires body)
	lightFogFront: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	lightFogRear: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	lightHighbeamAuto: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	lightAmbient: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	lightHome: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	lightDomeOff: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	lightDomeOn: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	lightDomeAuto: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],

	// Wiper (requires body)
	wiperOff: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	wiper1: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	wiper2: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	wiper3: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],

	// Seat heating (requires vehicle or body)
	seatFL: [
		{
			check: (s) => s.vehicleOnline || s.bodyOnline,
			message: "Vehicle or Body CAN bus not available",
		},
	],
	seatFR: [
		{
			check: (s) => s.vehicleOnline || s.bodyOnline,
			message: "Vehicle or Body CAN bus not available",
		},
	],
	seatRL: [
		{
			check: (s) => s.vehicleOnline || s.bodyOnline,
			message: "Vehicle or Body CAN bus not available",
		},
	],
	seatRR: [
		{
			check: (s) => s.vehicleOnline || s.bodyOnline,
			message: "Vehicle or Body CAN bus not available",
		},
	],
	seatRC: [
		{
			check: (s) => s.vehicleOnline || s.bodyOnline,
			message: "Vehicle or Body CAN bus not available",
		},
	],

	// Display (requires body)
	mainDisplay: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],

	// Power (requires chassis)
	powerAccOn: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	powerAccOff: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	powerOff: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	powerReady: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],

	// Window (requires body)
	windowVentOpen: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	windowVentClose: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	ventOpen: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	ventClose: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	windowVent: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],

	// Sentry (requires chassis)
	sentryOn: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	sentryOff: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],

	// Climate (requires chassis)
	climateKeep: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	climateOff: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],

	// Charge (requires chassis)
	chargeStart: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	chargeStop: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	chargePort: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],

	// Drive Config (requires chassis)
	pedalStandard: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	pedalChill: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	pedalSport: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	regenOff: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	regenLow: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	regenStd: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	regenMax: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	stopCreep: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	stopRoll: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	stopHold: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],

	// Drive Mode Override (requires chassis)
	driveModeOff: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	driveModeChill: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	driveModeStandard: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],
	driveModePerformance: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],

	// TPMS (requires hardware support)
	tpms: [
		{
			check: (s) => s.hasTpms,
			message: "TPMS hardware not available or not enabled",
		},
	],

	// ECE R79 (requires vehicle bus)
	eceR79: [
		{
			check: (s) => s.vehicleOnline,
			message: "Vehicle CAN bus not available",
		},
	],

	// Region spoof (requires chassis for VIN/region detection)
	regionSpoof: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],

	// Auto Lane Change auto-confirm (requires chassis)
	alc: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],

	// Rate limiting (requires chassis)
	rateLimit: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
	],

	// Turn signals (requires body)
	turnLeft3: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	turnRight3: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	turnHazard: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],
	turnOff: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],

	// Seatbelt emulation (requires body)
	seatbelt: [
		{
			check: (s) => s.seatbeltEmulation,
			message: "Seatbelt emulation not enabled",
		},
	],

	// Air recirculation (requires body)
	airRecirc: [
		{
			check: (s) => s.bodyOnline,
			message: "Body CAN bus not available",
		},
	],

	// Wiper persistence (requires body)
	wiperPersist: [
		{
			check: (s) => s.wiperPersist,
			message: "Wiper persistence feature not enabled",
		},
	],

	// Powertrain telemetry (requires hardware support)
	powertrain: [
		{
			check: (s) => s.hasPowertrain,
			message: "Powertrain telemetry hardware not available or not enabled",
		},
	],

	// CAN simulation (requires chassis + feature)
	canSimStart: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
		{
			check: (s) => s.canSim,
			message: "CAN simulation feature not enabled",
		},
	],
	canSimStop: [
		{
			check: (s) => s.chassisOnline,
			message: "Chassis CAN bus not available",
		},
		{
			check: (s) => s.canSim,
			message: "CAN simulation feature not enabled",
		},
	],

	// Single-shot TX (requires chassis)
	singleShot: [
		{
			check: (s) => s.singleShot,
			message: "Single-shot TX mode not enabled",
		},
	],

	// MQTT (requires wifi or network)
	mqtt: [
		{
			check: (s) => s.mqtt,
			message: "MQTT not supported by this firmware",
		},
	],
	mqttBroker: [
		{
			check: (s) => s.mqtt,
			message: "MQTT not supported by this firmware",
		},
	],
	mqttPort: [
		{
			check: (s) => s.mqtt,
			message: "MQTT not supported by this firmware",
		},
	],
	mqttInterval: [
		{
			check: (s) => s.mqtt,
			message: "MQTT not supported by this firmware",
		},
	],

	// Button remapping (requires feature)
	btnMap: [
		{
			check: (s) => s.hasBtnMap,
			message: "Button remapping not supported by this firmware",
		},
	],
	btnMapQuery: [
		{
			check: (s) => s.hasBtnMap,
			message: "Button remapping not supported by this firmware",
		},
	],
	btnMapReset: [
		{
			check: (s) => s.hasBtnMap,
			message: "Button remapping not supported by this firmware",
		},
	],

	// Speed alert (requires feature)
	speedAlert: [
		{
			check: (s) => s.speedAlert,
			message: "Speed camera alert not supported by this firmware",
		},
	],

	// GVRET gateway (requires feature)
	gvret: [
		{
			check: (s) => s.gvret,
			message: "GVRET TCP gateway not supported by this firmware",
		},
	],
	gvretPort: [
		{
			check: (s) => s.gvret,
			message: "GVRET TCP gateway not supported by this firmware",
		},
	],

	// ESP-NOW multi-device (requires feature)
	espNow: [
		{
			check: (s) => s.espNow,
			message: "ESP-NOW multi-device not supported by this firmware",
		},
	],
	espNowChannel: [
		{
			check: (s) => s.espNow,
			message: "ESP-NOW multi-device not supported by this firmware",
		},
	],

	// ScanMyTesla BT bridge (requires feature)
	scanMyTesla: [
		{
			check: (s) => s.scanMyTesla,
			message: "ScanMyTesla BT bridge not supported by this firmware",
		},
	],

	// ELM327 emulation (requires feature)
	elm327: [
		{
			check: (s) => s.elm327,
			message: "ELM327 emulation not supported by this firmware",
		},
	],

	// Tesla BLE Vehicle Control (requires feature)
	teslaBle: [
		{
			check: (s) => s.teslaBle,
			message: "Tesla BLE Vehicle Control not supported by this firmware",
		},
	],
	teslaBleAuth: [
		{
			check: (s) => s.teslaBle,
			message: "Tesla BLE Vehicle Control not supported by this firmware",
		},
	],
	teslaBleForget: [
		{
			check: (s) => s.teslaBle,
			message: "Tesla BLE Vehicle Control not supported by this firmware",
		},
	],

	// Home Assistant integration (requires feature)
	homeAssistant: [
		{
			check: (s) => s.homeAssistant,
			message: "Home Assistant integration not supported by this firmware",
		},
	],
	haDiscovery: [
		{
			check: (s) => s.homeAssistant,
			message: "Home Assistant integration not supported by this firmware",
		},
	],
	haInterval: [
		{
			check: (s) => s.homeAssistant,
			message: "Home Assistant integration not supported by this firmware",
		},
	],

	// Encrypted BLE Multi-Device (requires feature)
	bleEncrypt: [
		{
			check: (s) => s.bleEncrypt,
			message: "Encrypted BLE multi-device not supported by this firmware",
		},
	],
	blePair: [
		{
			check: (s) => s.bleEncrypt,
			message: "Encrypted BLE multi-device not supported by this firmware",
		},
	],
	bleUnpair: [
		{
			check: (s) => s.bleEncrypt,
			message: "Encrypted BLE multi-device not supported by this firmware",
		},
	],
};

/**
 * Query whether a command can be executed given the current board state.
 *
 * @param name The command name
 * @param state The current board state
 * @returns A gate object indicating availability and reason if blocked
 */
export function getCommandGate(name: CommandName, state: BoardState): CommandGate {
	const gates = COMMAND_GATES[name];

	// No gates defined = always available
	if (!gates) {
		return { available: true, reason: null };
	}

	// Check all gates; return on first failure
	for (const gate of gates) {
		if (!gate.check(state)) {
			return { available: false, reason: gate.message };
		}
	}

	// All gates passed
	return { available: true, reason: null };
}
