/** Command builders for all TeslaCANModder firmware commands. */

/** Valid firmware variants. */
export const VALID_VARIANTS = ["hw3", "hw4", "legacy", "auto"] as const;
export type Variant = (typeof VALID_VARIANTS)[number];
export const VALID_NAG_MODES = [
	"off",
	"bit19",
	"legacy",
	"safe",
	"natural",
	"organic",
	"full",
] as const;
export type NagMode = (typeof VALID_NAG_MODES)[number];

/** Valid region spoof codes. */
export const VALID_REGION_SPOOF_CODES = ["na", "eu", "cn", "apac", "me", "off"] as const;
export type RegionSpoofCode = (typeof VALID_REGION_SPOOF_CODES)[number];

/** Valid button names for button remapping. */
export const VALID_BUTTONS = ["lamp", "parking"] as const;
export type ButtonName = (typeof VALID_BUTTONS)[number];

/** Valid press types for button actions. */
export const VALID_PRESS_TYPES = ["short", "long", "double"] as const;
export type PressType = (typeof VALID_PRESS_TYPES)[number];

/** Valid button actions for remapping. */
export const VALID_BUTTON_ACTIONS = [
	"none",
	"trunk",
	"frunk",
	"sentry",
	"horn",
	"fold",
	"hazard",
	"recirc",
] as const;
export type ButtonAction = (typeof VALID_BUTTON_ACTIONS)[number];

/** Range limits for numeric command parameters. */
export const COMMAND_RANGES = {
	profile: { min: 0, max: 4 },
	offset: { min: 0, max: 100 },
	canClockMHz: { min: 8, max: 20 },
	seat: { min: 0, max: 3 },
	mainDisplay: { min: 0, max: 127 },
	windowVent: { min: 0, max: 100 },
	gvretPort: { min: 1, max: 65535 },
	espNowChannel: { min: 1, max: 13 },
	haInterval: { min: 500, max: 60000 },
} as const;

function assertRange(name: string, value: number, min: number, max: number): void {
	if (!Number.isInteger(value) || value < min || value > max) {
		throw new RangeError(`${name} must be an integer between ${min} and ${max}, got ${value}`);
	}
}

function assertInList<const T extends readonly string[]>(
	name: string,
	value: string,
	valid: T,
): asserts value is T[number] {
	if (!valid.includes(value)) {
		throw new RangeError(`${name} must be one of ${valid.join(", ")}, got "${value}"`);
	}
}

function assertVariant(v: string): asserts v is Variant {
	assertInList("variant", v, VALID_VARIANTS);
}

function assertNagMode(m: string): asserts m is NagMode {
	assertInList("nag mode", m, VALID_NAG_MODES);
}

function assertRegionSpoofCode(c: string): asserts c is RegionSpoofCode {
	assertInList("region spoof code", c, VALID_REGION_SPOOF_CODES);
}

function assertButtonName(b: string): asserts b is ButtonName {
	assertInList("button", b, VALID_BUTTONS);
}

function assertPressType(p: string): asserts p is PressType {
	assertInList("press type", p, VALID_PRESS_TYPES);
}

function assertButtonAction(a: string): asserts a is ButtonAction {
	assertInList("button action", a, VALID_BUTTON_ACTIONS);
}

export const commands = {
	// System
	ping: () => "ping",
	status: () => "status",
	statusLive: (on: boolean) => (on ? "status:live:on" : "status:live:off"),
	statusLiveQuery: () => "status:live",
	statusCompact: () => "status:compact",
	statusMeta: () => "status:meta",
	statusState: () => "status:state",
	statusFeatures: () => "status:features",
	statusCan: () => "status:can",
	variant: (v: string) => {
		assertVariant(v);
		return `variant:${v}`;
	},

	// FSD
	fsd: (on: boolean) => (on ? "fsd:on" : "fsd:off"),
	fsdForce: (on: boolean) => (on ? "fsd:force:on" : "fsd:force:off"),

	// Nag
	nagMode: (mode: string) => {
		assertNagMode(mode);
		return `nag:mode:${mode}`;
	},
	nagBypass: (on: boolean) => (on ? "nag:bypass:on" : "nag:bypass:off"),

	// Speed Profile
	profile: (p: number) => {
		assertRange("profile", p, COMMAND_RANGES.profile.min, COMMAND_RANGES.profile.max);
		return `profile:${p}`;
	},
	profileAuto: () => "profile:auto",
	profileLock: () => "profile:lock",
	profileUnlock: () => "profile:unlock",

	// Speed Offset (auto-routes to HW4 range 0-63 or legacy range 0-100 based on detected HW)
	offset: (o: number) => {
		assertRange("offset", o, COMMAND_RANGES.offset.min, COMMAND_RANGES.offset.max);
		return `offset:${o}`;
	},
	offsetAuto: () => "offset:auto",
	offsetOff: () => "offset:off",

	// ISA Chime (HW4)
	isaChime: (on: boolean) => (on ? "isa-chime:on" : "isa-chime:off"),

	// Summon
	summonInject: (on: boolean) => (on ? "summon-inject:on" : "summon-inject:off"),
	summon: () => "summon",
	summonForward: () => "summon:forward",
	summonReverse: () => "summon:reverse",
	summonStop: () => "summon:stop",

	// Streaming
	stream: (on: boolean) => (on ? "stream:on" : "stream:off"),
	rawCan: (on: boolean) => (on ? "can:raw:on" : "can:raw:off"),
	canClockAuto: () => "canclock:auto",
	canClock: (mhz: number) => {
		if (![8, 12, 16, 20].includes(mhz)) {
			throw new RangeError(`canClockMHz must be one of 8, 12, 16, 20, got ${mhz}`);
		}
		return `canclock:${mhz}`;
	},

	// BMS Telemetry
	bms: () => "bms",

	// Ban Shield (experimental telemetry monitoring)
	banShield: (on: boolean) => (on ? "banshield:on" : "banshield:off"),
	gtwShieldArm: () => "gtwshield:arm",
	gtwShieldDisarm: () => "gtwshield:disarm",
	gtwShieldReset: () => "gtwshield:reset",

	// AP config restore / unlock features
	tlssc: (on: boolean) => (on ? "tlssc:on" : "tlssc:off"),
	eap: (on: boolean) => (on ? "eap:on" : "eap:off"),
	evd: (on: boolean) => (on ? "evd:on" : "evd:off"),
	apGate: (on: boolean) => (on ? "apgate:on" : "apgate:off"),
	apGateStatus: () => "apgate:status",

	// Driver Assist toggles
	lhd: (on: boolean) => (on ? "lhd:on" : "lhd:off"),
	apFirstToggle: (on: boolean) => (on ? "apfirst:on" : "apfirst:off"),
	laneGraph: (on: boolean) => (on ? "lanegraph:on" : "lanegraph:off"),
	assistDev: (on: boolean) => (on ? "assist-dev:on" : "assist-dev:off"),
	assistNav: (on: boolean) => (on ? "assist-nav:on" : "assist-nav:off"),
	assistHof: (on: boolean) => (on ? "assist-hof:on" : "assist-hof:off"),
	assistTel: (on: boolean) => (on ? "assist-tel:on" : "assist-tel:off"),

	// DAS Drive (gamepad-driven CAN injection of openpilot-protocol frames)
	// See firmware/lib/vehicle/can/feature/das_drive.h. Speed values are
	// integer km/h; cap is bounded firmware-side to 1..200, user limit to
	// 1..current cap.
	drive: (on: boolean) => (on ? "drive:on" : "drive:off"),
	driveSpeed: (kph: number) => `drive:speed:${Math.max(1, Math.floor(kph))}`,
	driveCap: (kph: number) => `drive:cap:${Math.max(1, Math.min(200, Math.floor(kph)))}`,

	// Preconditioning
	precondition: (on: boolean) => (on ? "precondition:on" : "precondition:off"),

	// Track Mode
	trackMode: (on: boolean) => (on ? "trackmode:on" : "trackmode:off"),

	// Mirror
	mirrorFold: () => "mirror:fold",
	mirrorUnfold: () => "mirror:unfold",
	mirrorHeat: () => "mirror:heat",
	mirrorAutofold: () => "mirror:autofold",
	mirrorDip: () => "mirror:dip",

	// Lock
	lock: () => "lock",
	unlock: () => "unlock",
	lockChild: () => "lock:child",
	horn: () => "horn",

	// Trunk/Frunk
	frunkOpen: () => "frunk:open",
	frunkClose: () => "frunk:close",
	trunkOpen: () => "trunk:open",
	trunkClose: () => "trunk:close",
	glovebox: () => "glovebox",

	// Lights
	lightFogFront: () => "light:fog:front",
	lightFogRear: () => "light:fog:rear",
	lightHighbeamAuto: () => "light:highbeam:auto",
	lightAmbient: () => "light:ambient",
	lightHome: () => "light:home",
	lightDomeOff: () => "light:dome:off",
	lightDomeOn: () => "light:dome:on",
	lightDomeAuto: () => "light:dome:auto",

	// Wiper
	wiperOff: () => "wiper:off",
	wiper1: () => "wiper:1",
	wiper2: () => "wiper:2",
	wiper3: () => "wiper:3",

	// Seat Heating
	seatFL: (level: number) => {
		assertRange("seatFL", level, COMMAND_RANGES.seat.min, COMMAND_RANGES.seat.max);
		return `seat:fl:${level}`;
	},
	seatFR: (level: number) => {
		assertRange("seatFR", level, COMMAND_RANGES.seat.min, COMMAND_RANGES.seat.max);
		return `seat:fr:${level}`;
	},
	seatRL: (level: number) => {
		assertRange("seatRL", level, COMMAND_RANGES.seat.min, COMMAND_RANGES.seat.max);
		return `seat:rl:${level}`;
	},
	seatRR: (level: number) => {
		assertRange("seatRR", level, COMMAND_RANGES.seat.min, COMMAND_RANGES.seat.max);
		return `seat:rr:${level}`;
	},
	seatRC: (level: number) => {
		assertRange("seatRC", level, COMMAND_RANGES.seat.min, COMMAND_RANGES.seat.max);
		return `seat:rc:${level}`;
	},

	// Display
	mainDisplay: (level: number) => {
		assertRange(
			"mainDisplay",
			level,
			COMMAND_RANGES.mainDisplay.min,
			COMMAND_RANGES.mainDisplay.max,
		);
		return `maindisplay:${level}`;
	},

	// Power
	powerAccOn: () => "power:acc:on",
	powerAccOff: () => "power:acc:off",
	powerOff: () => "power:off",
	powerReady: () => "power:ready",

	// Window
	windowVentOpen: () => "window:vent:open",
	windowVentClose: () => "window:vent:close",
	ventOpen: () => "vent:open",
	ventClose: () => "vent:close",
	windowVent: (position: number) => {
		assertRange(
			"windowVent",
			position,
			COMMAND_RANGES.windowVent.min,
			COMMAND_RANGES.windowVent.max,
		);
		return `window:vent:${position}`;
	},

	// Sentry
	sentryOn: () => "sentry:on",
	sentryOff: () => "sentry:off",

	// Climate
	climateKeep: () => "climate:keep",
	climateOff: () => "climate:off",

	// Charge
	chargeStart: () => "charge:start",
	chargeStop: () => "charge:stop",
	chargePort: () => "chargeport",

	// Drive Config
	pedalStandard: () => "pedal:standard",
	pedalChill: () => "pedal:chill",
	pedalSport: () => "pedal:sport",
	regenOff: () => "regen:off",
	regenLow: () => "regen:low",
	regenStd: () => "regen:std",
	regenMax: () => "regen:max",
	stopCreep: () => "stop:creep",
	stopRoll: () => "stop:roll",
	stopHold: () => "stop:hold",

	// Drive Mode Override ("Ghost Mode")
	driveModeOff: () => "drivemode:off",
	driveModeChill: () => "drivemode:chill",
	driveModeStandard: () => "drivemode:standard",
	driveModePerformance: () => "drivemode:performance",

	// TPMS query
	tpms: () => "tpms",

	// ECE R79 bypass (EU only)
	eceR79: (on: boolean) => (on ? "ecer79:on" : "ecer79:off"),

	// Region spoofing
	regionSpoof: (code: string) => {
		assertRegionSpoofCode(code);
		return `region:spoof:${code}`;
	},

	// Auto Lane Change confirmation
	alc: (on: boolean) => (on ? "alc:on" : "alc:off"),

	// Turn signals (3-blink lane change)
	turnLeft3: () => "turn:left3",
	turnRight3: () => "turn:right3",
	turnHazard: () => "turn:hazard",
	turnOff: () => "turn:off",

	// Seatbelt emulation
	seatbelt: (on: boolean) => (on ? "seatbelt:on" : "seatbelt:off"),

	// Air recirculation
	airRecirc: (on: boolean) => (on ? "airecirc:on" : "airecirc:off"),

	// Wiper speed persistence
	wiperPersist: (on: boolean) => (on ? "wiperpersist:on" : "wiperpersist:off"),

	// Mirror auto-fold on lock
	mirrorAutoFold: (on: boolean) => (on ? "mirror:autofold:on" : "mirror:autofold:off"),

	// Powertrain telemetry query
	powertrain: () => "powertrain",

	// CAN simulation
	canSimStart: () => "simu:start",
	canSimStop: () => "simu:stop",

	// Single-shot TX mode
	singleShot: (on: boolean) => (on ? "singleshot:on" : "singleshot:off"),

	// Firmware version compatibility query
	fwCompat: () => "fwcompat",

	// MQTT bridge
	mqtt: (on: boolean) => (on ? "mqtt:on" : "mqtt:off"),
	mqttBroker: (host: string) => {
		if (!host || host.length > 63)
			throw new RangeError(`MQTT broker host must be 1-63 chars, got "${host}"`);
		return `mqtt:broker:${host}`;
	},
	mqttPort: (port: number) => {
		if (!Number.isInteger(port) || port < 1 || port > 65535)
			throw new RangeError(`MQTT port must be 1-65535, got ${port}`);
		return `mqtt:port:${port}`;
	},
	mqttInterval: (ms: number) => {
		if (!Number.isInteger(ms) || ms < 100 || ms > 60000)
			throw new RangeError(`MQTT interval must be 100-60000 ms, got ${ms}`);
		return `mqtt:interval:${ms}`;
	},

	// Vehicle config query
	vehicle: () => "vehicle",

	// Vehicle platform identity query
	platform: () => "platform",

	// Log ring buffer dump
	log: () => "log",

	// Button remapping (2.2)
	btnMap: (button: string, press: string, action: string) => {
		assertButtonName(button);
		assertPressType(press);
		assertButtonAction(action);
		return `btnmap:${button}:${press}:${action}`;
	},
	btnMapQuery: () => "btnmap:query",
	btnMapReset: () => "btnmap:reset",

	// Speed camera alert BLE service (2.8)
	speedAlert: (on: boolean) => (on ? "speedalert:on" : "speedalert:off"),

	// GVRET TCP gateway (3.5)
	gvret: (on: boolean) => (on ? "gvret:on" : "gvret:off"),
	gvretPort: (port: number) => {
		if (!Number.isInteger(port) || port < 1 || port > 65535)
			throw new RangeError(`GVRET port must be 1-65535, got ${port}`);
		return `gvret:port:${port}`;
	},

	// ESP-NOW multi-device (4.1)
	espNow: (on: boolean) => (on ? "espnow:on" : "espnow:off"),
	espNowChannel: (ch: number) => {
		if (!Number.isInteger(ch) || ch < 1 || ch > 13)
			throw new RangeError(`ESP-NOW channel must be 1-13, got ${ch}`);
		return `espnow:channel:${ch}`;
	},

	// ScanMyTesla BT bridge (4.2)
	scanMyTesla: (on: boolean) => (on ? "smt:on" : "smt:off"),

	// ELM327 emulation (4.3)
	elm327: (on: boolean) => (on ? "elm327:on" : "elm327:off"),

	// Tesla BLE Vehicle Control (4.4)
	teslaBle: (on: boolean) => (on ? "teslable:on" : "teslable:off"),
	teslaBleAuth: () => "teslable:auth",
	teslaBleForget: () => "teslable:forget",

	// Home Assistant / ESPHome integration (4.5)
	homeAssistant: (on: boolean) => (on ? "ha:on" : "ha:off"),
	haDiscovery: () => "ha:discovery",
	haInterval: (ms: number) => {
		if (!Number.isInteger(ms) || ms < 500 || ms > 60000)
			throw new RangeError(`HA polling interval must be 500-60000 ms, got ${ms}`);
		return `ha:interval:${ms}`;
	},

	// Encrypted BLE Multi-Device (4.6)
	bleEncrypt: (on: boolean) => (on ? "bleencrypt:on" : "bleencrypt:off"),
	blePair: () => "bleencrypt:pair",
	bleUnpair: () => "bleencrypt:unpair",

	// Gamepad (BLE HID) — pair/unpair, scan, enable, status, bindings, axis tuning.
	// See firmware/lib/client/gamepad/* and dispatch.h "gamepad:" handler.
	gamepadScan: () => "gamepad:scan",
	gamepadPair: (addr: string) => {
		if (!/^[0-9a-fA-F]{2}(:[0-9a-fA-F]{2}){5}$/.test(addr)) {
			throw new RangeError(`gamepadPair expects MAC AA:BB:CC:DD:EE:FF, got "${addr}"`);
		}
		return `gamepad:pair:${addr.toLowerCase()}`;
	},
	gamepadUnpair: () => "gamepad:unpair",
	gamepad: (on: boolean) => (on ? "gamepad:on" : "gamepad:off"),
	gamepadStatus: () => "gamepad:status",
	gamepadCancel: () => "gamepad:cancel",
	gamepadBind: (button: number, cmd: string) => {
		if (!Number.isInteger(button) || button < 0 || button > 15) {
			throw new RangeError(`gamepadBind button must be 0..15, got ${button}`);
		}
		return `gamepad:bind:${button}:${cmd}`;
	},
	gamepadHold: (button: number, cmd: string) => {
		if (!Number.isInteger(button) || button < 0 || button > 15) {
			throw new RangeError(`gamepadHold button must be 0..15, got ${button}`);
		}
		return `gamepad:hold:${button}:${cmd}`;
	},
	gamepadAxis: (axis: number, kind: "dz" | "expo" | "inv", value: number) => {
		if (!Number.isInteger(axis) || axis < 0 || axis > 5) {
			throw new RangeError(`gamepadAxis axis must be 0..5, got ${axis}`);
		}
		if (kind !== "dz" && kind !== "expo" && kind !== "inv") {
			throw new RangeError(`gamepadAxis kind must be dz|expo|inv, got "${kind}"`);
		}
		return `gamepad:axis:${axis}:${kind}:${value}`;
	},

	// DAS Gate
	dasArm: () => "das:arm",
	dasDisarm: () => "das:disarm",
	dasStatus: () => "das:status",

	// Tesla BLE Key Protocol
	teslaKeyGen: () => "tesla:key:gen",
	teslaKeyShow: () => "tesla:key:show",
	teslaKeyRoleOwner: () => "tesla:key:role:owner",
	teslaKeyRoleChargingManager: () => "tesla:key:role:charging_manager",
	teslaKeySend: () => "tesla:key:send",
	teslaWake: () => "tesla:wake",
	teslaChargeStart: () => "tesla:charge:start",
	teslaChargeStop: () => "tesla:charge:stop",
	teslaClimateOn: () => "tesla:climate:on",
	teslaClimateOff: () => "tesla:climate:off",
	teslaVin: (vin: string) => `tesla:vin:${vin}`,
	teslaChargeAmps: (amps: number) => `tesla:charge:amps:${amps}`,
	teslaChargeLimit: (pct: number) => `tesla:charge:limit:${pct}`,
} as const;

/** Command object returned by standalone command builders. */
export interface Command {
	cmd: string;
}

/** Profile label map */
export const PROFILE_LABELS: Record<number, string> = {
	0: "Chill",
	1: "Normal",
	2: "Hurry",
	3: "Max",
	4: "Sloth",
};

// ── Standalone command builders ──────────────────────────────────────────────

export function lhdOn(): Command {
	return { cmd: "lhd:on" };
}
export function lhdOff(): Command {
	return { cmd: "lhd:off" };
}
export function apFirstOn(): Command {
	return { cmd: "apfirst:on" };
}
export function apFirstOff(): Command {
	return { cmd: "apfirst:off" };
}
export function laneGraphOn(): Command {
	return { cmd: "lanegraph:on" };
}
export function laneGraphOff(): Command {
	return { cmd: "lanegraph:off" };
}
export function assistDevOn(): Command {
	return { cmd: "assist-dev:on" };
}
export function assistDevOff(): Command {
	return { cmd: "assist-dev:off" };
}
export function assistNavOn(): Command {
	return { cmd: "assist-nav:on" };
}
export function assistNavOff(): Command {
	return { cmd: "assist-nav:off" };
}
export function assistHofOn(): Command {
	return { cmd: "assist-hof:on" };
}
export function assistHofOff(): Command {
	return { cmd: "assist-hof:off" };
}
export function assistTelOn(): Command {
	return { cmd: "assist-tel:on" };
}
export function assistTelOff(): Command {
	return { cmd: "assist-tel:off" };
}

// ── TLSSC Restore ──────────────────────────────────────────────────────────

export function tlsscOn(): Command {
	return { cmd: "tlssc:on" };
}
export function tlsscOff(): Command {
	return { cmd: "tlssc:off" };
}

// ── Emergency Vehicle Detection ────────────────────────────────────────────

export function evdOn(): Command {
	return { cmd: "evd:on" };
}
export function evdOff(): Command {
	return { cmd: "evd:off" };
}

// ── DAS Gate ───────────────────────────────────────────────────────────────

export function dasArm(): Command {
	return { cmd: "das:arm" };
}
export function dasDisarm(): Command {
	return { cmd: "das:disarm" };
}
export function dasStatus(): Command {
	return { cmd: "das:status" };
}

// ── Tesla BLE Key Protocol ─────────────────────────────────────────────────

export function teslaKeyGen(): Command {
	return { cmd: "tesla:key:gen" };
}
export function teslaKeyShow(): Command {
	return { cmd: "tesla:key:show" };
}
export function teslaKeyRoleOwner(): Command {
	return { cmd: "tesla:key:role:owner" };
}
export function teslaKeyRoleChargingManager(): Command {
	return { cmd: "tesla:key:role:charging_manager" };
}
export function teslaKeySend(): Command {
	return { cmd: "tesla:key:send" };
}
export function teslaWake(): Command {
	return { cmd: "tesla:wake" };
}
export function teslaChargeStart(): Command {
	return { cmd: "tesla:charge:start" };
}
export function teslaChargeStop(): Command {
	return { cmd: "tesla:charge:stop" };
}
export function teslaClimateOn(): Command {
	return { cmd: "tesla:climate:on" };
}
export function teslaClimateOff(): Command {
	return { cmd: "tesla:climate:off" };
}

// ── Aliases ─────────────────────────────────────────────────────────────────

export function pedalStd(): Command {
	return { cmd: "pedal:std" };
}
export function regenStandard(): Command {
	return { cmd: "regen:standard" };
}
export function frunk(): Command {
	return { cmd: "frunk" };
}
