// Integration test: HardwareController -> Transport -> mock board response
// -> CommandExecutionResult parsing -> board state extraction.
// Verifies the full command pipeline from user invocation to parsed result.

jest.mock("@teslacanmodder/protocol", () => ({
	initialBoardState: { frames: [], messages: [] },
	commands: {
		status: () => "status",
		lock: () => "lock",
		unlock: () => "unlock",
		horn: () => "horn",
	},
}));

import { HardwareController } from "../../src/hardware/controller";

describe("command flow integration", () => {
	function createMockBlePeripheral(response: string) {
		const writeAndRead = jest.fn(async () => response);
		const readStatus = jest.fn(async () => response);
		return {
			peripheral: { isConnected: true, writeAndRead, readStatus },
			writeAndRead,
			readStatus,
		};
	}

	it("dispatches a known command and returns ok with response text", async () => {
		const controller = new HardwareController();
		const { peripheral, writeAndRead } = createMockBlePeripheral("OK");
		controller.connectViaBle(peripheral);

		const result = await controller.runCommand("lock");

		expect(writeAndRead).toHaveBeenCalledWith("lock");
		expect(result.ok).toBe(true);
		if (result.ok) {
			expect(result.command).toBe("lock");
			expect(result.responseText).toBe("OK");
		}
	});

	it("returns ok=false for an unknown command without touching the transport", async () => {
		const controller = new HardwareController();
		const { peripheral, writeAndRead } = createMockBlePeripheral("OK");
		controller.connectViaBle(peripheral);

		const result = await controller.runCommand("notARealCommand" as never);

		expect(writeAndRead).not.toHaveBeenCalled();
		expect(result.ok).toBe(false);
		if (!result.ok) {
			expect(result.error).toMatch(/unknown command/);
		}
	});

	it("propagates transport failures as ok=false with error message", async () => {
		const controller = new HardwareController();
		const peripheral = {
			isConnected: true,
			writeAndRead: jest.fn(async () => {
				throw new Error("transport boom");
			}),
			readStatus: jest.fn(async () => "{}"),
		};
		controller.connectViaBle(peripheral);

		const result = await controller.runCommand("lock");

		expect(result.ok).toBe(false);
		if (!result.ok) {
			expect(result.error).toBe("transport boom");
		}
	});

	it("extracts board state from a JSON status response", async () => {
		const controller = new HardwareController();
		const json = JSON.stringify({ chassisOnline: true, bmsSoc: 0.5 });
		const peripheral = {
			isConnected: true,
			writeAndRead: jest.fn(async () => json),
			readStatus: jest.fn(async () => json),
		};
		controller.connectViaBle(peripheral);

		const status = await controller.readStatus();

		expect(status.raw).toEqual(JSON.parse(json));
		expect(status.boardState).toBeDefined();
	});
});
