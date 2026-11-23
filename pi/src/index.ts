import {
	createServer,
	type IncomingMessage,
	type RequestListener,
	type Server,
	type ServerResponse,
} from "node:http";

/** CAN backend selected by the Pi service. `vcan` is the test loopback backend. */
export type CanTransport = "vcan" | "socketcan";

export interface PiServerOptions {
	/**
	 * CAN transport reported by `/health` and consumed by the A2 reader.
	 * Defaults to the `CAN_TRANSPORT` environment variable.
	 */
	transport?: CanTransport;
	/**
	 * A3 authenticated command bridge. Mounted after `/health`; it owns
	 * `/status`, `/api/status`, and `POST /api/command`, while the public
	 * health probe stays served by this skeleton. This is the only extension
	 * seam — there is no separate generic fallback listener.
	 */
	bridge?: RequestListener;
}

/** Resolve `CAN_TRANSPORT`; anything other than `vcan` falls back to `socketcan`. */
export function getCanTransport(env: NodeJS.ProcessEnv = process.env): CanTransport {
	return env.CAN_TRANSPORT === "vcan" ? "vcan" : "socketcan";
}

/**
 * Hard feature flag for the gated CAN TX backend (`pi/src/can/tx.ts`).
 * Default is OFF: only the literal string `1` enables it, and even then the
 * backend stays inert until B3's signed RT gate artifact verifies. Consumed by
 * `createGatedTx()`; no TX path is mounted by this HTTP server itself.
 */
export function isCanTxEnabled(env: NodeJS.ProcessEnv = process.env): boolean {
	return env.PI_CAN_TX === "1";
}

/** Minimal health endpoint. A3 mounts the authenticated bridge via `options.bridge`. */
export function createPiServer(options: PiServerOptions = {}): Server {
	const transport = options.transport ?? getCanTransport();
	const { bridge } = options;

	const server = createServer((request: IncomingMessage, response: ServerResponse) => {
		if (request.method === "GET" && request.url === "/health") {
			response.writeHead(200, { "content-type": "application/json" });
			response.end(JSON.stringify({ ok: true, transport }));
			return;
		}

		if (bridge) {
			bridge(request, response);
			return;
		}

		response.writeHead(404, { "content-type": "application/json" });
		response.end(JSON.stringify({ error: "not found" }));
	});

	// Hardened defaults from the serial-HTTP bridge: slowloris / idle-socket guard.
	server.timeout = 30_000;
	server.headersTimeout = 5_000;
	return server;
}
