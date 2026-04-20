/**
 * Cross-check: docs ↔ protocol commands ↔ firmware wire commands.
 *
 * Keeps docs/commands.md, packages/protocol/src/commands.ts, and
 * firmware wire commands in sync. If a command is added, removed,
 * or renamed in any layer, this test fails until all three match.
 */
import { commands } from '../../src/commands.js';

// Declare Node.js ESM globals for ts-jest (tsconfig has types:["jest"] only)
declare const import_meta_url: string; // eslint-disable-line @typescript-eslint/naming-convention

// @ts-expect-error — Node.js built-in, no @types/node
// eslint-disable-next-line @typescript-eslint/no-unsafe-assignment
const { createRequire } = await import('module');
// eslint-disable-next-line @typescript-eslint/no-unsafe-assignment, @typescript-eslint/no-unsafe-call
const require = createRequire(import.meta.url);
const fs = require('fs') as { readFileSync(path: string, encoding: string): string };
const path = require('path') as { resolve(...segs: string[]): string; dirname(p: string): string };
const url = require('url') as { fileURLToPath(u: string): string };

const __filename = url.fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const docsPath = path.resolve(__dirname, '../../../../docs/reference/commands.md');
const docsContent: string = fs.readFileSync(docsPath, 'utf-8');

// ── Canonical wire commands the firmware accepts ─────────────────────────────
// Single source of truth for every wire-level command string.
// When firmware adds/removes a command, update this list and the test will
// force you to update docs + protocol too.

const FIRMWARE_WIRE_COMMANDS: string[] = [
	// System
	'ping',
	'status',

	// Variant
	'variant:hw4',
	'variant:hw3',
	'variant:legacy',
	'variant:auto',

	// FSD
	'fsd:on',
	'fsd:off',
	'fsd:force:on',
	'fsd:force:off',

	// Nag
	'nag:on',
	'nag:off',

	// Ban Shield
	'banshield:on',
	'banshield:off',

	// Speed Profile
	'profile:0',
	'profile:1',
	'profile:2',
	'profile:3',
	'profile:4',
	'profile:auto',
	'profile:lock',
	'profile:unlock',

	// Speed Offset (auto-routes HW3 0-100 / HW4 0-63)
	'offset:0',
	'offset:auto',
	'offset:off',

	// ISA Chime
	'isa-chime:on',
	'isa-chime:off',

	// Nag Killer
	'nag:killer:on',
	'nag:killer:off',
	'nag:killer:mode:legacy',
	'nag:killer:mode:safe',
	'nag:killer:mode:natural',

	// Summon
	'summon-inject:on',
	'summon-inject:off',
	'summon',
	'summon:forward',
	'summon:reverse',
	'summon:stop',

	// Preconditioning
	'precondition:on',
	'precondition:off',

	// Track Mode
	'trackmode:on',
	'trackmode:off',

	// BMS
	'bms',

	// CAN Clock
	'canclock:auto',
	'canclock:8',
	'canclock:12',
	'canclock:16',
	'canclock:20',

	// Streaming
	'stream:on',
	'stream:off',
	'can:raw:on',
	'can:raw:off',

	// Vehicle Commands (3 CAN buses)
	'lock',
	'unlock',
	'lock:child',
	'horn',
	'frunk:open',
	'frunk:close',
	'trunk:open',
	'trunk:close',
	'glovebox',
	'mirror:fold',
	'mirror:unfold',
	'mirror:heat',
	'mirror:autofold',
	'mirror:dip',
	'light:fog:front',
	'light:fog:rear',
	'light:highbeam:auto',
	'light:ambient',
	'light:home',
	'light:dome:off',
	'light:dome:on',
	'light:dome:auto',
	'wiper:off',
	'wiper:1',
	'wiper:2',
	'wiper:3',
	'seat:fl:0',
	'seat:fr:0',
	'seat:rl:0',
	'seat:rr:0',
	'seat:rc:0',
	'maindisplay:0',
	'power:acc:on',
	'power:acc:off',
	'power:ready',
	'power:off',
	'window:vent:open',
	'window:vent:close',
	'vent:open',
	'vent:close',
	'sentry:on',
	'sentry:off',
	'climate:keep',
	'climate:off',
	'charge:start',
	'charge:stop',
	'chargeport',
	'pedal:standard',
	'pedal:chill',
	'pedal:sport',
	'regen:off',
	'regen:low',
	'regen:std',
	'regen:max',
	'stop:creep',
	'stop:roll',
	'stop:hold',
	'drivemode:off',
	'drivemode:chill',
	'drivemode:standard',
	'drivemode:performance',
	'ecer79:on',
	'ecer79:off',
	'region:spoof:na',
	'region:spoof:eu',
	'region:spoof:cn',
	'region:spoof:apac',
	'region:spoof:me',
	'region:spoof:off',
	'alc:on',
	'alc:off',
	'gtwshield:arm',
	'gtwshield:disarm',
	'gtwshield:reset',
	'ratelimit:on',
	'ratelimit:off',
	'tpms',
	'turn:left3',
	'turn:right3',
	'turn:hazard',
	'turn:off',
	'seatbelt:on',
	'seatbelt:off',
	'airecirc:on',
	'airecirc:off',
	'wiperpersist:on',
	'wiperpersist:off',
	'mirror:autofold:on',
	'mirror:autofold:off',
	'powertrain',
	'simu:start',
	'simu:stop',
	'singleshot:on',
	'singleshot:off',
	'fwcompat',
	'mqtt:on',
	'mqtt:off',
	'mqtt:broker:<host>',
	'mqtt:port:1883',
	'mqtt:interval:1000',
	'vehicle',
	'platform',
	'log',
	'window:vent:42',

	// Button Remapping
	'btnmap:lamp:short:trunk',
	'btnmap:lamp:long:sentry',
	'btnmap:lamp:double:horn',
	'btnmap:parking:short:fold',
	'btnmap:parking:long:hazard',
	'btnmap:parking:double:recirc',
	'btnmap:query',
	'btnmap:reset',

	// Speed Camera Alert
	'speedalert:on',
	'speedalert:off',

	// GVRET
	'gvret:on',
	'gvret:off',
	'gvret:port:23',

	// ESP-NOW
	'espnow:on',
	'espnow:off',
	'espnow:channel:1',

	// ScanMyTesla
	'smt:on',
	'smt:off',

	// ELM327
	'elm327:on',
	'elm327:off',

	// Tesla BLE
	'teslable:on',
	'teslable:off',
	'teslable:auth',
	'teslable:forget',

	// Home Assistant
	'ha:on',
	'ha:off',
	'ha:discovery',
	'ha:interval:1000',

	// Encrypted BLE
	'bleencrypt:on',
	'bleencrypt:off',
	'bleencrypt:pair',
	'bleencrypt:unpair',
];

// ── Helpers ──────────────────────────────────────────────────────────────────

// Known BMS response field names (not commands) that appear in backtick table cells
const BMS_FIELDS = new Set([
	'enhanced', 'packVoltage', 'packCurrent', 'soc', 'maxTemp', 'minTemp',
	'avgTemp', 'maxCellV', 'minCellV', 'balancing', 'faults',
]);

// Bare commands that are valid wire commands without ':' separators.
const BARE_COMMANDS = new Set([
	'ping', 'status', 'summon', 'bms', 'lock', 'unlock', 'horn', 'glovebox',
	'chargeport', 'tpms', 'powertrain', 'fwcompat', 'vehicle', 'platform', 'log',
]);

/** Extract all `backtick-wrapped` commands from docs/commands.md tables */
function extractDocCommands(): string[] {
	const cmds: string[] = [];
	// Match first backtick column in table rows: | `command` | ... |
	const tableRe = /^\|\s*`([^`]+)`\s*\|/gm;
	let m: RegExpExecArray | null;
	while ((m = tableRe.exec(docsContent)) !== null) {
		const cmd = m[1];
		// Skip range notations, BMS field names, and header-like entries
		if (cmd.includes('–') || cmd.includes('...') || cmd.includes('..')) continue;
		if (BMS_FIELDS.has(cmd)) continue;
		if (cmd.includes('<') || cmd.includes('>')) continue;
		if (!cmd.includes(':') && !BARE_COMMANDS.has(cmd)) continue;
		cmds.push(cmd);
	}
	return [...new Set(cmds)];
}

/** Collect every wire string the protocol commands object can produce */
function extractProtocolCommands(): string[] {
	const cmds: string[] = [];
	for (const [key, fn] of Object.entries(commands)) {
		if (typeof fn !== 'function') continue;
		try {
			// Zero-arg commands
			if (fn.length === 0) {
				cmds.push((fn as () => string)());
				continue;
			}
			// Boolean commands
			if (key === 'fsd' || key === 'fsdForce' || key === 'nag' || key === 'isaChime' ||
				key === 'nagKiller' || key === 'banShield' || key === 'precondition' ||
				key === 'trackMode' || key === 'stream' || key === 'rawCan' || key === 'summonInject' ||
				key === 'eceR79' || key === 'rateLimit' || key === 'seatbelt' || key === 'airRecirc' ||
				key === 'wiperPersist' || key === 'mirrorAutoFold' || key === 'singleShot' || key === 'mqtt' ||
				key === 'speedAlert' || key === 'gvret' || key === 'espNow' || key === 'scanMyTesla' || key === 'elm327' ||
				key === 'teslaBle' || key === 'homeAssistant' || key === 'bleEncrypt' || key === 'alc') {
				cmds.push((fn as (b: boolean) => string)(true));
				cmds.push((fn as (b: boolean) => string)(false));
				continue;
			}
			// Variant
			if (key === 'variant') {
				cmds.push((fn as (v: string) => string)('hw3'));
				cmds.push((fn as (v: string) => string)('hw4'));
				cmds.push((fn as (v: string) => string)('legacy'));
				cmds.push((fn as (v: string) => string)('auto'));
				continue;
			}
			// Nag killer mode
			if (key === 'nagKillerMode') {
				cmds.push((fn as (m: string) => string)('legacy'));
				cmds.push((fn as (m: string) => string)('safe'));
				cmds.push((fn as (m: string) => string)('natural'));
				continue;
			}
			// Region spoof
			if (key === 'regionSpoof') {
				for (const r of ['na', 'eu', 'cn', 'apac', 'me', 'off']) {
					cmds.push((fn as (r: string) => string)(r));
				}
				continue;
			}
			// Numeric: enumerate all firmware-accepted values
			if (key === 'profile') {
				for (let i = 0; i <= 4; i++) cmds.push((fn as (n: number) => string)(i));
				continue;
			}
			if (key === 'canClock') {
				for (const mhz of [8, 12, 16, 20]) cmds.push((fn as (n: number) => string)(mhz));
				continue;
			}
			if (key === 'offset' || key === 'seatFL' || key === 'seatFR' || key === 'seatRL' ||
				key === 'seatRR' || key === 'seatRC' || key === 'mainDisplay') {
				cmds.push((fn as (n: number) => string)(0));
				continue;
			}
			if (key === 'windowVent') {
				cmds.push((fn as (n: number) => string)(42));
				continue;
			}
			if (key === 'mqttPort') {
				cmds.push((fn as (n: number) => string)(1883));
				continue;
			}
			if (key === 'mqttInterval') {
				cmds.push((fn as (n: number) => string)(1000));
				continue;
			}
			if (key === 'mqttBroker') {
				cmds.push((fn as (h: string) => string)('<host>'));
				continue;
			}
			if (key === 'btnMap') {
				cmds.push((fn as (b: string, p: string, a: string) => string)('lamp', 'short', 'trunk'));
				cmds.push((fn as (b: string, p: string, a: string) => string)('lamp', 'long', 'sentry'));
				cmds.push((fn as (b: string, p: string, a: string) => string)('lamp', 'double', 'horn'));
				cmds.push((fn as (b: string, p: string, a: string) => string)('parking', 'short', 'fold'));
				cmds.push((fn as (b: string, p: string, a: string) => string)('parking', 'long', 'hazard'));
				cmds.push((fn as (b: string, p: string, a: string) => string)('parking', 'double', 'recirc'));
				continue;
			}
			if (key === 'gvretPort') {
				cmds.push((fn as (n: number) => string)(23));
				continue;
			}
			if (key === 'espNowChannel') {
				cmds.push((fn as (n: number) => string)(1));
				continue;
			}
			if (key === 'haInterval') {
				cmds.push((fn as (n: number) => string)(1000));
				continue;
			}
		} catch {
			// Skip any that throw
		}
	}
	return [...new Set(cmds)];
}

// ── Tests ────────────────────────────────────────────────────────────────────

describe('Cross-check: docs ↔ protocol ↔ firmware', () => {
	const docCmds = extractDocCommands();
	const protocolCmds = extractProtocolCommands();

	it('every firmware wire command has a protocol builder', () => {
		const missing = FIRMWARE_WIRE_COMMANDS.filter(cmd => !protocolCmds.includes(cmd));
		expect(missing).toEqual([]);
	});

	it('every protocol builder produces a valid firmware wire command', () => {
		const extra = protocolCmds.filter(cmd => !FIRMWARE_WIRE_COMMANDS.includes(cmd));
		expect(extra).toEqual([]);
	});

	it('every firmware wire command is documented in commands.md', () => {
		// Some commands use numeric ranges; docs show representative values
		// We check the prefix exists in docs for parameterized commands
		const undocumented: string[] = [];
		for (const cmd of FIRMWARE_WIRE_COMMANDS) {
			// For numeric commands (offset:0, seat:fl:0, etc.), check the prefix pattern is in docs
			const prefix = cmd.replace(/:\d+$/, ':');
			const found = docCmds.some(dc => dc === cmd || dc.startsWith(prefix) || cmd.startsWith(dc.replace(/N$/, '')));
			if (!found) {
				// Check if a backtick pattern like `offset:N` or `ha:interval:<ms>` covers this
				const nPattern = prefix + 'N';
				const anglePattern = cmd.split(':').slice(0, -1).join(':') + ':<';
				const inDoc = docsContent.includes('`' + cmd + '`') ||
					docsContent.includes('`' + nPattern + '`') ||
					docsContent.includes('`' + cmd.split(':').slice(0, -1).join(':') + ':N`') ||
					docsContent.includes(anglePattern);
				if (!inDoc) {
					undocumented.push(cmd);
				}
			}
		}
		expect(undocumented).toEqual([]);
	});

	it('docs do not reference commands that firmware does not accept', () => {
		// Filter out range notations and descriptive entries
		const bogus = docCmds.filter(dc => {
			// Commands with N placeholder are patterns, not literal commands
			if (dc.endsWith(':N')) return false;
			if (/[A-Z]/.test(dc) && !dc.includes('MHz')) return false;
			// Range entries like "seat:fl:0" – "seat:fl:3" — check base
			return !FIRMWARE_WIRE_COMMANDS.includes(dc);
		});
		expect(bogus).toEqual([]);
	});
});

