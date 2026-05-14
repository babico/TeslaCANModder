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

jest.mock("../../src/state/BoardStateContext", () => {
	const actual = jest.requireActual("../../src/state/BoardStateContext");
	return {
		...actual,
		useBoardInstanceState: () => ({
			boardState: {
				chassisOnline: true,
				vehicleOnline: false,
				bodyOnline: false,
				vehicleSpeed: 48,
			},
			statusText: "",
			lastResult: "",
		}),
	};
});

import { DashboardScreen } from "../../src/screens/DashboardScreen";

describe("DashboardScreen", () => {
	it("renders overview feature description copy", () => {
		const { getByText, getAllByText } = render(<DashboardScreen />);
		expect(getByText(/Telemetry shows vehicle health/)).toBeTruthy();
		expect(getAllByText(/Dashboard/).length).toBeGreaterThan(0);
	});
});
