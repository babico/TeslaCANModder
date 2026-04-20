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

import { Badge } from "../../src/ui/Badge";

function texts(renderer: TestRenderer.ReactTestRenderer): string {
	return renderer.root
		.findAll((n) => (n.type as any) === "Text")
		.map((n) => n.props.children)
		.flat()
		.join(" ");
}

describe("Badge", () => {
	it("renders label children", () => {
		let r!: TestRenderer.ReactTestRenderer;
		TestRenderer.act(() => {
			r = TestRenderer.create(<Badge>online</Badge>);
		});
		expect(texts(r)).toContain("online");
	});

	it.each(["default", "success", "warning", "destructive", "muted", "dark"] as const)(
		"renders %s variant",
		(variant) => {
			let r!: TestRenderer.ReactTestRenderer;
			TestRenderer.act(() => {
				r = TestRenderer.create(<Badge variant={variant}>x</Badge>);
			});
			expect(texts(r)).toContain("x");
		},
	);
});
