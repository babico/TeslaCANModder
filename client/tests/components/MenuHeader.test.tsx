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

import { MenuHeader } from "../../src/components/MenuHeader";

function render(node: React.ReactElement) {
	let r!: TestRenderer.ReactTestRenderer;
	TestRenderer.act(() => {
		r = TestRenderer.create(node);
	});
	return r;
}

describe("MenuHeader", () => {
	const tabs: any = [
		{ id: "dashboard", label: "Dashboard" },
		{ id: "console", label: "Console" },
		{ id: "docs", label: "Docs" },
	];

	it("renders all tab labels", () => {
		const r = render(<MenuHeader tabs={tabs} activeTab="dashboard" onSelectTab={jest.fn()} />);
		const t = r.root
			.findAll((n) => (n.type as any) === "Text")
			.map((n) => String(n.props.children))
			.join(" ");
		expect(t).toContain("Dashboard");
		expect(t).toContain("Console");
		expect(t).toContain("Docs");
	});

	it("calls onSelectTab when a tab is pressed", () => {
		const onSelectTab = jest.fn();
		const r = render(<MenuHeader tabs={tabs} activeTab="dashboard" onSelectTab={onSelectTab} />);
		const pressables = r.root.findAll((n) => (n.type as any) === "Pressable");
		TestRenderer.act(() => pressables[1].props.onPress());
		expect(onSelectTab).toHaveBeenCalledWith("console");
	});
});
