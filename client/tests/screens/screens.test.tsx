import React from "react";
import { render, act } from "@testing-library/react-native";

jest.mock("../../src/state/commandGating", () => ({
	getCommandGate: () => ({ available: true, reason: null }),
}));

jest.mock("../../src/hardware/controller", () => ({
	ALL_COMMANDS: [
		{ name: "lock", argCount: 0 },
		{ name: "unlock", argCount: 0 },
		{ name: "horn", argCount: 0 },
	],
}));

jest.mock("../../src/docs/catalog", () => ({
	DEFAULT_DOC_ROUTE: "getting-started",
	EMPTY_DOC_TREE: { kind: "folder", path: "", title: "Docs", order: 0, children: [] },
	loadDocsCatalog: async () => ({
		docsByRoute: {
			"getting-started": {
				routePath: "getting-started",
				path: "docs/getting-started.md",
				category: "guides",
				title: "Getting Started",
				description: "Welcome doc",
				order: 1,
				body: "# Getting Started",
			},
		},
		docTree: {
			title: "Docs",
			children: [
				{
					kind: "doc",
					path: "getting-started",
					routePath: "getting-started",
					title: "Getting Started",
					order: 1,
				},
			],
		},
	}),
}));

jest.mock("../../src/components/docs/MarkdownRenderer", () => ({
	MarkdownRenderer: () => null,
}));

const mockBoardState = {
	chassisOnline: true,
	vehicleOnline: true,
	bodyOnline: true,
	hasBms: true,
	vehicleSpeed: 0,
	frameCount: 2,
	canHealth: { 0: { on: true, det: true } },
};

jest.mock("../../src/state/BoardStateContext", () => ({
	useBoardInstanceState: () => ({
		boardState: mockBoardState,
	}),
}));

jest.mock("../../src/state/CommandContext", () => ({
	useCommandActions: () => ({
		runCommand: jest.fn(async () => undefined),
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
import { FlasherScreen } from "../../src/screens/FlasherScreen";
import { DocsScreen } from "../../src/screens/DocsScreen";

describe("Screen UI coverage", () => {
	it("renders ControlsScreen sections", () => {
		const { getByText, getAllByText } = render(React.createElement(ControlsScreen));
		expect(getByText(/Controls Cockpit/)).toBeTruthy();
		expect(getAllByText(/Palette/).length).toBeGreaterThan(0);
	});

	it("renders FlasherScreen with Chassis bus label", () => {
		const { getAllByText } = render(React.createElement(FlasherScreen));
		expect(getAllByText(/Chassis/).length).toBeGreaterThan(0);
	});

	it("renders DocsScreen and loads docs asynchronously", async () => {
		const onNavigateDoc = jest.fn();
		const { getByText } = render(
			React.createElement(DocsScreen, {
				activeDocRoute: "getting-started",
				onNavigateDoc,
			}),
		);
		expect(getByText(/Docs/)).toBeTruthy();
		await act(async () => {});
	});
});
