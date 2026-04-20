import React from "react";
import TestRenderer from "react-test-renderer";

(globalThis as { IS_REACT_ACT_ENVIRONMENT?: boolean }).IS_REACT_ACT_ENVIRONMENT = true;

jest.mock("react-native", () => {
	const View = (props: any) => React.createElement("View", props, props.children);
	const Text = (props: any) => React.createElement("Text", props, props.children);
	return {
		View,
		Text,
		StyleSheet: {
			create: <T extends Record<string, unknown>>(obj: T) => obj,
			absoluteFillObject: {},
		},
	};
});

import { Alert } from "../../src/ui/Alert";

function renderAlert(props: React.ComponentProps<typeof Alert>) {
	let renderer!: TestRenderer.ReactTestRenderer;
	TestRenderer.act(() => {
		renderer = TestRenderer.create(<Alert {...props} />);
	});
	return renderer;
}

function texts(renderer: TestRenderer.ReactTestRenderer): string {
	return renderer.root
		.findAll((node) => (node.type as any) === "Text")
		.map((node) => node.props.children)
		.flat()
		.join(" ");
}

describe("Alert", () => {
	it("renders description without title by default", () => {
		const r = renderAlert({ description: "hello world" });
		expect(texts(r)).toContain("hello world");
	});

	it("renders title when provided", () => {
		const r = renderAlert({ title: "Heads up", description: "check this" });
		const t = texts(r);
		expect(t).toContain("Heads up");
		expect(t).toContain("check this");
	});

	it.each(["default", "success", "warning", "destructive"] as const)(
		"renders %s variant",
		(variant) => {
			const r = renderAlert({ description: "msg", variant });
			expect(texts(r)).toContain("msg");
		},
	);
});
