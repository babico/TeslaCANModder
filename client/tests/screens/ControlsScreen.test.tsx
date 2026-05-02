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
});
