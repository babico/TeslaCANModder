#!/usr/bin/env node
/**
 * Serial HTTP Bridge
 *
 * Exposes ESP32 serial commands as REST endpoints for Expo Go clients.
 *
 * Usage:
 *   node tools/serial-http-bridge.js --port COM4 --baud 115200 --listen 8080
 *
 * Endpoints:
 *   GET  /health
 *   GET  /api/status
 *   POST /api/command { "cmd": "status" }
 */

import http from "node:http";
import { openSerial, BoardSession } from "./lib/session.js";

function parseArgs(argv) {
	const out = {
		port: "",
		baud: 115200,
		listen: 8080,
		host: "0.0.0.0",
		timeoutMs: 3000,
	};

	for (let i = 2; i < argv.length; i += 1) {
		const token = argv[i];
		const next = argv[i + 1];

		if (token === "--port" && next) {
			out.port = next;
			i += 1;
			continue;
		}
		if (token === "--baud" && next) {
			out.baud = Number(next) || out.baud;
			i += 1;
			continue;
		}
		if (token === "--listen" && next) {
			out.listen = Number(next) || out.listen;
			i += 1;
			continue;
		}
		if (token === "--host" && next) {
			out.host = next;
			i += 1;
			continue;
		}
		if (token === "--timeout" && next) {
			out.timeoutMs = Number(next) || out.timeoutMs;
			i += 1;
			continue;
		}
		if (token === "--help" || token === "-h") {
			printHelpAndExit(0);
		}
	}

	if (!out.port) {
		printHelpAndExit(1);
	}

	return out;
}

function printHelpAndExit(code) {
	console.log(
		"Usage: node tools/serial-http-bridge.js --port COM4 [--baud 115200] [--listen 8080] [--host 127.0.0.1] [--timeout 3000]",
	);
	process.exit(code);
}

function json(res, status, body) {
	const payload = JSON.stringify(body);
	res.writeHead(status, {
		"Content-Type": "application/json; charset=utf-8",
		"Cache-Control": "no-store",
		"Access-Control-Allow-Origin": "*",
		"Access-Control-Allow-Headers": "Content-Type",
		"Access-Control-Allow-Methods": "GET,POST,OPTIONS",
	});
	res.end(payload);
}

async function readBody(req) {
	const chunks = [];
	for await (const chunk of req) {
		chunks.push(Buffer.from(chunk));
	}
	return Buffer.concat(chunks).toString("utf8");
}

function normalizeEntry(entry) {
	if (!entry) {
		return null;
	}
	if (entry.msg) {
		return entry.msg;
	}
	return { t: "text", raw: entry.raw ?? "" };
}

async function main() {
	const options = parseArgs(process.argv);
	const serial = await openSerial(options.port, options.baud);
	const session = new BoardSession(serial, options.timeoutMs);

	const server = http.createServer(async (req, res) => {
		try {
			if (req.method === "OPTIONS") {
				json(res, 200, { ok: true });
				return;
			}

			const url = req.url || "/";

			if (req.method === "GET" && url === "/health") {
				json(res, 200, {
					ok: true,
					mode: "serial-http-bridge",
					port: options.port,
					baud: options.baud,
					listen: options.listen,
					timeoutMs: options.timeoutMs,
					serialOpen: Boolean(serial.isOpen),
				});
				return;
			}

			if (req.method === "GET" && url === "/api/status") {
				session.send("status");
				const entry = await session.waitFor(
					(item) => item.msg?.t === "status" || item.msg?.t === "boot",
					options.timeoutMs,
				);
				if (!entry) {
					json(res, 504, { ok: false, error: "status timeout" });
					return;
				}
				json(res, 200, normalizeEntry(entry));
				return;
			}

			if (req.method === "POST" && url === "/api/command") {
				const raw = await readBody(req);
				let cmd = "";

				try {
					const parsed = JSON.parse(raw || "{}");
					cmd = typeof parsed.cmd === "string" ? parsed.cmd.trim() : "";
				} catch {
					json(res, 400, { ok: false, error: "invalid JSON body" });
					return;
				}

				if (!cmd) {
					json(res, 400, { ok: false, error: "missing cmd" });
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
				}, options.timeoutMs);

				if (!entry) {
					json(res, 504, { ok: false, error: "command timeout", cmd });
					return;
				}

				const payload = normalizeEntry(entry);
				const ok = payload && payload.t !== "err";
				json(res, ok ? 200 : 400, payload ?? { ok: false, error: "empty response" });
				return;
			}

			json(res, 404, { ok: false, error: "not found" });
		} catch (error) {
			const message = error instanceof Error ? error.message : "unknown error";
			json(res, 500, { ok: false, error: message });
		}
	});

	server.listen(options.listen, options.host, () => {
		console.log(`[bridge] listening on http://${options.host}:${options.listen}`);
		console.log(`[bridge] serial ${options.port} @ ${options.baud}`);
	});

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

main().catch((error) => {
	const message = error instanceof Error ? error.message : String(error);
	console.error(`[bridge] failed: ${message}`);
	process.exit(1);
});
