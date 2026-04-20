// ConnectionHeader's full integration is exercised in tests/components/headers.test.tsx.
// This file adds a focused smoke test that the module is exportable in isolation.
import React from "react";
import TestRenderer from "react-test-renderer";

(globalThis as { IS_REACT_ACT_ENVIRONMENT?: boolean }).IS_REACT_ACT_ENVIRONMENT = true;

const boardConnState = {
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

jest.mock("react-native", () => {
	const View = (props: any) => React.createElement("View", props, props.children);
	const Text = (props: any) => React.createElement("Text", props, props.children);
	const TextInput = (props: any) => React.createElement("TextInput", props, props.children);
	const Pressable = (props: any) => React.createElement("Pressable", props, props.children);
	const ScrollView = (props: any) => React.createElement("ScrollView", props, props.children);
	return {
		View,
		Text,
		TextInput,
		Pressable,
		ScrollView,
		StyleSheet: { create: <T extends Record<string, unknown>>(o: T) => o, absoluteFillObject: {} },
	};
});

jest.mock("../../src/ui/Sheet", () => ({
	Sheet: ({ children, visible }: any) =>
		visible ? React.createElement("Sheet", null, children) : null,
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
	useBoardConnection: () => boardConnState,
}));

import { ConnectionHeader } from "../../src/components/ConnectionHeader";

describe("ConnectionHeader (smoke)", () => {
	it("renders title with board context", () => {
		let r!: TestRenderer.ReactTestRenderer;
		TestRenderer.act(() => {
			r = TestRenderer.create(React.createElement(ConnectionHeader));
		});
		const t = r.root
			.findAll((n) => (n.type as any) === "Text")
			.map((n) => String(n.props.children).replace(/,/g, " "))
			.join(" ");
		expect(t).toContain("Tesla CAN Modder");
	});
});
