import React from "react";
import { render, fireEvent } from "@testing-library/react-native";

const mockBoardConnState = {
	selectedTransportType: "http",
	isSelectedTransportReady: true,
	connectionBusy: false,
	baseUrl: "http://192.168.4.1",
	commandPath: "/api/command",
	statusPath: "/api/status",
	lastResult: "ready",
	setSelectedTransportType: jest.fn(),
	setBaseUrl: jest.fn(),
	setCommandPath: jest.fn(),
	setStatusPath: jest.fn(),
	applyConnection: jest.fn(async () => undefined),
	applyPreset: jest.fn(),
};

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

jest.mock("../../src/ui/Sheet", () => ({
	Sheet: ({ children, visible }: any) => (visible ? children : null),
}));

import { MenuHeader } from "../../src/components/MenuHeader";
import { ConnectionHeader } from "../../src/components/ConnectionHeader";

describe("Header UI", () => {
	it("renders menu tabs and triggers tab selection", () => {
		const onSelectTab = jest.fn();
		const { getByText } = render(
			React.createElement(MenuHeader, {
				tabs: [
					{ id: "dashboard", label: "Dashboard" },
					{ id: "console", label: "Console" },
				],
				activeTab: "dashboard",
				onSelectTab,
			}),
		);
		fireEvent.press(getByText("Console"));
		expect(onSelectTab).toHaveBeenCalledWith("console");
	});

	it("renders connection header with shared board context", () => {
		const { getByText } = render(React.createElement(ConnectionHeader));
		expect(getByText(/Tesla CAN Modder/)).toBeTruthy();
	});
});
