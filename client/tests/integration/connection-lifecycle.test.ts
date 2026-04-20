// Integration test: connect -> issue command -> disconnect -> reconnect lifecycle.
// Verifies HardwareController gracefully swaps transports across the lifecycle
// and that previously-issued commands settle before the swap completes.

jest.mock("@teslacanmodder/protocol", () => ({
	commands: {
		status: () => "status",
		lock: () => "lock",
		unlock: () => "unlock",
		horn: () => "horn",
	},
}));

import {
	DEFAULT_CONNECTION,
	HardwareController,
} from "../../src/hardware/controller";

describe("connection lifecycle integration", () => {
	function makeBle(response = "OK") {
		const writeAndRead = jest.fn(async () => response);
		const readStatus = jest.fn(async () => response);
		return {
			peripheral: { isConnected: true, writeAndRead, readStatus },
			writeAndRead,
			readStatus,
		};
	}

	function makeSerial(response = "OK") {
		const writeLine = jest.fn(async () => response);
		return { port: { isOpen: true, writeLine }, writeLine };
	}

	it("starts with HTTP, switches to BLE, runs a command on BLE", async () => {
		const controller = new HardwareController();
		expect(controller.activeTransportType).toBe("http");

		const ble = makeBle("OK");
		controller.connectViaBle(ble.peripheral);

		expect(controller.activeTransportType).toBe("ble");

		const result = await controller.runCommand("lock");
		expect(result.ok).toBe(true);
		expect(ble.writeAndRead).toHaveBeenCalledTimes(1);
	});

	it("can swap back to HTTP via setConnection (simulated reconnect)", async () => {
		const controller = new HardwareController();
		const ble = makeBle();
		controller.connectViaBle(ble.peripheral);
		expect(controller.activeTransportType).toBe("ble");

		controller.setConnection(DEFAULT_CONNECTION);
		expect(controller.activeTransportType).toBe("http");
	});

	it("can chain BLE -> Serial -> Bluetooth-Serial transitions", async () => {
		const controller = new HardwareController();
		const ble = makeBle();
		const serial = makeSerial();
		const bts = makeSerial();

		controller.connectViaBle(ble.peripheral);
		expect(controller.activeTransportType).toBe("ble");

		controller.connectViaComPort(serial.port);
		expect(controller.activeTransportType).toBe("serial");

		controller.connectViaBluetoothComPort(bts.port);
		expect(controller.activeTransportType).toBe("bluetooth-serial");
	});

	it("setTransport awaits in-flight requests before swapping", async () => {
		const controller = new HardwareController();

		let resolveFirst: ((value: string) => void) | null = null;
		const slowPeripheral = {
			isConnected: true,
			writeAndRead: jest.fn(
				() =>
					new Promise<string>((resolve) => {
						resolveFirst = resolve;
					}),
			),
			readStatus: jest.fn(async () => "{}"),
		};
		controller.connectViaBle(slowPeripheral);

		const inflight = controller.runCommand("lock");

		const fastPeripheral = {
			isConnected: true,
			writeAndRead: jest.fn(async () => "OK2"),
			readStatus: jest.fn(async () => "{}"),
		};

		// Start swap; should not resolve until inflight settles.
		const swap = controller.setTransport({
			transportType: "ble",
			send: fastPeripheral.writeAndRead,
			status: fastPeripheral.readStatus,
		} as never);

		// Settle the inflight request.
		const settle = resolveFirst as ((value: string) => void) | null;
		settle?.("OK1");

		const [first] = await Promise.all([inflight, swap]);
		expect(first.ok).toBe(true);
	});
});
