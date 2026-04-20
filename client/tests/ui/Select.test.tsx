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

jest.mock("../../src/ui/Sheet", () => ({
	Sheet: ({ visible, children, title }: any) =>
		visible
			? React.createElement("Sheet", { title }, children)
			: null,
}));

import { Select } from "../../src/ui/Select";

function render(node: React.ReactElement) {
	let r!: TestRenderer.ReactTestRenderer;
	TestRenderer.act(() => {
		r = TestRenderer.create(node);
	});
	return r;
}

function texts(r: TestRenderer.ReactTestRenderer): string {
	return r.root
		.findAll((n) => (n.type as any) === "Text")
		.map((n) => String(n.props.children))
		.join(" ");
}

describe("Select", () => {
	const options = [
		{ label: "One", value: "1" },
		{ label: "Two", value: "2" },
	];

	it("shows placeholder when no value selected", () => {
		const r = render(<Select options={options} placeholder="Pick…" onChange={jest.fn()} />);
		expect(texts(r)).toContain("Pick");
	});

	it("renders selected label when value provided", () => {
		const r = render(<Select options={options} value="2" onChange={jest.fn()} />);
		expect(texts(r)).toContain("Two");
	});

	it("opens sheet on press and fires onChange when option selected", () => {
		const onChange = jest.fn();
		const r = render(<Select options={options} onChange={onChange} />);
		const trigger = r.root.findAll((n) => (n.type as any) === "Pressable")[0];
		TestRenderer.act(() => trigger.props.onPress());
		const optionPressables = r.root.findAll((n) => (n.type as any) === "Pressable");
		TestRenderer.act(() => optionPressables[1].props.onPress());
		expect(onChange).toHaveBeenCalledWith("1");
	});

	it("does not open when disabled", () => {
		const onChange = jest.fn();
		const r = render(<Select options={options} onChange={onChange} disabled />);
		const trigger = r.root.findAll((n) => (n.type as any) === "Pressable")[0];
		TestRenderer.act(() => trigger.props.onPress());
		expect(r.root.findAll((n) => (n.type as any) === "Sheet")).toHaveLength(0);
	});

	it("renders error in place of hint", () => {
		const r = render(<Select options={options} hint="hint" error="bad" onChange={jest.fn()} />);
		const t = texts(r);
		expect(t).toContain("bad");
		expect(t).not.toContain("hint");
	});
});
