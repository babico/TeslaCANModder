import React from "react";
import TestRenderer from "react-test-renderer";

(globalThis as { IS_REACT_ACT_ENVIRONMENT?: boolean }).IS_REACT_ACT_ENVIRONMENT = true;

jest.mock("react-native", () => {
	const Text = (props: any) => React.createElement("Text", props, props.children);
	return {
		Text,
		StyleSheet: { create: <T extends Record<string, unknown>>(o: T) => o, absoluteFillObject: {} },
	};
});

import { Label } from "../../src/ui/Label";

function render(node: React.ReactElement) {
	let r!: TestRenderer.ReactTestRenderer;
	TestRenderer.act(() => {
		r = TestRenderer.create(node);
	});
	return r;
}

describe("Label", () => {
	it.each(["default", "dark", "muted"] as const)("renders %s variant", (variant) => {
		const r = render(<Label variant={variant}>Hello</Label>);
		const text = r.root.findAll((n) => (n.type as any) === "Text")[0];
		expect(text.props.children).toBe("Hello");
	});

	it("uppercases text in caps variant", () => {
		const r = render(<Label variant="caps">hello</Label>);
		const text = r.root.findAll((n) => (n.type as any) === "Text")[0];
		expect(text.props.children).toBe("HELLO");
	});
});
