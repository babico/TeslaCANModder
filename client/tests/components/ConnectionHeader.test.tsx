import React from "react";
import { render } from "@testing-library/react-native";

const mockBoardConnState = {
	selectedTransportType: "http",
	isSelectedTransportReady: false,
	connectionBusy: false,
	baseUrl: "http://192.168.4.1",
	commandPath: "/api/command",
	statusPath: "/api/status",
	lastResult: "idle",
	setSelectedTransportType: jest.fn(),
	setBaseUrl: jest.fn(),
	setCommandPath: jest.fn(),
	setStatusPath: jest.fn(),
	applyConnection: jest.fn(async () => undefined),
	applyPreset: jest.fn(),
};

jest.mock("../../src/ui/shadcn/sheet", () => ({
	Sheet: ({ children, open }: any) => (open ? children : null),
}));

jest.mock("lucide-react-native", () => ({
	Sun: "SunIcon",
	Moon: "MoonIcon",
	Monitor: "MonitorIcon",
}));

jest.mock("../../src/state/BoardConnectionContext", () => ({
	CONNECTION_PRESETS: [
		{
			name: "Vehicle AP",
			connection: {
				baseUrl: "http://192.168.4.1",
				commandPath: "/api/command",
				statusPath: "/api/status",
			},
		},
	],
	useBoardConnection: () => mockBoardConnState,
}));

import { ConnectionHeader } from "../../src/components/ConnectionHeader";

describe("ConnectionHeader (smoke)", () => {
	it("renders title with board context", () => {
		const { getByText } = render(React.createElement(ConnectionHeader));
		expect(getByText(/Tesla CAN Modder/)).toBeTruthy();
	});
});
