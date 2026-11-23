import { afterEach, describe, expect, it } from "@jest/globals";
import type { Server } from "node:http";
import { commands } from "@teslacanmodder/protocol";
import { createPiServer, getCanTransport } from "../src/index.js";

describe("pi workspace scaffold", () => {
	let server: Server | undefined;

	afterEach(async () => {
		if (!server) return;
		const current = server;
		server = undefined;
		await new Promise<void>((resolve, reject) => {
			current.close((error) => (error ? reject(error) : resolve()));
		});
	});

	it("resolves the shared protocol workspace", () => {
		expect(typeof commands.status).toBe("function");
		expect(commands.status()).toBe("status");
	});

	it("exports createPiServer and serves GET /health", async () => {
		expect(typeof createPiServer).toBe("function");

		server = createPiServer({ transport: "socketcan" });
		await new Promise<void>((resolve) => {
			server!.listen(0, "127.0.0.1", resolve);
		});
		const address = server.address();
		if (!address || typeof address === "string") {
			throw new Error("expected a TCP address");
		}
		const response = await fetch(`http://127.0.0.1:${address.port}/health`);
		expect(response.status).toBe(200);
		expect(response.headers.get("content-type")).toContain("application/json");
		expect(await response.json()).toEqual({ ok: true, transport: "socketcan" });
	});

	it("mounts the A3 bridge listener for non-health routes", async () => {
		server = createPiServer({
			bridge: (_request, response) => {
				response.writeHead(204);
				response.end();
			},
		});
		await new Promise<void>((resolve) => {
			server!.listen(0, "127.0.0.1", resolve);
		});
		const address = server.address();
		if (!address || typeof address === "string") {
			throw new Error("expected a TCP address");
		}
		const response = await fetch(`http://127.0.0.1:${address.port}/api/command`);
		expect(response.status).toBe(204);
	});

	it("defaults CAN_TRANSPORT to socketcan and honours vcan", () => {
		expect(getCanTransport({})).toBe("socketcan");
		expect(getCanTransport({ CAN_TRANSPORT: "vcan" })).toBe("vcan");
	});
});
