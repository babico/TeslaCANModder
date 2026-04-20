import React from "react";
import TestRenderer from "react-test-renderer";

(globalThis as { IS_REACT_ACT_ENVIRONMENT?: boolean }).IS_REACT_ACT_ENVIRONMENT = true;

jest.mock("react-native", () => {
	const View = (props: any) => React.createElement("View", props, props.children);
	const Text = (props: any) => React.createElement("Text", props, props.children);
	const Pressable = (props: any) => React.createElement("Pressable", props, props.children);
	return {
		View,
		Text,
		Pressable,
		StyleSheet: { create: <T extends Record<string, unknown>>(o: T) => o, absoluteFillObject: {} },
	};
});

import { MonitorSidebarNavigation, MonitorBottomBar } from "../../../src/components/monitor/MonitorNavigation";

function render(node: React.ReactElement) {
	let r!: TestRenderer.ReactTestRenderer;
	TestRenderer.act(() => {
		r = TestRenderer.create(node);
	});
	return r;
}

const items: any = [
	{ tab: "console", label: "Console", icon: "►" },
	{ tab: "connection", label: "Connection", icon: "⚡" },
	{ tab: "diagnostics", label: "Diagnostics", icon: "◉" },
];

describe("MonitorSidebarNavigation", () => {
	it("renders all nav items, frame count and CAN status", () => {
		const r = render(
			<MonitorSidebarNavigation
				items={items}
				activeSection="console"
				onChangeSection={jest.fn()}
				frameCount={42}
				canConnected={true}
			/>,
		);
		const t = r.root
			.findAll((n) => (n.type as any) === "Text")
			.map((n) => String(n.props.children).replace(/,/g, " "))
			.join(" ");
		expect(t).toContain("Console");
		expect(t).toContain("Connection");
		expect(t).toContain("42");
		expect(t).toContain("CAN OK");
	});

	it("calls onChangeSection on item press", () => {
		const onChange = jest.fn();
		const r = render(
			<MonitorSidebarNavigation
				items={items}
				activeSection="console"
				onChangeSection={onChange}
				frameCount={0}
				canConnected={false}
			/>,
		);
		const pressables = r.root.findAll((n) => (n.type as any) === "Pressable");
		TestRenderer.act(() => pressables[1].props.onPress());
		expect(onChange).toHaveBeenCalledWith("connection");
	});
});

describe("MonitorBottomBar", () => {
	it("renders all items and routes presses", () => {
		const onChange = jest.fn();
		const r = render(
			<MonitorBottomBar items={items} activeSection="console" onChangeSection={onChange} />,
		);
		const pressables = r.root.findAll((n) => (n.type as any) === "Pressable");
		expect(pressables).toHaveLength(3);
		TestRenderer.act(() => pressables[2].props.onPress());
		expect(onChange).toHaveBeenCalledWith("diagnostics");
	});
});
