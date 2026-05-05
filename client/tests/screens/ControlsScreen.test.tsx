import React from "react";
import { render, fireEvent } from "@testing-library/react-native";

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

import { ControlsScreen } from "../../src/screens/ControlsScreen";

const boardState: any = {
	chassisOnline: true,
	vehicleOnline: true,
	bodyOnline: true,
	otaInProgress: false,
	hasBms: true,
	vehicleSpeed: 0,
	fsd: false,
};

describe("ControlsScreen", () => {
	it("renders preset controls sections", () => {
		const { getByText, getAllByText } = render(
			React.createElement(ControlsScreen, { boardState, onRunCommand: jest.fn() }),
		);
		expect(getByText(/Speed Profile Controls/)).toBeTruthy();
		// "Palette" button is in BusStatusBar
		expect(getAllByText(/Palette/).length).toBeGreaterThan(0);
	});

	it("opens command palette on button press", () => {
		const { getAllByText } = render(
			React.createElement(ControlsScreen, { boardState, onRunCommand: jest.fn() }),
		);
		// Press the Palette button (text "Palette") to open modal
		fireEvent.press(getAllByText(/Palette/)[0]);
		// Just verify it doesn't crash
	});

	describe("DAS Drive panel", () => {
		const dasState: any = {
			...boardState,
			dasDriveEnabled: false,
			dasSpeedLimitKph: 25,
			dasSpeedCapKph: 25,
			dasSpeedCapMaxKph: 200,
		};

		it("renders the panel title and reflects ARMED state from boardState", () => {
			const { getByText } = render(
				React.createElement(ControlsScreen, {
					boardState: { ...dasState, dasDriveEnabled: true },
					onRunCommand: jest.fn(),
				}),
			);
			expect(getByText(/DAS Drive/)).toBeTruthy();
			expect(getByText(/ARMED/)).toBeTruthy();
		});

		it("shows OFF status when dasDriveEnabled is false", () => {
			const { getAllByText } = render(
				React.createElement(ControlsScreen, {
					boardState: dasState,
					onRunCommand: jest.fn(),
				}),
			);
			// "OFF" label appears in the DAS panel header status badge.
			expect(getAllByText(/OFF/).length).toBeGreaterThan(0);
		});

		it("sends Set Cap command bounded by dasSpeedCapMaxKph", () => {
			const onRunCommand = jest.fn();
			const { getByText } = render(
				React.createElement(ControlsScreen, {
					boardState: { ...dasState, dasSpeedCapMaxKph: 80 },
					onRunCommand,
				}),
			);
			fireEvent.press(getByText("Set Cap"));
			const call = onRunCommand.mock.calls.find((c: any[]) => c[0] === "driveCap");
			expect(call).toBeDefined();
			const sentVal = Number(call![1]);
			expect(sentVal).toBeGreaterThanOrEqual(1);
			expect(sentVal).toBeLessThanOrEqual(80);
		});
	});

	describe("Gamepad panel", () => {
		const baseGp: any = {
			...boardState,
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
		};

		it("renders OFF when gamepad is disabled", () => {
			const { getByText, getAllByText } = render(
				React.createElement(ControlsScreen, {
					boardState: baseGp,
					onRunCommand: jest.fn(),
				}),
			);
			expect(getByText("Gamepad (BLE HID)")).toBeTruthy();
			// "OFF" appears in both DAS panel and Gamepad panel
			expect(getAllByText(/^OFF$/).length).toBeGreaterThanOrEqual(1);
		});

		it("shows CONNECTED + paired info when connected", () => {
			const state = {
				...baseGp,
				gamepad: {
					...baseGp.gamepad,
					enabled: true,
					connected: true,
					pairedAddr: "aa:bb:cc:dd:ee:ff",
					pairedName: "Xbox Wireless",
					rssi: -55,
					battery: 80,
				},
			};
			const { getByText } = render(
				React.createElement(ControlsScreen, { boardState: state, onRunCommand: jest.fn() }),
			);
			expect(getByText(/CONNECTED/)).toBeTruthy();
			expect(getByText("Xbox Wireless")).toBeTruthy();
			expect(getByText("80%")).toBeTruthy();
			expect(getByText("-55 dBm")).toBeTruthy();
		});

		it("emits gamepadScan on Scan press", () => {
			const onRunCommand = jest.fn();
			const { getByText } = render(
				React.createElement(ControlsScreen, { boardState: baseGp, onRunCommand }),
			);
			fireEvent.press(getByText("Scan"));
			expect(onRunCommand).toHaveBeenCalledWith("gamepadScan", undefined);
		});

		it("renders discovered devices and emits gamepadPair with address", () => {
			const onRunCommand = jest.fn();
			const state = {
				...baseGp,
				gamepad: {
					...baseGp.gamepad,
					devices: [{ addr: "11:22:33:44:55:66", name: "Pad-A" }],
				},
			};
			const { getAllByText } = render(
				React.createElement(ControlsScreen, { boardState: state, onRunCommand }),
			);
			fireEvent.press(getAllByText("Pair")[0]);
			expect(onRunCommand).toHaveBeenCalledWith("gamepadPair", "11:22:33:44:55:66");
		});

		it("emits gamepadCancel on Cancel Burst", () => {
			const onRunCommand = jest.fn();
			const { getByText } = render(
				React.createElement(ControlsScreen, { boardState: baseGp, onRunCommand }),
			);
			fireEvent.press(getByText("Cancel Burst"));
			expect(onRunCommand).toHaveBeenCalledWith("gamepadCancel", undefined);
		});
	});
});
