import React from "react";
import TestRenderer from "react-test-renderer";

(globalThis as { IS_REACT_ACT_ENVIRONMENT?: boolean }).IS_REACT_ACT_ENVIRONMENT = true;

jest.mock("react-native", () => {
	const Pressable = (props: any) =>
		React.createElement("Pressable", props, typeof props.style === "function" ? null : null, props.children);
	const Text = (props: any) => React.createElement("Text", props, props.children);
	return {
		Pressable,
		Text,
		StyleSheet: { create: <T extends Record<string, unknown>>(o: T) => o, absoluteFillObject: {} },
	};
});

import { Button } from "../../src/ui/Button";

function render(node: React.ReactElement) {
	let r!: TestRenderer.ReactTestRenderer;
	TestRenderer.act(() => {
		r = TestRenderer.create(node);
	});
	return r;
}

describe("Button", () => {
	it("renders label", () => {
		const r = render(<Button>Save</Button>);
		const t = r.root.findAll((n) => (n.type as any) === "Text")[0];
		expect(t.props.children).toBe("Save");
	});

	it("fires onPress when not disabled", () => {
		const onPress = jest.fn();
		const r = render(<Button onPress={onPress}>Go</Button>);
		const pressable = r.root.findAll((n) => (n.type as any) === "Pressable")[0];
		TestRenderer.act(() => pressable.props.onPress());
		expect(onPress).toHaveBeenCalledTimes(1);
	});

	it("does not fire onPress when disabled", () => {
		const onPress = jest.fn();
		const r = render(
			<Button onPress={onPress} disabled>
				Disabled
			</Button>,
		);
		const pressable = r.root.findAll((n) => (n.type as any) === "Pressable")[0];
		expect(pressable.props.onPress).toBeUndefined();
	});

	it.each(["primary", "secondary", "destructive", "ghost"] as const)(
		"renders %s variant",
		(variant) => {
			const r = render(<Button variant={variant}>v</Button>);
			expect(r.root.findAllByType("Text" as any)[0].props.children).toBe("v");
		},
	);
});
