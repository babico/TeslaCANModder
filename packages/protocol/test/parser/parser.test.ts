import { parseSerialLine, parseSerialChunk } from "../../src/parser.js";

describe("parseSerialLine", () => {
	it("extracts valid JSON from a line", () => {
		const events = parseSerialLine('{"t":"boot","variant":"hw4"}');
		expect(events).toHaveLength(1);
		expect(events[0].type).toBe("message");
		expect(events[0].message).toEqual({ t: "boot", variant: "hw4" });
	});

	it("extracts JSON from noisy line", () => {
		const events = parseSerialLine('noise {"t":"pong"} more');
		expect(events).toHaveLength(1);
		expect(events[0].type).toBe("message");
		expect(events[0].message?.t).toBe("pong");
	});

	it("returns ignore for non-JSON lines", () => {
		const events = parseSerialLine("just plain text");
		expect(events).toHaveLength(1);
		expect(events[0].type).toBe("ignore");
	});

	it("returns parse-error for invalid JSON", () => {
		const events = parseSerialLine("{bad json}");
		expect(events).toHaveLength(1);
		expect(events[0].type).toBe("parse-error");
		expect(events[0].reason).toBeDefined();
	});

	it("returns empty for empty string", () => {
		expect(parseSerialLine("")).toEqual([]);
	});

	it("returns empty for null bytes only", () => {
		expect(parseSerialLine("\0\0\0")).toEqual([]);
	});

	it("returns empty for whitespace only", () => {
		expect(parseSerialLine("   ")).toEqual([]);
	});

	it("handles nested JSON", () => {
		const events = parseSerialLine('{"t":"status","stream":{"on":true}}');
		expect(events).toHaveLength(1);
		expect(events[0].message?.stream).toEqual({ on: true });
	});

	it("normalizes structured boot payloads from hardware JSON", () => {
		const events = parseSerialLine(
			'{"t":"boot","meta":{"variant":"hw3","hw":"ESP32S_DevKit","drv":"mcp2515"},"connectivity":{"chassisOnline":1,"vehicleOnline":0,"bus":{"chassis":1,"vehicle":1,"body":0}},"state":{"fsd":1,"profile":{"value":3,"pinned":1},"offset":{"value":5,"pinned":0},"gtwAutopilotSeen":1},"vehicle":{"vehicleLockedState":1},"platform":{"model":2,"hwGen":2,"swYear":2026,"swWeek":14,"swRelease":1,"fsdProto":1,"swCompat":1,"resolved":1},"firmware":{"year":2026,"release":14,"minor":1,"fwBuild":12345,"compat":1,"hasVersion":1,"mqtt":0,"mqttConnected":0},"can":{"clockReqMHz":8,"clockMHz":8,"health":{"chassis":{"on":1,"det":1},"vehicle":{"on":1,"det":0},"body":{"on":0,"det":0}}},"features":{"fsd":true}}',
		);

		expect(events).toHaveLength(1);
		expect(events[0].type).toBe("message");
		expect(events[0].message).toMatchObject({
			t: "boot",
			variant: "hw3",
			hw: "ESP32S_DevKit",
			drv: "mcp2515",
			chassisOnline: 1,
			vehicleOnline: 0,
			busChassis: 1,
			busVehicle: 1,
			busBody: 0,
			fsd: 1,
			sp: 3,
			spPin: 1,
			offset: 5,
			offsetPin: 0,
			gtwAutopilotSeen: 1,
			vehicleLockedState: 1,
			platformModel: 2,
			platformHwGen: 2,
			platformResolved: 1,
			fwYear: 2026,
			fwRelease: 14,
			fwMinor: 1,
			fwBuild: 12345,
			fwCompat: 1,
			hasFwVersion: 1,
			canClockReqMHz: 8,
			canClockMHz: 8,
			canHealth: {
				chassis: { on: 1, det: 1 },
				vehicle: { on: 1, det: 0 },
				body: { on: 0, det: 0 },
			},
			features: { fsd: true },
		});
	});

	it("normalizes structured status payloads without dropping nested sections", () => {
		const events = parseSerialLine(
			'{"t":"status","meta":{"variant":"hw4","hw":"ESP32","drv":"native","up":1234},"state":{"stream":{"on":1,"emitted":7},"nagKiller":1,"rawCan":1},"driverAssist":{"maxSpeed":860},"platform":{"model":1,"hwGen":3,"swYear":2026,"swWeek":14,"swRelease":1,"fsdProto":3,"swCompat":1,"resolved":1},"firmware":{"year":2026,"release":14,"minor":1,"compat":1,"hasVersion":1,"mqtt":1,"mqttConnected":0},"battery":{"nomFullPack":7123},"safety":{"banShield":1,"gtwShieldArmed":0},"can":{"clockReqMHz":16,"clockMHz":16,"health":{"chassis":{"on":1,"det":1},"vehicle":{"on":1,"det":1},"body":{"on":1,"det":0}}}}',
		);

		expect(events).toHaveLength(1);
		expect(events[0].type).toBe("message");
		expect(events[0].message).toMatchObject({
			t: "status",
			variant: "hw4",
			hw: "ESP32",
			drv: "native",
			up: 1234,
			nagKiller: 1,
			stream: { on: 1, emitted: 7 },
			rawCan: 1,
			maxSpeed: 860,
			bmsNomFullPack: 7123,
			platformModel: 1,
			platformResolved: 1,
			fwYear: 2026,
			mqtt: 1,
			banShield: 1,
			gtwShieldArmed: 0,
			canClockReqMHz: 16,
			canClockMHz: 16,
			canHealth: {
				chassis: { on: 1, det: 1 },
				vehicle: { on: 1, det: 1 },
				body: { on: 1, det: 0 },
			},
			meta: { variant: "hw4", hw: "ESP32", drv: "native", up: 1234 },
		});
	});

	it("normalizes split status_can payloads", () => {
		const events = parseSerialLine(
			'{"t":"status_can","clock":{"reqMHz":16,"activeMHz":8},"health":{"chassis":{"on":1,"det":1},"vehicle":{"on":1,"det":0},"body":{"on":0,"det":0}}}',
		);

		expect(events).toHaveLength(1);
		expect(events[0].message).toMatchObject({
			t: "status_can",
			canClockReqMHz: 16,
			canClockMHz: 8,
			canHealth: {
				chassis: { on: 1, det: 1 },
				vehicle: { on: 1, det: 0 },
				body: { on: 0, det: 0 },
			},
		});
	});

	it("normalizes split status_state payloads", () => {
		const events = parseSerialLine(
			'{"t":"status_state","state":{"fsd":1,"fsdForce":0,"nag":1,"nagKiller":1,"profile":{"value":2,"pinned":1},"offset":{"value":7,"pinned":0},"precondition":0,"trackMode":1,"apGateEnabled":1,"apGateOpen":0,"apGateReason":"waiting","gtwAutopilotSeen":1}}',
		);

		expect(events).toHaveLength(1);
		expect(events[0].message).toMatchObject({
			t: "status_state",
			fsd: 1,
			fsdForce: 0,
			nag: 1,
			nagKiller: 1,
			sp: 2,
			spPin: 1,
			offset: 7,
			offsetPin: 0,
			precondition: 0,
			trackMode: 1,
			apGateEnabled: 1,
			apGateOpen: 0,
			apGateReason: "waiting",
			gtwAutopilotSeen: 1,
		});
	});

	it("normalizes compact status payloads", () => {
		const events = parseSerialLine(
			'{"t":"status_compact","meta":{"hw":"ESP32S_DevKit","variant":"hw4","ready":"runtime-ready","up":999},"connectivity":{"bt":1,"wifi":1,"chassisOnline":1,"vehicleOnline":1,"bodyOnline":0,"standby":0},"state":{"fsd":1,"fsdForce":1,"nag":0,"profile":3,"offset":12,"precondition":0,"trackMode":1,"apGateEnabled":1,"apGateOpen":1,"apGateReason":"parked"},"features":{"fsd":true,"fsdForce":true,"profile":true,"nag":true,"offset":true,"isaSpeedChime":true,"summon":true},"can":{"clock":{"reqMHz":8,"activeMHz":8},"health":{"chassis":{"on":1,"det":1},"vehicle":{"on":1,"det":1},"body":{"on":0,"det":0}}},"stream":{"on":1,"emitted":44},"rawCan":0}',
		);

		expect(events).toHaveLength(1);
		expect(events[0].message).toMatchObject({
			t: "status_compact",
			hw: "ESP32S_DevKit",
			variant: "hw4",
			up: 999,
			chassisOnline: 1,
			vehicleOnline: 1,
			bodyOnline: 0,
			standby: 0,
			fsd: 1,
			fsdForce: 1,
			sp: 3,
			offset: 12,
			apGateEnabled: 1,
			apGateOpen: 1,
			apGateReason: "parked",
			canClockReqMHz: 8,
			canClockMHz: 8,
			stream: { on: 1, emitted: 44 },
		});
	});

	it("parses ban shield fields from status payload", () => {
		const events = parseSerialLine(
			'{"t":"status","banShield":1,"banThreat":2,"banDetectCount":7}',
		);
		expect(events).toHaveLength(1);
		expect(events[0].type).toBe("message");
		expect(events[0].message?.banShield).toBe(1);
		expect(events[0].message?.banThreat).toBe(2);
		expect(events[0].message?.banDetectCount).toBe(7);
	});

	it("normalizes fwBuild, vehicleLockedState, and gtwAutopilotSeen from boot payload", () => {
		const events = parseSerialLine(
			'{"t":"boot","state":{"gtwAutopilotSeen":1},"vehicle":{"vehicleLockedState":0},"firmware":{"fwBuild":98765}}',
		);
		expect(events).toHaveLength(1);
		expect(events[0].type).toBe("message");
		expect(events[0].message).toMatchObject({
			t: "boot",
			fwBuild: 98765,
			vehicleLockedState: 0,
			gtwAutopilotSeen: 1,
		});
	});

	it("normalizes CAN diagnostic counters and per-bus metrics from status payload", () => {
		const events = parseSerialLine(
			'{"t":"status","can":{"nagEchoCount":42,"eapModCount":7,"txFailCount":3,"busOffCount":1,"frames":{"chassis":1234,"vehicle":5678,"body":90},"hz":{"chassis":450,"vehicle":220,"body":80},"hzMin":{"chassis":400,"vehicle":200,"body":70},"hzMax":{"chassis":500,"vehicle":250,"body":95}}}',
		);
		expect(events).toHaveLength(1);
		expect(events[0].type).toBe("message");
		expect(events[0].message).toMatchObject({
			t: "status",
			canNagEchoCount: 42,
			canEapModCount: 7,
			canTxFailCount: 3,
			canBusOffCount: 1,
			canFrames: { chassis: 1234, vehicle: 5678, body: 90 },
			canHz: { chassis: 450, vehicle: 220, body: 80 },
			canHzMin: { chassis: 400, vehicle: 200, body: 70 },
			canHzMax: { chassis: 500, vehicle: 250, body: 95 },
		});
	});

	it("normalizes CAN diagnostic fields from boot payload", () => {
		const events = parseSerialLine(
			'{"t":"boot","can":{"nagEchoCount":0,"eapModCount":0,"txFailCount":0,"busOffCount":0,"frames":{"chassis":100,"vehicle":0,"body":0}}}',
		);
		expect(events).toHaveLength(1);
		expect(events[0].type).toBe("message");
		expect(events[0].message).toMatchObject({
			t: "boot",
			canNagEchoCount: 0,
			canEapModCount: 0,
			canTxFailCount: 0,
			canBusOffCount: 0,
			canFrames: { chassis: 100, vehicle: 0, body: 0 },
		});
	});

	it("strips null bytes before parsing", () => {
		const events = parseSerialLine('\0{"t":"pong"}\0');
		expect(events).toHaveLength(1);
		expect(events[0].type).toBe("message");
	});
});

describe("parseSerialChunk", () => {
	it("splits chunk on newlines", () => {
		const { events, remainder } = parseSerialChunk("", '{"t":"boot"}\n{"t":"pong"}\n');
		expect(events).toHaveLength(2);
		expect(remainder).toBe("");
	});

	it("carries partial line into remainder", () => {
		const { events, remainder } = parseSerialChunk("", '{"t":"bo');
		expect(events).toHaveLength(0);
		expect(remainder).toBe('{"t":"bo');
	});

	it("joins remainder with next chunk", () => {
		const r1 = parseSerialChunk("", '{"t":"bo');
		const r2 = parseSerialChunk(r1.remainder, 'ot"}\n');
		expect(r2.events).toHaveLength(1);
		expect(r2.events[0].type).toBe("message");
		expect(r2.events[0].message?.t).toBe("boot");
	});

	it("handles multiple chunks with leftovers", () => {
		const r1 = parseSerialChunk("", '{"t":"a"}\n{"t":');
		expect(r1.events).toHaveLength(1);
		expect(r1.remainder).toBe('{"t":');

		const r2 = parseSerialChunk(r1.remainder, '"b"}\n');
		expect(r2.events).toHaveLength(1);
		expect(r2.events[0].message?.t).toBe("b");
		expect(r2.remainder).toBe("");
	});
});
