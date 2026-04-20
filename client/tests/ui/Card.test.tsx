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

import { Card } from "../../src/ui/Card";

describe("Card", () => {
	it.each(["default", "dark", "subtle"] as const)("renders %s variant with children", (variant) => {
		let r!: TestRenderer.ReactTestRenderer;
		TestRenderer.act(() => {
			r = TestRenderer.create(
				<Card variant={variant}>
					<>{"inner"}</>
				</Card>,
			);
		});
		const views = r.root.findAll((n) => (n.type as any) === "View");
		expect(views.length).toBeGreaterThan(0);
	});
});
