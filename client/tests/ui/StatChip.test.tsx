import React from "react";
import TestRenderer from "react-test-renderer";

(globalThis as { IS_REACT_ACT_ENVIRONMENT?: boolean }).IS_REACT_ACT_ENVIRONMENT = true;

jest.mock("react-native", () => {
	const View = (props: any) => React.createElement("View", props, props.children);
	const Text = (props: any) => React.createElement("Text", props, props.children);
	return {
		View,
		Text,
		StyleSheet: { create: <T extends Record<string, unknown>>(o: T) => o, absoluteFillObject: {} },
	};
});

import { StatChip } from "../../src/ui/StatChip";

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

describe("StatChip", () => {
	it("renders label and value", () => {
		const r = render(<StatChip label="Speed" value="42" />);
		const t = texts(r);
		expect(t).toContain("Speed");
		expect(t).toContain("42");
	});
	it("renders unit when provided", () => {
		const r = render(<StatChip label="Speed" value="42" unit="mph" />);
		expect(texts(r)).toContain("mph");
	});
	it("renders dark+accent variant", () => {
		const r = render(<StatChip label="X" value="1" variant="dark" accent />);
		expect(texts(r)).toContain("1");
	});
});
