import React from "react";
import TestRenderer from "react-test-renderer";

(globalThis as { IS_REACT_ACT_ENVIRONMENT?: boolean }).IS_REACT_ACT_ENVIRONMENT = true;

jest.mock("react-native", () => {
	const View = (props: any) => React.createElement("View", props, props.children);
	const Text = (props: any) => React.createElement("Text", props, props.children);
	const TextInput = (props: any) => React.createElement("TextInput", props, props.children);
	return {
		View,
		Text,
		TextInput,
		StyleSheet: { create: <T extends Record<string, unknown>>(o: T) => o, absoluteFillObject: {} },
	};
});

import { Input } from "../../src/ui/Input";

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
		.map((n) => n.props.children)
		.flat()
		.join(" ");
}

describe("Input", () => {
	it("renders label and hint when no error", () => {
		const r = render(<Input label="Name" hint="helpful hint" />);
		const t = texts(r);
		expect(t).toContain("Name");
		expect(t).toContain("helpful hint");
	});

	it("renders error and hides hint when error provided", () => {
		const r = render(<Input label="Name" hint="hint" error="required" />);
		const t = texts(r);
		expect(t).toContain("required");
		expect(t).not.toContain("hint");
	});

	it("forwards onChangeText", () => {
		const onChangeText = jest.fn();
		const r = render(<Input onChangeText={onChangeText} />);
		const input = r.root.findAll((n) => (n.type as any) === "TextInput")[0];
		TestRenderer.act(() => input.props.onChangeText("hello"));
		expect(onChangeText).toHaveBeenCalledWith("hello");
	});

	it("renders dark variant", () => {
		const r = render(<Input label="L" variant="dark" />);
		expect(texts(r)).toContain("L");
	});
});
