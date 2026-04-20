import React from "react";
import TestRenderer from "react-test-renderer";

(globalThis as { IS_REACT_ACT_ENVIRONMENT?: boolean }).IS_REACT_ACT_ENVIRONMENT = true;

jest.mock("react-native", () => {
	const View = (props: any) => React.createElement("View", props, props.children);
	const Text = (props: any) => React.createElement("Text", props, props.children);
	const Pressable = (props: any) => React.createElement("Pressable", props, props.children);
	const Modal = (props: any) =>
		props.visible ? React.createElement("Modal", props, props.children) : null;
	const Animated = {
		Value: function (this: any, v: number) {
			(this as any).__v = v;
		},
		timing: () => ({ start: () => undefined }),
		parallel: () => ({ start: () => undefined }),
		View,
	};
	return {
		View,
		Text,
		Pressable,
		Modal,
		Animated,
		Platform: { OS: "ios" },
		StyleSheet: { create: <T extends Record<string, unknown>>(o: T) => o, absoluteFill: {}, absoluteFillObject: {} },
	};
});

import { Sheet } from "../../src/ui/Sheet";

describe("Sheet", () => {
	it("does not render Modal content when not visible", () => {
		let r!: TestRenderer.ReactTestRenderer;
		TestRenderer.act(() => {
			r = TestRenderer.create(
				<Sheet visible={false} onClose={jest.fn()} title="T">
					<></>
				</Sheet>,
			);
		});
		expect(r.root.findAll((n) => (n.type as any) === "Modal")).toHaveLength(0);
	});

	it("renders title and children when visible", () => {
		let r!: TestRenderer.ReactTestRenderer;
		TestRenderer.act(() => {
			r = TestRenderer.create(
				<Sheet visible onClose={jest.fn()} title="My Sheet">
					<></>
				</Sheet>,
			);
		});
		const t = r.root
			.findAll((n) => (n.type as any) === "Text")
			.map((n) => String(n.props.children))
			.join(" ");
		expect(t).toContain("My Sheet");
	});

	it("calls onClose when backdrop is pressed", () => {
		const onClose = jest.fn();
		let r!: TestRenderer.ReactTestRenderer;
		TestRenderer.act(() => {
			r = TestRenderer.create(
				<Sheet visible onClose={onClose}>
					<></>
				</Sheet>,
			);
		});
		const pressables = r.root.findAll((n) => (n.type as any) === "Pressable");
		TestRenderer.act(() => pressables[0].props.onPress());
		expect(onClose).toHaveBeenCalled();
	});
});
