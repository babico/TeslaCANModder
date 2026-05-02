/**
 * B-02 Transport layer unit tests
 * Covers: BleCommandTransport, SerialCommandTransport,
 *         UnsupportedTransport, capability detection helpers,
 *         and createTransport factory.
 */

import {
	BleCommandTransport,
	BluetoothSerialCommandTransport,
	SerialCommandTransport,
	HttpCommandTransport,
	UnsupportedTransport,
	createTransport,
	detectAvailableTransportType,
	supportsTransport,
	type IBluetoothSerialPort,
	type IBlePeripheral,
	type ISerialPort,
} from "../../src/hardware/transport";

// ---------------------------------------------------------------------------
// Helpers / fakes
// ---------------------------------------------------------------------------

function makeBlePeripheral(response: string = '{"ok":true}', connected = true): IBlePeripheral {
	return {
		isConnected: connected,
		writeAndRead: jest.fn().mockResolvedValue(response),
		readStatus: jest.fn().mockResolvedValue(response),
	};
}

function makeSerialPort(response: string = '{"ok":true}', open = true): ISerialPort {
	return {
		isOpen: open,
		writeLine: jest.fn().mockResolvedValue(response),
	};
}

function makeBluetoothSerialPort(
	response: string = '{"ok":true}',
	open = true,
): IBluetoothSerialPort {
	return {
		isOpen: open,
		writeLine: jest.fn().mockResolvedValue(response),
	};
}

// ---------------------------------------------------------------------------
// UnsupportedTransport
// ---------------------------------------------------------------------------

describe("UnsupportedTransport", () => {
	const transport = new UnsupportedTransport();

	it("exposes transportType = unsupported", () => {
		expect(transport.transportType).toBe("unsupported");
	});

	it("send() rejects with descriptive error", async () => {
		await expect(transport.send("anyCmd")).rejects.toThrow("No transport available");
	});

	it("status() rejects with descriptive error", async () => {
		await expect(transport.status()).rejects.toThrow("No transport available");
	});
});

// ---------------------------------------------------------------------------
// BleCommandTransport
// ---------------------------------------------------------------------------

describe("BleCommandTransport", () => {
	it("exposes transportType = ble", () => {
		const t = new BleCommandTransport(makeBlePeripheral());
		expect(t.transportType).toBe("ble");
	});

	it("send() calls writeAndRead on the peripheral and parses JSON", async () => {
		const peripheral = makeBlePeripheral('{"status":"ack"}');
		const t = new BleCommandTransport(peripheral);
		const result = await t.send("fsdOn");
		expect(peripheral.writeAndRead).toHaveBeenCalledWith("fsdOn");
		expect(result).toEqual({ status: "ack" });
	});

	it("send() returns raw string when response is not JSON", async () => {
		const peripheral = makeBlePeripheral("OK");
		const t = new BleCommandTransport(peripheral);
		const result = await t.send("fsdOn");
		expect(result).toBe("OK");
	});

	it("status() calls readStatus and parses JSON", async () => {
		const peripheral = makeBlePeripheral('{"speed":42}');
		const t = new BleCommandTransport(peripheral);
		const result = await t.status();
		expect(peripheral.readStatus).toHaveBeenCalled();
		expect(result).toEqual({ speed: 42 });
	});

	it("send() throws when peripheral is not connected", async () => {
		const peripheral = makeBlePeripheral("{}", false);
		const t = new BleCommandTransport(peripheral);
		await expect(t.send("fsdOn")).rejects.toThrow("not connected");
	});

	it("status() throws when peripheral is not connected", async () => {
		const peripheral = makeBlePeripheral("{}", false);
		const t = new BleCommandTransport(peripheral);
		await expect(t.status()).rejects.toThrow("not connected");
	});

	it("send() rejects on timeout when peripheral hangs", async () => {
		jest.useFakeTimers();
		const peripheral: IBlePeripheral = {
			isConnected: true,
			writeAndRead: jest.fn().mockReturnValue(
				new Promise(() => {
					/* never resolves */
				}),
			),
			readStatus: jest.fn().mockReturnValue(
				new Promise(() => {
					/* never resolves */
				}),
			),
		};
		const t = new BleCommandTransport(peripheral, { timeoutMs: 100 });
		const promise = t.send("fsdOn");
		jest.advanceTimersByTime(150);
		await expect(promise).rejects.toThrow("timed out");
		jest.useRealTimers();
	});
});

// ---------------------------------------------------------------------------
// SerialCommandTransport
// ---------------------------------------------------------------------------

describe("SerialCommandTransport", () => {
	it("exposes transportType = serial", () => {
		const t = new SerialCommandTransport(makeSerialPort());
		expect(t.transportType).toBe("serial");
	});

	it("send() calls writeLine with command + line ending and parses JSON", async () => {
		const port = makeSerialPort('{"speed":55}');
		const t = new SerialCommandTransport(port);
		const result = await t.send("fsdOn");
		expect(port.writeLine).toHaveBeenCalledWith("fsdOn\n");
		expect(result).toEqual({ speed: 55 });
	});

	it("send() does not double-append line ending if already present", async () => {
		const port = makeSerialPort("OK");
		const t = new SerialCommandTransport(port);
		await t.send("fsdOn\n");
		expect(port.writeLine).toHaveBeenCalledWith("fsdOn\n");
	});

	it("send() returns raw string when response is not JSON", async () => {
		const port = makeSerialPort("ACK");
		const t = new SerialCommandTransport(port);
		const result = await t.send("test");
		expect(result).toBe("ACK");
	});

	it("status() sends the configured status command", async () => {
		const port = makeSerialPort('{"v":1}');
		const t = new SerialCommandTransport(port, { statusCommand: "status" });
		await t.status();
		expect(port.writeLine).toHaveBeenCalledWith("status\n");
	});

	it("send() throws when port is not open", async () => {
		const port = makeSerialPort("{}", false);
		const t = new SerialCommandTransport(port);
		await expect(t.send("test")).rejects.toThrow("not open");
	});

	it("send() rejects on timeout when port hangs", async () => {
		jest.useFakeTimers();
		const port: ISerialPort = {
			isOpen: true,
			writeLine: jest.fn().mockReturnValue(
				new Promise(() => {
					/* never resolves */
				}),
			),
		};
		const t = new SerialCommandTransport(port, { timeoutMs: 50 });
		const promise = t.send("test");
		jest.advanceTimersByTime(100);
		await expect(promise).rejects.toThrow("timed out");
		jest.useRealTimers();
	});
});

// ---------------------------------------------------------------------------
// SocketCommandTransport
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// BluetoothSerialCommandTransport
// ---------------------------------------------------------------------------

describe("BluetoothSerialCommandTransport", () => {
	it("exposes transportType = bluetooth-serial", () => {
		const t = new BluetoothSerialCommandTransport(makeBluetoothSerialPort());
		expect(t.transportType).toBe("bluetooth-serial");
	});

	it("send() writes line and parses JSON", async () => {
		const port = makeBluetoothSerialPort('{"ok":1}');
		const t = new BluetoothSerialCommandTransport(port);
		const result = await t.send("status");
		expect(port.writeLine).toHaveBeenCalledWith("status\n");
		expect(result).toEqual({ ok: 1 });
	});

	it("status() sends configured status command", async () => {
		const port = makeBluetoothSerialPort("OK");
		const t = new BluetoothSerialCommandTransport(port, { statusCommand: "status" });
		await t.status();
		expect(port.writeLine).toHaveBeenCalledWith("status\n");
	});

	it("send() throws when bluetooth COM port is closed", async () => {
		const port = makeBluetoothSerialPort("{}", false);
		const t = new BluetoothSerialCommandTransport(port);
		await expect(t.send("status")).rejects.toThrow("not open");
	});
});

// ---------------------------------------------------------------------------
// HttpCommandTransport (smoke — full coverage is in integration tests)
// ---------------------------------------------------------------------------

describe("HttpCommandTransport", () => {
	it("exposes transportType = http", () => {
		const t = new HttpCommandTransport({
			baseUrl: "http://192.168.4.1",
			commandPath: "/api/command",
			statusPath: "/api/status",
		});
		expect(t.transportType).toBe("http");
	});
});

// ---------------------------------------------------------------------------
// detectAvailableTransportType
// ---------------------------------------------------------------------------

describe("detectAvailableTransportType", () => {
	it("returns one of known transport types", () => {
		const result = detectAvailableTransportType();
		expect(["http", "ble", "serial", "socket"]).toContain(result);
	});
});

// ---------------------------------------------------------------------------
// supportsTransport
// ---------------------------------------------------------------------------

describe("supportsTransport", () => {
	it("http is always supported", () => {
		expect(supportsTransport("http")).toBe(true);
	});

	it("unsupported always returns false", () => {
		expect(supportsTransport("unsupported")).toBe(false);
	});
});

// ---------------------------------------------------------------------------
// createTransport factory
// ---------------------------------------------------------------------------

describe("createTransport", () => {
	it("creates HttpCommandTransport for type=http", () => {
		const t = createTransport({
			type: "http",
			config: {
				baseUrl: "http://192.168.4.1",
				commandPath: "/api/command",
				statusPath: "/api/status",
			},
		});
		expect(t.transportType).toBe("http");
	});

	it("creates BleCommandTransport for type=ble", () => {
		const t = createTransport({
			type: "ble",
			peripheral: makeBlePeripheral(),
		});
		expect(t.transportType).toBe("ble");
	});

	it("creates SerialCommandTransport for type=serial", () => {
		const t = createTransport({
			type: "serial",
			port: makeSerialPort(),
		});
		expect(t.transportType).toBe("serial");
	});

	it("creates BluetoothSerialCommandTransport for type=bluetooth-serial", () => {
		const t = createTransport({
			type: "bluetooth-serial",
			port: makeBluetoothSerialPort(),
		});
		expect(t.transportType).toBe("bluetooth-serial");
	});
});
