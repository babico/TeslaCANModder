import { afterEach, describe, expect, it } from "@jest/globals";
import type { RequestListener, Server } from "node:http";
import { commands } from "@teslacanmodder/protocol";
import { frameToWire, parseCanFrame, type CanFrame } from "../src/can/socketcan.js";
import { createPiServer } from "../src/index.js";
import { createBridgeListener, MAX_BODY_BYTES } from "../src/http/bridge.js";
import { canFrameBytes } from "./support/fakeSocket.js";

const TOKEN = "test-secret-token";

/** Two genuine A2 frames parsed from literal SocketCAN bytes. */
const FRAMES: CanFrame[] = [
	parseCanFrame(canFrameBytes(0x3fd, 8, [1, 2, 3, 4, 5, 6, 7, 8]), 0, 1000),
	parseCanFrame(canFrameBytes(0x2b9, 2, [0xaa, 0xbb]), 1, 1001),
];

/** Injected fake reader: finite async iterable, no hardware, no socket. */
function fakeReader(frames: readonly CanFrame[]): AsyncIterable<CanFrame> {
	return {
		async *[Symbol.asyncIterator](): AsyncGenerator<CanFrame> {
			for (const frame of frames) yield frame;
		},
	};
}

function bridge(
	overrides: Partial<Parameters<typeof createBridgeListener>[0]> = {},
): RequestListener {
	return createBridgeListener({ token: TOKEN, reader: fakeReader(FRAMES), ...overrides });
}

function authorized(extra: Record<string, string> = {}): Record<string, string> {
	return { authorization: `Bearer ${TOKEN}`, ...extra };
}

describe("authenticated command bridge", () => {
	let server: Server | undefined;

	afterEach(async () => {
		if (!server) return;
		const current = server;
		server = undefined;
		await new Promise<void>((resolve, reject) => {
			current.close((error) => (error ? reject(error) : resolve()));
		});
	});

	async function start(listener: RequestListener): Promise<string> {
		server = createPiServer({ bridge: listener });
		await new Promise<void>((resolve) => {
			server!.listen(0, "127.0.0.1", resolve);
		});
		const address = server.address();
		if (!address || typeof address === "string") throw new Error("expected a TCP address");
		return `http://127.0.0.1:${address.port}`;
	}

	it("rejects POST /api/command without a bearer token (401, fail-closed)", async () => {
		const base = await start(bridge());
		const response = await fetch(`${base}/api/command`, {
			method: "POST",
			headers: { "content-type": "application/json" },
			body: JSON.stringify({ cmd: commands.status() }),
		});
		expect(response.status).toBe(401);
		expect(response.headers.get("www-authenticate")).toBe("Bearer");
		expect(await response.json()).toEqual({ ok: false, error: "unauthorized" });
	});

	it("rejects a wrong bearer token with constant-time comparison (401)", async () => {
		const base = await start(bridge());
		const response = await fetch(`${base}/api/command`, {
			method: "POST",
			headers: {
				authorization: `Bearer ${TOKEN}x`,
				"content-type": "application/json",
			},
			body: JSON.stringify({ cmd: commands.status() }),
		});
		expect(response.status).toBe(401);
		expect(await response.json()).toEqual({ ok: false, error: "unauthorized" });
	});

	it("fails closed when no token is configured (401 for every authenticated route)", async () => {
		const base = await start(bridge({ token: "" }));
		for (const [path, init] of [
			[
				"/api/command",
				{ method: "POST", headers: { "content-type": "application/json" }, body: "{}" },
			],
			["/status", { method: "GET" }],
		] as const) {
			const response = await fetch(`${base}${path}`, init);
			expect(response.status).toBe(401);
		}
	});

	it("rejects POST /api/command without Content-Type: application/json (415)", async () => {
		const base = await start(bridge());
		const response = await fetch(`${base}/api/command`, {
			method: "POST",
			headers: authorized(),
			body: JSON.stringify({ cmd: commands.status() }),
		});
		expect(response.status).toBe(415);
		expect(await response.json()).toEqual({
			ok: false,
			error: "Content-Type must be application/json",
		});
	});

	it("rejects an oversized body over 1 MiB (413)", async () => {
		const base = await start(bridge());
		const response = await fetch(`${base}/api/command`, {
			method: "POST",
			headers: authorized({ "content-type": "application/json" }),
			body: "x".repeat(MAX_BODY_BYTES + 1),
		});
		expect(response.status).toBe(413);
		expect(await response.json()).toEqual({ ok: false, error: "request body too large" });
	});

	it("returns status JSON from the injected A2 reader for the status command", async () => {
		const base = await start(bridge());
		const response = await fetch(`${base}/api/command`, {
			method: "POST",
			headers: authorized({ "content-type": "application/json" }),
			body: JSON.stringify({ cmd: commands.status() }),
		});
		expect(response.status).toBe(200);
		expect(await response.json()).toEqual({
			t: "status",
			frames: 2,
			perBus: [1, 1, 0],
			lastFrame: frameToWire(FRAMES[1]),
		});
	});

	it("serves GET /status (and the /api/status alias) from the same reader", async () => {
		const base = await start(bridge());
		for (const path of ["/status", "/api/status"]) {
			const response = await fetch(`${base}${path}`, { headers: authorized() });
			expect(response.status).toBe(200);
			expect(await response.json()).toEqual({
				t: "status",
				frames: 2,
				perBus: [1, 1, 0],
				lastFrame: frameToWire(FRAMES[1]),
			});
		}
	});

	it("answers ping with the serial-contract pong envelope", async () => {
		const base = await start(bridge());
		const response = await fetch(`${base}/api/command`, {
			method: "POST",
			headers: authorized({ "content-type": "application/json" }),
			body: JSON.stringify({ cmd: commands.ping() }),
		});
		expect(response.status).toBe(200);
		expect(await response.json()).toEqual({ t: "pong", v: 1 });
	});

	it("refuses CAN-TX commands from the registry (unknown command, no injection)", async () => {
		const base = await start(bridge());
		const response = await fetch(`${base}/api/command`, {
			method: "POST",
			headers: authorized({ "content-type": "application/json" }),
			body: JSON.stringify({ cmd: commands.fsd(true) }), // "fsd:on"
		});
		expect(response.status).toBe(400);
		expect(await response.json()).toEqual({ t: "error", msg: "Unknown command" });
	});

	it("rejects malformed or missing cmd bodies (400)", async () => {
		const base = await start(bridge());
		for (const body of ["not json", JSON.stringify({ cmd: 42 })]) {
			const response = await fetch(`${base}/api/command`, {
				method: "POST",
				headers: authorized({ "content-type": "application/json" }),
				body,
			});
			expect(response.status).toBe(400);
			expect(await response.json()).toEqual({ ok: false, error: expect.any(String) });
		}
	});

	it("keeps the A1 /health route and returns 404 for unknown routes", async () => {
		const base = await start(bridge());
		const health = await fetch(`${base}/health`);
		expect(health.status).toBe(200);
		expect(await health.json()).toEqual({ ok: true, transport: expect.any(String) });

		const missing = await fetch(`${base}/nope`);
		expect(missing.status).toBe(404);
	});
});
