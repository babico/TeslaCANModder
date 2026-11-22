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
	 * Listener for routes not owned by the skeleton. A3 mounts its
	 * authenticated command bridge here.
	 */
	fallback?: RequestListener;
}

/** Resolve `CAN_TRANSPORT`; anything other than `vcan` falls back to `socketcan`. */
export function getCanTransport(env: NodeJS.ProcessEnv = process.env): CanTransport {
	return env.CAN_TRANSPORT === "vcan" ? "vcan" : "socketcan";
}

/** Minimal health endpoint. A3 mounts the authenticated bridge via `options.fallback`. */
export function createPiServer(options: PiServerOptions = {}): Server {
	const transport = options.transport ?? getCanTransport();
	const fallback = options.fallback;

	return createServer((request: IncomingMessage, response: ServerResponse) => {
		if (request.method === "GET" && request.url === "/health") {
			response.writeHead(200, { "content-type": "application/json" });
			response.end(JSON.stringify({ ok: true, transport }));
			return;
		}

		if (fallback) {
			fallback(request, response);
			return;
		}

		response.writeHead(404, { "content-type": "application/json" });
		response.end(JSON.stringify({ error: "not found" }));
	});
}
