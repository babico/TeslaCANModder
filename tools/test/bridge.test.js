import { afterEach, describe, expect, it, jest } from "@jest/globals";

import {
	BRIDGE_HELP,
	MAX_BODY_BYTES,
	closeBridgeServer,
	createBridgeServer,
	parseBridgeArgs,
} from "../serial-http-bridge.js";

const TOKEN = "test-token-123";

function makeSession() {
	return {
		send: jest.fn(),
		waitFor: jest.fn(async () => ({ msg: { t: "ack", cmd: "status" } })),
		close: jest.fn(),
	};
}

let server = null;

async function startBridge(session, options = {}) {
	server = createBridgeServer({
		session,
		serial: { isOpen: true },
		options: {
			token: TOKEN,
			host: "127.0.0.1",
			listen: 0,
			timeoutMs: 100,
			port: "COM4",
			baud: 115200,
			...options,
		},
	});
	await new Promise((resolve) => server.listen(0, "127.0.0.1", resolve));
	return `http://127.0.0.1:${server.address().port}`;
}

afterEach(async () => {
	if (server) await new Promise((resolve) => server.close(resolve));
	server = null;
});

describe("bridge defaults and help", () => {
	it("defaults to loopback bind", () => {
		expect(parseBridgeArgs(["node", "bridge", "--port", "COM4"]).host).toBe("127.0.0.1");
	});

	it("help text advertises loopback default and token flag", () => {
		expect(BRIDGE_HELP).toContain("--host 127.0.0.1");
		expect(BRIDGE_HELP).toContain("--token");
	});

	it("parses every bridge flag through the shared argument parser", () => {
		const opts = parseBridgeArgs([
			"node",
			"bridge",
			"--port",
			"COM7",
			"--baud",
			"921600",
			"--listen",
			"9090",
			"--host",
			"127.0.0.2",
			"--timeout",
			"5000",
			"--token",
			"explicit-token",
			"--allow-origin",
			"none",
		]);

		expect(opts).toMatchObject({
			port: "COM7",
			baud: 921600,
			listen: 9090,
			host: "127.0.0.2",
			timeoutMs: 5000,
			token: "explicit-token",
			allowOrigin: null,
		});
	});

	it("rejects a wildcard CORS origin instead of emitting it verbatim", () => {
		expect(() =>
			parseBridgeArgs(["node", "bridge", "--port", "COM4", "--allow-origin", "*"]),
		).toThrow(/wildcard/i);
	});

	it("derives the default CORS origin from host and listen", () => {
		const opts = parseBridgeArgs([
			"node",
			"bridge",
			"--port",
			"COM4",
			"--host",
			"127.0.0.2",
			"--listen",
			"9090",
		]);
		expect(opts.allowOrigin).toBe("http://127.0.0.2:9090");
	});
});

describe("bridge authentication", () => {
	it("rejects command POST without a token (401)", async () => {
		const session = makeSession();
		const base = await startBridge(session);

		const res = await fetch(`${base}/api/command`, {
			method: "POST",
			headers: { "Content-Type": "application/json" },
			body: JSON.stringify({ cmd: "status" }),
		});

		expect(res.status).toBe(401);
		expect(session.send).not.toHaveBeenCalled();
	});

	it("rejects command POST with a wrong token (401)", async () => {
		const session = makeSession();
		const base = await startBridge(session);

		const res = await fetch(`${base}/api/command`, {
			method: "POST",
			headers: {
				"Content-Type": "application/json",
				Authorization: "Bearer wrong-token",
			},
			body: JSON.stringify({ cmd: "status" }),
		});

		expect(res.status).toBe(401);
		expect(session.send).not.toHaveBeenCalled();
	});

	it("accepts command POST with the configured bearer token", async () => {
		const session = makeSession();
		const base = await startBridge(session);

		const res = await fetch(`${base}/api/command`, {
			method: "POST",
			headers: {
				"Content-Type": "application/json",
				Authorization: `Bearer ${TOKEN}`,
			},
			body: JSON.stringify({ cmd: "status" }),
		});

		expect(res.status).toBe(200);
		expect(session.send).toHaveBeenCalledWith("status");
	});

	it("protects status reads but leaves health public", async () => {
		const session = makeSession();
		const base = await startBridge(session);

		const health = await fetch(`${base}/health`);
		const status = await fetch(`${base}/api/status`);

		expect(health.status).toBe(200);
		expect(status.status).toBe(401);
		expect(session.send).not.toHaveBeenCalled();
	});
});

describe("bridge request limits", () => {
	it("rejects an oversized command body with 413", async () => {
		const session = makeSession();
		const base = await startBridge(session);

		const res = await fetch(`${base}/api/command`, {
			method: "POST",
			headers: {
				"Content-Type": "application/json",
				Authorization: `Bearer ${TOKEN}`,
			},
			body: "x".repeat(MAX_BODY_BYTES + 1),
		});

		expect(res.status).toBe(413);
		expect(session.send).not.toHaveBeenCalled();
	});

	it("does not emit a wildcard CORS origin", async () => {
		const base = await startBridge(makeSession(), {
			allowOrigin: "http://localhost:8081",
		});

		const res = await fetch(`${base}/health`, {
			headers: { Origin: "http://localhost:8081" },
		});

		expect(res.headers.get("access-control-allow-origin")).toBe("http://localhost:8081");
		expect(res.headers.get("access-control-allow-origin")).not.toBe("*");
	});

	it("sets request and header timeouts", async () => {
		await startBridge(makeSession());
		expect(server.timeout).toBe(30000);
		expect(server.headersTimeout).toBe(5000);
	});
});

describe("bridge request validation", () => {
	it("rejects command POST without application/json Content-Type (415)", async () => {
		const session = makeSession();
		const base = await startBridge(session);

		const res = await fetch(`${base}/api/command`, {
			method: "POST",
			headers: {
				"Content-Type": "text/plain",
				Authorization: `Bearer ${TOKEN}`,
			},
			body: JSON.stringify({ cmd: "status" }),
		});

		expect(res.status).toBe(415);
		expect(session.send).not.toHaveBeenCalled();
	});

	it("accepts application/json with a charset parameter", async () => {
		const session = makeSession();
		const base = await startBridge(session);

		const res = await fetch(`${base}/api/command`, {
			method: "POST",
			headers: {
				"Content-Type": "application/json; charset=utf-8",
				Authorization: `Bearer ${TOKEN}`,
			},
			body: JSON.stringify({ cmd: "status" }),
		});

		expect(res.status).toBe(200);
		expect(session.send).toHaveBeenCalledWith("status");
	});

	it("returns parse-error entries with error detail instead of empty text", async () => {
		const session = makeSession();
		session.waitFor.mockResolvedValueOnce({
			raw: '{"t":',
			msg: null,
			type: "parse-error",
			error: "Unexpected end of JSON input",
		});
		const base = await startBridge(session);

		const res = await fetch(`${base}/api/command`, {
			method: "POST",
			headers: {
				"Content-Type": "application/json",
				Authorization: `Bearer ${TOKEN}`,
			},
			body: JSON.stringify({ cmd: "status" }),
		});
		const body = await res.json();

		expect(body.t).toBe("parse-error");
		expect(body.raw).toBe('{"t":');
		expect(body.error).toEqual(expect.any(String));
	});

	it("closes a partially started server on boot failure", async () => {
		const close = jest.fn((cb) => cb());
		await closeBridgeServer({ close, listening: true });

		expect(close).toHaveBeenCalledTimes(1);
	});
});
