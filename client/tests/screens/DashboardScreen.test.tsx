import React from "react";
import { render } from "@testing-library/react-native";

jest.mock("../../src/screens/DriveScreen", () => ({
	DriveScreen: () => null,
}));

jest.mock("../../src/components/TelemetryPanel", () => ({
	TelemetryPanel: () => null,
}));

jest.mock("../../src/components/IntegrationPanel", () => ({
	IntegrationPanel: () => null,
}));

jest.mock("../../src/components/UtilityPanel", () => ({
	UtilityPanel: () => null,
}));

import { DashboardScreen } from "../../src/screens/DashboardScreen";

describe("DashboardScreen", () => {
it("renders overview feature description copy", () => {
const boardState = {
chassisOnline: true,
vehicleOnline: false,
bodyOnline: false,
vehicleSpeed: 48,
} as any;
	const { getByText, getAllByText } = render(<DashboardScreen boardState={boardState} />);
	expect(getByText(/Telemetry shows vehicle health/)).toBeTruthy();
	expect(getAllByText(/Dashboard/).length).toBeGreaterThan(0);
});
});
