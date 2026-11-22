#!/usr/bin/env node
/**
 * Serial HTTP Bridge
 *
 * Exposes ESP32 serial commands as REST endpoints for local tooling.
 * Loopback-only by default. Every serial-touching endpoint requires a bearer
 * token supplied via `--token` or `TCM_BRIDGE_TOKEN`.
 *
 * Usage:
 *   node tools/serial-http-bridge.js --port COM4 --baud 115200 --listen 8080 \
 *     --token my-secret
 *
 * Endpoints:
 *   GET  /health          (public liveness probe)
 *   GET  /api/status      (bearer token required)
 *   POST /api/command     (bearer token required) { "cmd": "status" }
 */

import http from "node:http";
import { timingSafeEqual } from "node:crypto";
import { pathToFileURL } from "node:url";

import { parseArgs } from "./lib/args.js";
import { openSerial, BoardSession } from "./lib/session.js";

export const MAX_BODY_BYTES = 1024 * 1024;
const SERVER_TIMEOUT_MS = 30000;
const HEADERS_TIMEOUT_MS = 5000;

export const BRIDGE_HELP = [
	"Usage: node tools/serial-http-bridge.js --port COM4 [--baud 115200] [--listen 8080]",
	"       [--host 127.0.0.1] [--timeout 3000] --token <secret>",
	"",
	"Token: --token <secret> or TCM_BRIDGE_TOKEN env var (required for /api/*).",
	"CORS:  --allow-origin <origin> or 'none' (default: http://<host>:<listen>).",
	"       Wildcard '*' is rejected — name an explicit origin.",
].join("\n");

function printHelpAndExit(code) {
	console.log(BRIDGE_HELP);
	process.exit(code);
}

export function parseBridgeArgs(argv) {
	// Flag tokenization lives in the shared parser; only bridge defaults and
	// post-processing (token env fallback, CORS origin) stay local.
	const args = parseArgs(argv.slice(2));

	if (args["allow-origin"] === "*") {
		throw new Error("wildcard CORS origin is not allowed; use an explicit origin or 'none'");
	}
	let allowOrigin;
	if (args["allow-origin"] === "none") allowOrigin = null;
	else if (args["allow-origin"] !== undefined) allowOrigin = String(args["allow-origin"]);

	const out = {
		port: typeof args.port === "string" ? args.port : "",
		baud: typeof args.baud === "string" ? Number(args.baud) || 115200 : 115200,
		listen: typeof args.listen === "string" ? Number(args.listen) || 8080 : 8080,
		host: typeof args.host === "string" ? args.host : "127.0.0.1",
		timeoutMs: typeof args.timeout === "string" ? Number(args.timeout) || 3000 : 3000,
		token:
			(typeof args.token === "string" ? args.token : "") ||
			process.env.TCM_BRIDGE_TOKEN ||
			"",
		allowOrigin,
	};

	if (args.help || args.h) printHelpAndExit(0);
	if (!out.port) printHelpAndExit(1);
	if (out.allowOrigin === undefined) {
		out.allowOrigin = `http://${out.host}:${out.listen}`;
	}

	return out;
}

function json(res, status, body, allowOrigin) {
	const headers = {
		"Content-Type": "application/json; charset=utf-8",
		"Cache-Control": "no-store",
	};
	if (allowOrigin) {
		headers["Access-Control-Allow-Origin"] = allowOrigin;
		headers["Access-Control-Allow-Headers"] = "Content-Type, Authorization";
		headers["Access-Control-Allow-Methods"] = "GET,POST,OPTIONS";
	}
	res.writeHead(status, headers);
	res.end(JSON.stringify(body));
}

async function readBody(req, maxBytes) {
	const chunks = [];
	let size = 0;
	let tooLarge = false;

	for await (const chunk of req) {
		const buffer = Buffer.from(chunk);
		size += buffer.length;
		if (size > maxBytes) {
			tooLarge = true;
			continue; // drain the rest without retaining it
		}
		chunks.push(buffer);
	}

	if (tooLarge) {
		const error = new Error("request body too large");
		error.statusCode = 413;
		throw error;
	}
	return Buffer.concat(chunks).toString("utf8");
}

function headerValue(req, name) {
	const value = req.headers[name];
	return Array.isArray(value) ? value[0] : value;
}

function isAuthorized(req, token) {
	if (!token || typeof token !== "string") return false;
	const header = headerValue(req, "authorization") || "";
	const match = /^Bearer\s+(.+)$/i.exec(header.trim());
	if (!match) return false;

	const provided = Buffer.from(match[1]);
	const expected = Buffer.from(token);
	if (provided.length !== expected.length) return false;
	return timingSafeEqual(provided, expected);
}

function normalizeEntry(entry) {
	if (!entry) {
		return null;
	}
	if (entry.type === "parse-error") {
		return {
			t: "parse-error",
			raw: entry.raw ?? "",
			error: entry.error ?? "invalid JSON",
		};
	}
	if (entry.msg) {
		return entry.msg;
	}
	return { t: "text", raw: entry.raw ?? "" };
}

function isJsonContentType(req) {
	const value = headerValue(req, "content-type");
	if (typeof value !== "string") return false;
	const mediaType = value.split(";", 1)[0].trim().toLowerCase();
	return mediaType === "application/json";
}

/** Close a server that was started before a later boot step failed. */
export async function closeBridgeServer(server) {
	if (!server || !server.listening) return;
	await new Promise((resolve) => server.close(() => resolve()));
}

export function createBridgeServer({ session, serial = {}, options = {} }) {
	const {
		token = process.env.TCM_BRIDGE_TOKEN || "",
		host = "127.0.0.1",
		listen = 8080,
		timeoutMs = 3000,
		allowOrigin = `http://${host}:${listen}`,
		port = "",
		baud = 115200,
	} = options;

	const sendJson = (res, status, body) => json(res, status, body, allowOrigin);
	const rejectUnauthorized = (res) => {
		res.setHeader("WWW-Authenticate", "Bearer");
		sendJson(res, 401, { ok: false, error: "unauthorized" });
	};

	const server = http.createServer(async (req, res) => {
		try {
			if (req.method === "OPTIONS") {
				sendJson(res, 200, { ok: true });
				return;
			}

			const url = req.url || "/";

			if (req.method === "GET" && url === "/health") {
				sendJson(res, 200, {
					ok: true,
					mode: "serial-http-bridge",
					port,
					baud,
					listen,
					timeoutMs,
					serialOpen: Boolean(serial.isOpen),
				});
				return;
			}

			if (req.method === "GET" && url === "/api/status") {
				if (!isAuthorized(req, token)) {
					rejectUnauthorized(res);
					return;
				}
				session.send("status");
				const entry = await session.waitFor(
					(item) => item.msg?.t === "status" || item.msg?.t === "boot",
					timeoutMs,
				);
				if (!entry) {
					sendJson(res, 504, { ok: false, error: "status timeout" });
					return;
				}
				sendJson(res, 200, normalizeEntry(entry));
				return;
			}

			if (req.method === "POST" && url === "/api/command") {
				if (!isAuthorized(req, token)) {
					rejectUnauthorized(res);
					return;
				}

				if (!isJsonContentType(req)) {
					sendJson(res, 415, {
						ok: false,
						error: "Content-Type must be application/json",
					});
					return;
				}

				const raw = await readBody(req, MAX_BODY_BYTES);
				let cmd = "";

				try {
					const parsed = JSON.parse(raw || "{}");
					cmd = typeof parsed.cmd === "string" ? parsed.cmd.trim() : "";
				} catch {
					sendJson(res, 400, { ok: false, error: "invalid JSON body" });
					return;
				}

				if (!cmd) {
					sendJson(res, 400, { ok: false, error: "missing cmd" });
					return;
				}

				session.send(cmd);

				const entry = await session.waitFor((item) => {
					const t = item.msg?.t;
					if (t === "err") return true;
					if (t === "status") return true;
					if (t === "ack") {
						const ackCmd = typeof item.msg?.cmd === "string" ? item.msg.cmd : "";
						return ackCmd === cmd;
					}
					return false;
				}, timeoutMs);

				if (!entry) {
					sendJson(res, 504, { ok: false, error: "command timeout", cmd });
					return;
				}

				const payload = normalizeEntry(entry);
				const ok = payload && payload.t !== "err";
				sendJson(res, ok ? 200 : 400, payload ?? { ok: false, error: "empty response" });
				return;
			}

			sendJson(res, 404, { ok: false, error: "not found" });
		} catch (error) {
			const statusCode = Number.isInteger(error?.statusCode) ? error.statusCode : 500;
			const message = error instanceof Error ? error.message : "unknown error";
			sendJson(res, statusCode, { ok: false, error: message });
		}
	});

	server.timeout = SERVER_TIMEOUT_MS;
	server.headersTimeout = HEADERS_TIMEOUT_MS;
	return server;
}

async function main() {
	const options = parseBridgeArgs(process.argv);
	let server = null;
	let session = null;
	try {
		const serial = await openSerial(options.port, options.baud);
		session = new BoardSession(serial, options.timeoutMs);
		server = createBridgeServer({ session, serial, options });

		server.listen(options.listen, options.host, () => {
			console.log(`[bridge] listening on http://${options.host}:${options.listen}`);
			console.log(`[bridge] serial ${options.port} @ ${options.baud}`);
			console.log(
				options.token ? "[bridge] token auth enabled" : "[bridge] WARNING: no token set",
			);
		});
	} catch (error) {
		await closeBridgeServer(server);
		throw error;
	}

	const shutdown = () => {
		try {
			server.close();
		} catch {
			// ignore
		}
		try {
			session.close();
		} catch {
			// ignore
		}
		process.exit(0);
	};

	process.on("SIGINT", shutdown);
	process.on("SIGTERM", shutdown);
}

const invokedDirectly = process.argv[1] && pathToFileURL(process.argv[1]).href === import.meta.url;
if (invokedDirectly) {
	main().catch((error) => {
		const message = error instanceof Error ? error.message : String(error);
		console.error(`[bridge] failed: ${message}`);
		process.exit(1);
	});
}
