import React from "react";
import { render, fireEvent } from "@testing-library/react-native";
import { TestBoardStateContext } from "../../src/test/TestBoardStateContext";

jest.mock("../../src/state/commandGating", () => ({
	getCommandGate: () => ({ available: true, reason: null }),
}));

jest.mock("../../src/hardware/controller", () => ({
	ALL_COMMANDS: [
		{ name: "lock", argCount: 0 },
		{ name: "unlock", argCount: 0 },
		{ name: "horn", argCount: 0 },
		{ name: "powerOff", argCount: 0 },
		{ name: "drive", argCount: 1 },
		{ name: "driveSpeed", argCount: 1 },
		{ name: "driveCap", argCount: 1 },
		{ name: "gamepad", argCount: 1 },
		{ name: "gamepadScan", argCount: 0 },
		{ name: "gamepadCancel", argCount: 0 },
		{ name: "gamepadUnpair", argCount: 0 },
		{ name: "gamepadPair", argCount: 1 },
	],
}));

const mockRunCommand = jest.fn(async () => undefined);

jest.mock("../../src/state/BoardStateContext", () => ({
	useBoardInstanceState: () => {
		const React = jest.requireActual("react");
		const actual = jest.requireActual("../../src/test/TestBoardStateContext");
		return React.useContext(actual.TestBoardStateContext);
	},
}));

jest.mock("../../src/state/CommandContext", () => ({
	useCommandActions: () => ({
		runCommand: mockRunCommand,
	}),
	useCommandState: () => ({
		paletteOpen: false,
	}),
}));

jest.mock("../../src/state/BoardConnectionContext", () => ({
	useBoardConnection: () => ({
		statusText: "connected",
		sendCommand: jest.fn(async () => "ok"),
	}),
}));

import { ControlsScreen } from "../../src/screens/ControlsScreen";

const defaultBoardState = {
	chassisOnline: true,
	vehicleOnline: true,
	bodyOnline: true,
	hasBms: true,
	vehicleSpeed: 0,
	fsd: false,
};

function renderWithState(boardStateOverrides: Record<string, unknown> = {}) {
	const boardState = { ...defaultBoardState, ...boardStateOverrides };
	return render(
		<TestBoardStateContext.Provider value={{ boardState }}>
			<ControlsScreen />
		</TestBoardStateContext.Provider>,
	);
}

beforeEach(() => {
	mockRunCommand.mockClear();
});

describe("ControlsScreen", () => {
	it("renders preset controls sections", () => {
		const { getByText, getAllByText } = renderWithState();
		expect(getByText(/Controls Cockpit/)).toBeTruthy();
		expect(getAllByText(/Palette/).length).toBeGreaterThan(0);
	});

	it("opens command palette on button press", () => {
		const { getAllByText } = renderWithState();
		fireEvent.press(getAllByText(/Palette/)[0]);
	});

	describe("DAS Drive panel", () => {
		it("renders the panel title and reflects ARMED state from boardState", () => {
			const { getByText } = renderWithState({ dasDriveEnabled: true });
			expect(getByText(/DAS Drive/)).toBeTruthy();
			expect(getByText(/ARMED/)).toBeTruthy();
		});

		it("shows OFF status when dasDriveEnabled is false", () => {
			const { getAllByText } = renderWithState({ dasDriveEnabled: false });
			expect(getAllByText(/OFF/).length).toBeGreaterThan(0);
		});

		it("sends Set Cap command bounded by dasSpeedCapMaxKph", () => {
			const { getByText } = renderWithState({ dasSpeedCapMaxKph: 80 });
			fireEvent.press(getByText("Set Cap"));
			const call = mockRunCommand.mock.calls.find((c: unknown[]) => c[0] === "driveCap") as
				| [string, unknown]
				| undefined;
			expect(call).toBeDefined();
			const sentVal = Number(call![1]);
			expect(sentVal).toBeGreaterThanOrEqual(1);
			expect(sentVal).toBeLessThanOrEqual(80);
		});
	});

	describe("Gamepad panel", () => {
		it("renders OFF when gamepad is disabled", () => {
			const { getByText, getAllByText } = renderWithState({
				gamepad: {
					enabled: false,
					connected: false,
					scanning: false,
					pairedAddr: "",
					pairedName: "",
					rssi: 0,
					battery: 0xff,
					devices: [],
				},
			});
			expect(getByText("Gamepad (BLE HID)")).toBeTruthy();
			expect(getAllByText(/^OFF$/).length).toBeGreaterThanOrEqual(1);
		});

		it("shows CONNECTED + paired info when connected", () => {
			const { getByText, getAllByText } = renderWithState({
				gamepad: {
					enabled: true,
					connected: true,
					pairedAddr: "aa:bb:cc:dd:ee:ff",
					pairedName: "Xbox Wireless",
					rssi: -55,
					battery: 80,
					devices: [],
				},
			});
			expect(getByText(/CONNECTED/)).toBeTruthy();
			expect(getByText("Xbox Wireless")).toBeTruthy();
			expect(getAllByText("80%").length).toBeGreaterThan(0);
			expect(getByText("-55 dBm")).toBeTruthy();
		});

		it("emits gamepadScan on Scan press", () => {
			const { getByText } = renderWithState({
				gamepad: {
					enabled: false,
					connected: false,
					scanning: false,
					pairedAddr: "",
					pairedName: "",
					rssi: 0,
					battery: 0xff,
					devices: [],
				},
			});
			fireEvent.press(getByText("Scan"));
			expect(mockRunCommand).toHaveBeenCalledWith("gamepadScan", undefined);
		});

		it("renders discovered devices and emits gamepadPair with address", () => {
			const { getAllByText } = renderWithState({
				gamepad: {
					enabled: false,
					connected: false,
					scanning: false,
					pairedAddr: "",
					pairedName: "",
					rssi: 0,
					battery: 0xff,
					devices: [{ addr: "11:22:33:44:55:66", name: "Pad-A" }],
				},
			});
			fireEvent.press(getAllByText("Pair")[0]);
			expect(mockRunCommand).toHaveBeenCalledWith("gamepadPair", "11:22:33:44:55:66");
		});

		it("emits gamepadCancel on Cancel Burst", () => {
			const { getByText } = renderWithState({
				gamepad: {
					enabled: false,
					connected: false,
					scanning: false,
					pairedAddr: "",
					pairedName: "",
					rssi: 0,
					battery: 0xff,
					devices: [],
				},
			});
			fireEvent.press(getByText("Cancel Burst"));
			expect(mockRunCommand).toHaveBeenCalledWith("gamepadCancel", undefined);
		});
	});
});
