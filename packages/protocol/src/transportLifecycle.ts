export type TransportKind = "ble" | "serial" | "http";

export interface TransportCapabilities {
	ble?: boolean;
	serial?: boolean;
	http?: boolean;
}

export interface TransportSelection {
	transport: TransportKind | null;
	usedFallback: boolean;
	reason: string | null;
}

export interface TransportLifecycleState {
	connected: boolean;
	activeTransport: TransportKind | null;
	pendingCommandIds: string[];
}

export type TransportLifecycleEvent =
	| { type: "connect-success"; transport: TransportKind }
	| { type: "disconnect" }
	| { type: "command-enqueued"; id: string }
	| { type: "command-acked"; id: string }
	| { type: "command-failed"; id: string };

export const initialTransportLifecycleState: TransportLifecycleState = {
	connected: false,
	activeTransport: null,
	pendingCommandIds: [],
};

function isAvailable(kind: TransportKind, capabilities: TransportCapabilities): boolean {
	return Boolean(capabilities[kind]);
}

export function resolveTransportSelection(
	preferred: TransportKind,
	capabilities: TransportCapabilities,
	fallbackOrder: TransportKind[] = ["serial", "ble", "http"],
): TransportSelection {
	if (isAvailable(preferred, capabilities)) {
		return {
			transport: preferred,
			usedFallback: false,
			reason: null,
		};
	}

	for (const candidate of fallbackOrder) {
		if (candidate === preferred) continue;
		if (!isAvailable(candidate, capabilities)) continue;
		return {
			transport: candidate,
			usedFallback: true,
			reason: `${preferred} unavailable; fallback to ${candidate}`,
		};
	}

	return {
		transport: null,
		usedFallback: false,
		reason: `no available transport for preferred ${preferred}`,
	};
}

export function reduceTransportLifecycle(
	prev: TransportLifecycleState,
	event: TransportLifecycleEvent,
): TransportLifecycleState {
	if (event.type === "connect-success") {
		return {
			...prev,
			connected: true,
			activeTransport: event.transport,
		};
	}

	if (event.type === "disconnect") {
		// Disconnection invalidates in-flight command lifecycle tracking.
		return {
			...prev,
			connected: false,
			activeTransport: null,
			pendingCommandIds: [],
		};
	}

	if (event.type === "command-enqueued") {
		if (prev.pendingCommandIds.includes(event.id)) {
			return prev;
		}
		return {
			...prev,
			pendingCommandIds: [event.id, ...prev.pendingCommandIds],
		};
	}

	if (event.type === "command-acked" || event.type === "command-failed") {
		return {
			...prev,
			pendingCommandIds: prev.pendingCommandIds.filter((id) => id !== event.id),
		};
	}

	return prev;
}
