import { timingSafeEqual } from "node:crypto";
import type { IncomingMessage, RequestListener, ServerResponse } from "node:http";

import { commands } from "@teslacanmodder/protocol";
import { frameToWire, type CanFrame, type WireFrame } from "../can/socketcan.js";

/** Hardened body cap, mirrored from `tools/serial-http-bridge.js`. */
export const MAX_BODY_BYTES = 1024 * 1024;
export const SERVER_TIMEOUT_MS = 30_000;
export const HEADERS_TIMEOUT_MS = 5_000;

/** Status snapshot built from frames produced by the A2 reader. */
export interface CanStatus {
	t: "status";
	frames: number;
	perBus: [number, number, number];
	lastFrame?: WireFrame;
}

export interface BridgeOptions {
	/**
	 * Bearer token. Empty/undefined fails closed — every authenticated route
	 * returns 401. Env var `TCM_BRIDGE_TOKEN` is the production source.
	 */
	token?: string;
	/** Read-only A2 CAN reader. Never used for TX. */
	reader: AsyncIterable<CanFrame>;
	/** Reserved for callers that need an explicit command wait budget. */
	timeoutMs?: number;
}

/**
 * Wire strings come from the shared protocol registry, not a second command
 * list. Only read-only commands are exposed; every TX/injection builder in the
 * registry (fsd, summon, drive, ...) is deliberately absent — this bridge has
 * no CAN TX backend at all.
 */
const STATUS_WIRE = commands.status();
const PING_WIRE = commands.ping();
const ALLOWED_COMMANDS = new Set<string>([STATUS_WIRE, PING_WIRE]);

function headerValue(req: IncomingMessage, name: string): string {
	const value = req.headers[name];
	return Array.isArray(value) ? (value[0] ?? "") : (value ?? "");
}

/** Constant-time bearer comparison; no token configured means no access. */
function isAuthorized(req: IncomingMessage, token: string): boolean {
	if (!token) return false;
	const match = /^Bearer\s+(.+)$/i.exec(headerValue(req, "authorization").trim());
	if (!match) return false;
	const provided = Buffer.from(match[1] ?? "");
	const expected = Buffer.from(token);
	if (provided.length !== expected.length) return false;
	return timingSafeEqual(provided, expected);
}

function isJsonContentType(req: IncomingMessage): boolean {
	const value = headerValue(req, "content-type");
	if (!value) return false;
	return value.split(";", 1)[0]?.trim().toLowerCase() === "application/json";
}

function sendJson(res: ServerResponse, status: number, body: unknown): void {
	res.writeHead(status, {
		"content-type": "application/json; charset=utf-8",
		"cache-control": "no-store",
	});
	res.end(JSON.stringify(body));
}

function fail(res: ServerResponse, status: number, error: string): void {
	sendJson(res, status, { ok: false, error });
}

function rejectUnauthorized(res: ServerResponse): void {
	res.setHeader("WWW-Authenticate", "Bearer");
	fail(res, 401, "unauthorized");
}

/** Drain the request without retaining it once the cap is exceeded. */
async function readBody(req: IncomingMessage, maxBytes: number): Promise<string> {
	const chunks: Buffer[] = [];
	let size = 0;
	let tooLarge = false;

	for await (const chunk of req) {
		const buffer = Buffer.from(chunk);
		size += buffer.length;
		if (size > maxBytes) {
			tooLarge = true;
			continue;
		}
		chunks.push(buffer);
	}

	if (tooLarge) {
		const error = new Error("request body too large") as Error & { statusCode: number };
		error.statusCode = 413;
		throw error;
	}
	return Buffer.concat(chunks).toString("utf8");
}

interface StatusTracker {
	snapshot(): CanStatus;
}

/**
 * Pump the read-only A2 reader into a bounded snapshot. Transport errors end
 * the pump quietly; an HTTP status request never blocks on a live bus.
 */
function createStatusTracker(reader: AsyncIterable<CanFrame>): StatusTracker {
	let frames = 0;
	const perBus: [number, number, number] = [0, 0, 0];
	let lastFrame: WireFrame | undefined;

	void (async () => {
		try {
			for await (const frame of reader) {
				frames += 1;
				if (frame.bus >= 0 && frame.bus <= 2) perBus[frame.bus] += 1;
				lastFrame = frameToWire(frame);
			}
		} catch {
			// A broken reader must not take down the HTTP service.
		}
	})();

	return {
		snapshot(): CanStatus {
			return {
				t: "status",
				frames,
				perBus: [...perBus] as [number, number, number],
				...(lastFrame ? { lastFrame } : {}),
			};
		},
	};
}

/** Response envelope follows `docs/reference/serial-contract.md` (`t` field). */
function dispatchCommand(cmd: string, tracker: StatusTracker): { status: number; body: unknown } {
	if (!ALLOWED_COMMANDS.has(cmd)) {
		return { status: 400, body: { t: "error", msg: "Unknown command" } };
	}
	if (cmd === STATUS_WIRE) return { status: 200, body: tracker.snapshot() };
	return { status: 200, body: { t: "pong", v: 1 } };
}

/**
 * Build the request listener mounted by `createPiServer()` as its `bridge`.
 * `/health` is handled here only for standalone use; the Pi server serves its
 * own `/health` first. The listener binds no socket of its own (loopback bind
 * is the caller's decision).
 */
export function createBridgeListener(options: BridgeOptions): RequestListener {
	const token = options.token ?? process.env.TCM_BRIDGE_TOKEN ?? "";
	const tracker = createStatusTracker(options.reader);

	return (req: IncomingMessage, res: ServerResponse): void => {
		void (async () => {
			try {
				if (req.method === "OPTIONS") {
					res.writeHead(204, { allow: "GET, POST, OPTIONS" });
					res.end();
					return;
				}

				const url = req.url ?? "/";

				if (req.method === "GET" && url === "/health") {
					sendJson(res, 200, { ok: true, mode: "pi-command-bridge" });
					return;
				}

				if (req.method === "GET" && (url === "/status" || url === "/api/status")) {
					if (!isAuthorized(req, token)) {
						rejectUnauthorized(res);
						return;
					}
					sendJson(res, 200, tracker.snapshot());
					return;
				}

				if (req.method === "POST" && url === "/api/command") {
					if (!isAuthorized(req, token)) {
						rejectUnauthorized(res);
						return;
					}
					if (!isJsonContentType(req)) {
						fail(res, 415, "Content-Type must be application/json");
						return;
					}

					const raw = await readBody(req, MAX_BODY_BYTES);
					let parsed: unknown;
					try {
						parsed = JSON.parse(raw || "{}");
					} catch {
						fail(res, 400, "invalid JSON body");
						return;
					}

					const cmd =
						parsed && typeof parsed === "object" && "cmd" in parsed
							? (parsed as { cmd?: unknown }).cmd
							: undefined;
					if (typeof cmd !== "string" || !cmd.trim()) {
						fail(res, 400, "missing cmd");
						return;
					}

					const result = dispatchCommand(cmd.trim(), tracker);
					sendJson(res, result.status, result.body);
					return;
				}

				fail(res, 404, "not found");
			} catch (error) {
				const statusCode =
					typeof error === "object" &&
					error !== null &&
					"statusCode" in error &&
					Number.isInteger((error as { statusCode?: unknown }).statusCode)
						? (error as { statusCode: number }).statusCode
						: 500;
				const message = error instanceof Error ? error.message : "unknown error";
				fail(res, statusCode, message);
			}
		})();
	};
}
