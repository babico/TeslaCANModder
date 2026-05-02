jest.mock("@teslacanmodder/protocol", () => ({
	commands: {
		status: () => "status",
	},
}));

import { DEFAULT_CONNECTION, HardwareController } from "../../src/hardware/controller";

describe("HardwareController transport helpers", () => {
	it("starts with HTTP transport by default", () => {
		const controller = new HardwareController();

		expect(controller.activeTransportType).toBe("http");
	});

	it("switches to REST API transport explicitly", () => {
		const controller = new HardwareController();

		controller.connectViaRestApi(DEFAULT_CONNECTION);

		expect(controller.activeTransportType).toBe("http");
	});

	it("switches to BLE transport", () => {
		const controller = new HardwareController();

		controller.connectViaBle({
			isConnected: true,
			writeAndRead: jest.fn(async () => "ok"),
			readStatus: jest.fn(async () => "{}"),
		});

		expect(controller.activeTransportType).toBe("ble");
	});

	it("switches to serial COM transport", () => {
		const controller = new HardwareController();

		controller.connectViaComPort({
			isOpen: true,
			writeLine: jest.fn(async () => "ok"),
		});

		expect(controller.activeTransportType).toBe("serial");
	});

	it("switches to Bluetooth COM transport", () => {
		const controller = new HardwareController();

		controller.connectViaBluetoothComPort({
			isOpen: true,
			writeLine: jest.fn(async () => "ok"),
		});

		expect(controller.activeTransportType).toBe("bluetooth-serial");
	});
});
