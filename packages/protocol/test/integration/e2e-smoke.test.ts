/**
 * E2E Smoke Tests — Core Command Lifecycle
 *
 * Exercises the full round-trip: command building → serial line simulation →
 * parsing → state validation. Covers boot, status, ack, frame, error, and
 * streaming scenarios documented in docs/e2e-test-plan.md.
 */

import { commands, VALID_VARIANTS } from "../../src/commands.js";
import { parseSerialLine, parseSerialChunk } from "../../src/parser.js";
import type {
	BootMessage,
	StatusMessage,
	FrameMessage,
	AckMessage,
	ErrorMessage,
	LogMessage,
	PongMessage,
	ParsedEvent,
} from "../../src/types.js";

// ── Helpers ──────────────────────────────────────────────────────────────────

/** Simulate firmware echoing a JSON message on the serial line. */
function serial(json: Record<string, unknown>): string {
	return JSON.stringify(json);
}

/** Parse a single serial line and expect exactly one message event. */
function expectMessage(line: string): ParsedEvent & { type: "message" } {
	const events = parseSerialLine(line);
	expect(events).toHaveLength(1);
	expect(events[0].type).toBe("message");
	return events[0] as ParsedEvent & { type: "message" };
}

// ── Boot Sequence ────────────────────────────────────────────────────────────

describe("E2E Smoke: Boot Sequence", () => {
	const bootPayload = {
		t: "boot",
		variant: "hw4",
		hw: "ArduinoUnoR3CH340",
		can: "MCP2515_TJA1050_8MHz",
		drv: "arduino-mcp2515",
		busChassis: true,
		busVehicle: false,
		busBody: false,
		bt: "HC-05",
		btEnabled: false,
		fsd: false,
		nag: false,
		sp: 0,
		offset: 0,
		isaChime: false,
		summonInject: false,
		banShield: 0,
		banThreat: 0,
		banDetectCount: 0,
		chassisOnline: true,
		standby: false,
	};

	it("parses a full boot message", () => {
		const ev = expectMessage(serial(bootPayload));
		const boot = ev.message as unknown as BootMessage;
		expect(boot.t).toBe("boot");
		expect(boot.variant).toBe("hw4");
		expect(boot.hw).toBe("ArduinoUnoR3CH340");
		expect(boot.busChassis).toBe(true);
		expect(boot.busVehicle).toBe(false);
		expect(boot.fsd).toBe(false);
		expect(boot.banShield).toBe(0);
		expect(boot.banThreat).toBe(0);
		expect(boot.banDetectCount).toBe(0);
		expect(boot.chassisOnline).toBe(true);
	});

	it("parses boot with ESP32 fields", () => {
		const esp32Boot = {
			...bootPayload,
			hw: "ESP32DevKit",
			can: "MCP2515_TJA1050_8MHz",
			drv: "arduino-mcp2515",
			busChassis: true,
			busVehicle: true,
			busBody: true,
		};
		const ev = expectMessage(serial(esp32Boot));
		const boot = ev.message as unknown as BootMessage;
		expect(boot.busVehicle).toBe(true);
		expect(boot.busBody).toBe(true);
	});
});

// ── Command → Ack Round-Trip ─────────────────────────────────────────────────

describe("E2E Smoke: Command → Ack Round-Trip", () => {
	const ackScenarios: Array<{ name: string; cmd: string; ackCmd: string }> = [
		{ name: "ping → pong", cmd: commands.ping(), ackCmd: "ping" },
		{ name: "fsd on", cmd: commands.fsd(true), ackCmd: "fsd:on" },
		{ name: "fsd off", cmd: commands.fsd(false), ackCmd: "fsd:off" },
		{ name: "nag on", cmd: commands.nag(true), ackCmd: "nag:on" },
		{ name: "nag mode bit19", cmd: commands.nagMode("bit19"), ackCmd: "nag:mode:bit19" },
		{ name: "variant hw3", cmd: commands.variant("hw3"), ackCmd: "variant:hw3" },
		{ name: "variant hw4", cmd: commands.variant("hw4"), ackCmd: "variant:hw4" },
		{ name: "variant legacy", cmd: commands.variant("legacy"), ackCmd: "variant:legacy" },
		{ name: "profile 0", cmd: commands.profile(0), ackCmd: "profile:0" },
		{ name: "profile 4", cmd: commands.profile(4), ackCmd: "profile:4" },
		{ name: "offset 0", cmd: commands.offset(0), ackCmd: "offset:0" },
		{ name: "offset 100", cmd: commands.offset(100), ackCmd: "offset:100" },
		{ name: "stream on", cmd: commands.stream(true), ackCmd: "stream:on" },
		{ name: "stream off", cmd: commands.stream(false), ackCmd: "stream:off" },
		{ name: "lock", cmd: commands.lock(), ackCmd: "lock" },
		{ name: "unlock", cmd: commands.unlock(), ackCmd: "unlock" },
		{ name: "frunk open", cmd: commands.frunkOpen(), ackCmd: "frunkOpen" },
		{ name: "trunk open", cmd: commands.trunkOpen(), ackCmd: "trunkOpen" },
		{ name: "seat FL 3", cmd: commands.seatFL(3), ackCmd: "seatFL:3" },
		{ name: "mainDisplay 50", cmd: commands.mainDisplay(50), ackCmd: "mainDisplay:50" },
		{ name: "sentry on", cmd: commands.sentryOn(), ackCmd: "sentryOn" },
		{ name: "climate keep", cmd: commands.climateKeep(), ackCmd: "climateKeep" },
		{ name: "charge start", cmd: commands.chargeStart(), ackCmd: "chargeStart" },
		{ name: "ban shield on", cmd: commands.banShield(true), ackCmd: "banshield:on" },
		{ name: "ban shield off", cmd: commands.banShield(false), ackCmd: "banshield:off" },
	];

	it.each(ackScenarios)('$name: send "$cmd" → ack "$ackCmd"', ({ cmd, ackCmd }) => {
		// 1. Command string is non-empty
		expect(cmd).toBeTruthy();
		expect(typeof cmd).toBe("string");

		// 2. Firmware would echo an ack — simulate and parse
		const ackLine = serial({ t: "ack", cmd: ackCmd });
		const ev = expectMessage(ackLine);
		const ack = ev.message as unknown as AckMessage;
		expect(ack.t).toBe("ack");
		expect(ack.cmd).toBe(ackCmd);
	});

	it("all variants produce valid command strings", () => {
		for (const v of VALID_VARIANTS) {
			const cmd = commands.variant(v);
			expect(cmd).toBe(`variant:${v}`);
		}
	});
});

// ── Status Heartbeat ─────────────────────────────────────────────────────────

describe("E2E Smoke: Status Heartbeat", () => {
	it("parses periodic status messages", () => {
		const statusLine = serial({
			t: "status",
			variant: "hw4",
			up: 12345,
			rate: 42,
			fsd: true,
			nag: false,
			sp: 2,
			offset: 5,
			chassisOnline: true,
			stream: true,
			busChassis: true,
			busVehicle: true,
			busBody: false,
			banShield: 1,
			banThreat: 4,
			banDetectCount: 12,
		});
		const ev = expectMessage(statusLine);
		const status = ev.message as unknown as StatusMessage;
		expect(status.t).toBe("status");
		expect(status.up).toBe(12345);
		expect(status.rate).toBe(42);
		expect(status.fsd).toBe(true);
		expect(status.stream).toBe(true);
		expect(status.banShield).toBe(1);
		expect(status.banThreat).toBe(4);
		expect(status.banDetectCount).toBe(12);
	});

	it("handles status with minimal fields", () => {
		const ev = expectMessage(serial({ t: "status", up: 0 }));
		expect((ev.message as unknown as StatusMessage).t).toBe("status");
	});
});

// ── Frame Streaming ──────────────────────────────────────────────────────────

describe("E2E Smoke: Frame Streaming", () => {
	it("parses FSD bus frames", () => {
		const frame = {
			t: "frame",
			id: 0x399,
			dir: "rx",
			dlc: 8,
			d: "AABBCCDD11223344",
			bus: 0,
			seq: 1,
			ms: 100,
		};
		const ev = expectMessage(serial(frame));
		const f = ev.message as unknown as FrameMessage;
		expect(f.t).toBe("frame");
		expect(f.id).toBe(0x399);
		expect(f.dir).toBe("rx");
		expect(f.bus).toBe(0);
		expect(f.d).toBe("AABBCCDD11223344");
	});

	it("parses vehicle bus frames", () => {
		const frame = { t: "frame", id: 0x273, dir: "tx", dlc: 8, d: "0000000000000000", bus: 1 };
		const ev = expectMessage(serial(frame));
		expect((ev.message as unknown as FrameMessage).bus).toBe(1);
	});

	it("parses body bus frames", () => {
		const frame = { t: "frame", id: 0x100, dir: "rx", dlc: 4, d: "AABBCCDD", bus: 2 };
		const ev = expectMessage(serial(frame));
		expect((ev.message as unknown as FrameMessage).bus).toBe(2);
	});
});

// ── Error and Log Messages ───────────────────────────────────────────────────

describe("E2E Smoke: Error and Log Messages", () => {
	it("parses error messages", () => {
		const ev = expectMessage(serial({ t: "error", msg: "MCP2515 init failed" }));
		const err = ev.message as unknown as ErrorMessage;
		expect(err.t).toBe("error");
		expect(err.msg).toBe("MCP2515 init failed");
	});

	it("parses log messages", () => {
		const ev = expectMessage(serial({ t: "log", msg: "CAN bus online" }));
		const log = ev.message as unknown as LogMessage;
		expect(log.t).toBe("log");
		expect(log.msg).toBe("CAN bus online");
	});

	it("parses pong response", () => {
		const ev = expectMessage(serial({ t: "pong" }));
		expect((ev.message as unknown as PongMessage).t).toBe("pong");
	});
});

// ── Streaming Chunk Parsing ──────────────────────────────────────────────────

describe("E2E Smoke: Streaming Chunk Parsing", () => {
	it("handles multi-line chunks with remainder", () => {
		const chunk =
			'{"t":"status","up":100}\n{"t":"frame","id":921,"dir":"rx","dlc":8,"d":"FF","bus":0}\n{"t":"sta';

		const result = parseSerialChunk("", chunk);
		expect(result.events).toHaveLength(2);
		expect(result.events[0].type).toBe("message");
		expect((result.events[0] as any).message.t).toBe("status");
		expect(result.events[1].type).toBe("message");
		expect((result.events[1] as any).message.t).toBe("frame");
		expect(result.remainder).toBe('{"t":"sta');
	});

	it("resumes from remainder on next chunk", () => {
		const first = parseSerialChunk("", '{"t":"ack","cmd":"fsd:o');
		expect(first.events).toHaveLength(0);
		expect(first.remainder).toBe('{"t":"ack","cmd":"fsd:o');

		const second = parseSerialChunk(first.remainder, 'n"}\n');
		expect(second.events).toHaveLength(1);
		expect((second.events[0] as any).message.t).toBe("ack");
		expect((second.events[0] as any).message.cmd).toBe("fsd:on");
		expect(second.remainder).toBe("");
	});

	it("handles noisy serial data in chunks", () => {
		const chunk = 'garbage{"t":"boot","variant":"hw4"}\nmore noise\n';
		const result = parseSerialChunk("", chunk);
		// At least one message should be parsed from the JSON in the noise
		const messages = result.events.filter((e) => e.type === "message");
		expect(messages.length).toBeGreaterThanOrEqual(1);
		expect((messages[0] as any).message.t).toBe("boot");
	});
});

// ── Full Session Lifecycle ───────────────────────────────────────────────────

describe("E2E Smoke: Full Session Lifecycle", () => {
	it("simulates a complete user session", () => {
		const sessionLines = [
			// 1. Boot
			serial({
				t: "boot",
				variant: "hw4",
				hw: "ArduinoUnoR3CH340",
				busChassis: true,
				busVehicle: false,
				busBody: false,
				fsd: false,
				nag: false,
				sp: 0,
				offset: 0,
				chassisOnline: true,
			}),
			// 2. User sends variant command → ack
			serial({ t: "ack", cmd: commands.variant("hw4") }),
			// 3. User enables FSD → ack
			serial({ t: "ack", cmd: "fsd:on" }),
			// 4. Status heartbeat reflects FSD on
			serial({ t: "status", up: 5000, rate: 30, fsd: true, nag: false }),
			// 5. User enables streaming → ack
			serial({ t: "ack", cmd: "stream:on" }),
			// 6. Frames arrive
			serial({ t: "frame", id: 0x399, dir: "rx", dlc: 8, d: "AA", bus: 0 }),
			serial({ t: "frame", id: 0x3fd, dir: "rx", dlc: 8, d: "BB", bus: 0 }),
			// 7. User disables streaming
			serial({ t: "ack", cmd: "stream:off" }),
			// 8. User sets profile
			serial({ t: "ack", cmd: "profile:2" }),
			// 9. User enables ban shield
			serial({ t: "ack", cmd: "banshield:on" }),
			// 10. Final status
			serial({
				t: "status",
				up: 10000,
				rate: 28,
				fsd: true,
				sp: 2,
				banShield: 1,
				banThreat: 1,
				banDetectCount: 3,
			}),
		];

		const expectedTypes = [
			"boot",
			"ack",
			"ack",
			"status",
			"ack",
			"frame",
			"frame",
			"ack",
			"ack",
			"ack",
			"status",
		];

		const parsed: Array<{ type: string; message: Record<string, unknown> }> = [];

		// Simulate streaming: feed all lines as a single chunk
		let remainder = "";
		const fullChunk = sessionLines.join("\n") + "\n";
		const result = parseSerialChunk(remainder, fullChunk);
		remainder = result.remainder;

		for (const ev of result.events) {
			if (ev.type === "message") {
				parsed.push({ type: (ev.message as any).t, message: ev.message as any });
			}
		}

		expect(parsed).toHaveLength(expectedTypes.length);
		parsed.forEach((p, i) => {
			expect(p.type).toBe(expectedTypes[i]);
		});

		// Verify session state progression
		const boot = parsed[0].message as any;
		expect(boot.fsd).toBe(false);

		const midStatus = parsed[3].message as any;
		expect(midStatus.fsd).toBe(true);

		const banAck = parsed[9].message as any;
		expect(banAck.cmd).toBe("banshield:on");

		const finalStatus = parsed[10].message as any;
		expect(finalStatus.sp).toBe(2);
		expect(finalStatus.banShield).toBe(1);
		expect(finalStatus.banThreat).toBe(1);
		expect(finalStatus.banDetectCount).toBe(3);

		expect(remainder).toBe("");
	});
});

// ── Multi-Bus Scenario ───────────────────────────────────────────────────────

describe("E2E Smoke: Multi-Bus Scenario", () => {
	it("simulates UNO-3 full bus setup lifecycle", () => {
		// Boot with all 3 buses
		const bootEv = expectMessage(
			serial({
				t: "boot",
				variant: "hw4",
				busChassis: true,
				busVehicle: true,
				busBody: true,
				chassisOnline: true,
			}),
		);
		const boot = bootEv.message as unknown as BootMessage;
		expect(boot.busChassis).toBe(true);
		expect(boot.busVehicle).toBe(true);
		expect(boot.busBody).toBe(true);

		// FSD bus frame
		const fsdFrame = expectMessage(
			serial({ t: "frame", id: 0x399, dir: "rx", dlc: 8, d: "11", bus: 0 }),
		);
		expect((fsdFrame.message as unknown as FrameMessage).bus).toBe(0);

		// Vehicle bus frame
		const vehFrame = expectMessage(
			serial({ t: "frame", id: 0x273, dir: "rx", dlc: 8, d: "22", bus: 1 }),
		);
		expect((vehFrame.message as unknown as FrameMessage).bus).toBe(1);

		// Body bus frame
		const bodyFrame = expectMessage(
			serial({ t: "frame", id: 0x100, dir: "rx", dlc: 4, d: "33", bus: 2 }),
		);
		expect((bodyFrame.message as unknown as FrameMessage).bus).toBe(2);
	});
});

// ── Error Recovery ───────────────────────────────────────────────────────────

describe("E2E Smoke: Error Recovery", () => {
	it("handles parse errors in a stream gracefully", () => {
		const chunk = '{"t":"status","up":1}\nnot json at all\n{"t":"ack","cmd":"ping"}\n';
		const result = parseSerialChunk("", chunk);

		// Should get 3 events: message, parse-error/ignore, message
		expect(result.events.length).toBe(3);

		const types = result.events.map((e) => e.type);
		expect(types[0]).toBe("message");
		expect(types[2]).toBe("message");
		// Middle event should be either parse-error or ignore
		expect(["parse-error", "ignore"]).toContain(types[1]);
	});

	it("handles firmware error followed by recovery", () => {
		// Error message
		const errEv = expectMessage(serial({ t: "error", msg: "CAN bus offline" }));
		expect((errEv.message as unknown as ErrorMessage).msg).toBe("CAN bus offline");

		// Recovery: status shows chassisOnline again
		const statusEv = expectMessage(serial({ t: "status", up: 20000, chassisOnline: true }));
		expect((statusEv.message as unknown as StatusMessage).chassisOnline).toBe(true);
	});
});
