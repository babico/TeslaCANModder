import React from "react";
import TestRenderer from "react-test-renderer";

(globalThis as { IS_REACT_ACT_ENVIRONMENT?: boolean }).IS_REACT_ACT_ENVIRONMENT = true;

jest.mock("react-native", () => {
	const View = (props: any) => React.createElement("View", props, props.children);
	const Text = (props: any) => React.createElement("Text", props, props.children);
	const Pressable = (props: any) => React.createElement("Pressable", props, props.children);
	const ScrollView = (props: any) => React.createElement("ScrollView", props, props.children);
	return {
		View,
		Text,
		Pressable,
		ScrollView,
		StyleSheet: { create: <T extends Record<string, unknown>>(o: T) => o, absoluteFillObject: {} },
	};
});

import { Tabs } from "../../src/ui/Tabs";

function render(node: React.ReactElement) {
	let r!: TestRenderer.ReactTestRenderer;
	TestRenderer.act(() => {
		r = TestRenderer.create(node);
	});
	return r;
}

describe("Tabs", () => {
	const items = [
		{ key: "a", label: "Alpha" },
		{ key: "b", label: "Beta", badge: 3 },
	];

	it("renders all labels and badge", () => {
		const r = render(<Tabs items={items} activeKey="a" onSelect={jest.fn()} />);
		const t = r.root
			.findAll((n) => (n.type as any) === "Text")
			.map((n) => String(n.props.children))
			.join(" ");
		expect(t).toContain("Alpha");
		expect(t).toContain("Beta");
		expect(t).toContain("3");
	});

	it("calls onSelect for clicked tab", () => {
		const onSelect = jest.fn();
		const r = render(<Tabs items={items} activeKey="a" onSelect={onSelect} />);
		const pressables = r.root.findAll((n) => (n.type as any) === "Pressable");
		TestRenderer.act(() => pressables[1].props.onPress());
		expect(onSelect).toHaveBeenCalledWith("b");
	});

	it.each(["default", "dark", "underline"] as const)("renders %s variant", (variant) => {
		const r = render(<Tabs items={items} activeKey="a" variant={variant} onSelect={jest.fn()} />);
		expect(r.root.findAll((n) => (n.type as any) === "Pressable").length).toBe(2);
	});

	it("wraps in ScrollView when scrollable", () => {
		const r = render(<Tabs items={items} activeKey="a" scrollable onSelect={jest.fn()} />);
		expect(r.root.findAll((n) => (n.type as any) === "ScrollView").length).toBe(1);
	});
});
