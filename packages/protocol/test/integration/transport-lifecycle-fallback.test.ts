import {
	initialBoardState,
	initialTransportLifecycleState,
	parseSerialLine,
	reduceBoardMessage,
	reduceTransportLifecycle,
	resolveTransportSelection,
	type BoardMessage,
	type TransportKind,
} from "../../src/index.js";

describe("Integration: transport connect/disconnect/fallback", () => {
	it.each([
		{
			name: "BLE preferred and available",
			preferred: "ble" as const,
			capabilities: { ble: true, serial: true, http: true },
			expectedTransport: "ble",
			expectedFallback: false,
		},
		{
			name: "BLE unavailable -> serial fallback",
			preferred: "ble" as const,
			capabilities: { ble: false, serial: true, http: true },
			expectedTransport: "serial",
			expectedFallback: true,
		},
		{
			name: "Serial unavailable -> HTTP fallback",
			preferred: "serial" as const,
			capabilities: { ble: false, serial: false, http: true },
			expectedTransport: "http",
			expectedFallback: true,
		},
		{
			name: "HTTP preferred and available",
			preferred: "http" as const,
			capabilities: { ble: true, serial: true, http: true },
			expectedTransport: "http",
			expectedFallback: false,
		},
	])("$name", ({ preferred, capabilities, expectedTransport, expectedFallback }) => {
		const selection = resolveTransportSelection(preferred, capabilities, [
			"serial",
			"ble",
			"http",
		]);

		expect(selection.transport).toBe(expectedTransport);
		expect(selection.usedFallback).toBe(expectedFallback);
	});

	it("selects preferred transport when available", () => {
		const selection = resolveTransportSelection("ble", {
			ble: true,
			serial: true,
			http: true,
		});

		expect(selection.transport).toBe("ble");
		expect(selection.usedFallback).toBe(false);
		expect(selection.reason).toBeNull();
	});

	it("falls back when preferred is unavailable", () => {
		const selection = resolveTransportSelection(
			"ble",
			{
				ble: false,
				serial: true,
				http: true,
			},
			["serial", "http"],
		);

		expect(selection.transport).toBe("serial");
		expect(selection.usedFallback).toBe(true);
		expect(selection.reason).toContain("fallback to serial");
	});

	it("returns null transport when no capabilities are available", () => {
		const selection = resolveTransportSelection(
			"ble",
			{
				ble: false,
				serial: false,
				http: false,
			},
			["serial", "http"],
		);

		expect(selection.transport).toBeNull();
		expect(selection.usedFallback).toBe(false);
		expect(selection.reason).toContain("no available transport");
	});

	it("clears in-flight command lifecycle on disconnect and can reconnect using fallback", () => {
		let lifecycle = initialTransportLifecycleState;

		lifecycle = reduceTransportLifecycle(lifecycle, {
			type: "connect-success",
			transport: "serial",
		});
		lifecycle = reduceTransportLifecycle(lifecycle, { type: "command-enqueued", id: "cmd-1" });
		lifecycle = reduceTransportLifecycle(lifecycle, { type: "command-enqueued", id: "cmd-2" });
		lifecycle = reduceTransportLifecycle(lifecycle, { type: "command-acked", id: "cmd-1" });

		expect(lifecycle.connected).toBe(true);
		expect(lifecycle.pendingCommandIds).toEqual(["cmd-2"]);

		lifecycle = reduceTransportLifecycle(lifecycle, { type: "disconnect" });

		expect(lifecycle.connected).toBe(false);
		expect(lifecycle.activeTransport).toBeNull();
		expect(lifecycle.pendingCommandIds).toEqual([]);

		const reconnectSelection = resolveTransportSelection(
			"serial",
			{
				ble: false,
				serial: false,
				http: true,
			},
			["http", "ble"],
		);

		expect(reconnectSelection.transport).toBe("http");
		expect(reconnectSelection.usedFallback).toBe(true);

		lifecycle = reduceTransportLifecycle(lifecycle, {
			type: "connect-success",
			transport: reconnectSelection.transport as TransportKind,
		});

		expect(lifecycle.connected).toBe(true);
		expect(lifecycle.activeTransport).toBe("http");
	});

	it("deduplicates pending IDs and clears failed command entries", () => {
		let lifecycle = initialTransportLifecycleState;

		lifecycle = reduceTransportLifecycle(lifecycle, {
			type: "connect-success",
			transport: "ble",
		});
		lifecycle = reduceTransportLifecycle(lifecycle, { type: "command-enqueued", id: "cmd-9" });
		lifecycle = reduceTransportLifecycle(lifecycle, { type: "command-enqueued", id: "cmd-9" });

		expect(lifecycle.pendingCommandIds).toEqual(["cmd-9"]);

		lifecycle = reduceTransportLifecycle(lifecycle, { type: "command-failed", id: "cmd-9" });
		expect(lifecycle.pendingCommandIds).toEqual([]);
		expect(lifecycle.connected).toBe(true);
		expect(lifecycle.activeTransport).toBe("ble");
	});

	it("reconnect lifecycle still reduces parsed status payload into board state", () => {
		const events = parseSerialLine(
			'{"t":"status","up":3200,"chassisOnline":1,"busChassis":1,"busVehicle":0,"busBody":0}',
		);
		const messageEvent = events.find((event) => event.type === "message");

		expect(messageEvent).toBeDefined();
		if (!messageEvent || !messageEvent.message) {
			throw new Error("expected parsed message event");
		}

		const updated = reduceBoardMessage(
			initialBoardState,
			messageEvent.message as unknown as BoardMessage,
			() => 1,
			"12:00:00",
		);

		expect(updated.uptime).toBe(3200);
		expect(updated.chassisOnline).toBe(true);
		expect(updated.busChassis).toBe(true);
		expect(updated.busVehicle).toBe(false);
		expect(updated.busBody).toBe(false);
	});
});
