import React from "react";
import TestRenderer from "react-test-renderer";

(globalThis as { IS_REACT_ACT_ENVIRONMENT?: boolean }).IS_REACT_ACT_ENVIRONMENT = true;

jest.mock("react-native", () => {
	const View = (props: any) => React.createElement("View", props, props.children);
	const Text = (props: any) => React.createElement("Text", props, props.children);
	const Pressable = (props: any) => React.createElement("Pressable", props, props.children);
	const TextInput = (props: any) => React.createElement("TextInput", props, props.children);
	const ScrollView = (props: any) => React.createElement("ScrollView", props, props.children);
	const Modal = (props: any) =>
		props.visible ? React.createElement("Modal", props, props.children) : null;
	return {
		View,
		Text,
		Pressable,
		TextInput,
		ScrollView,
		Modal,
		StyleSheet: { create: <T extends Record<string, unknown>>(o: T) => o, absoluteFillObject: {} },
	};
});

jest.mock("react-native-svg", () => ({
	__esModule: true,
	default: (props: any) => React.createElement("Svg", props, props.children),
	Circle: (props: any) => React.createElement("Circle", props),
	Path: (props: any) => React.createElement("Path", props),
	Rect: (props: any) => React.createElement("Rect", props),
}));

jest.mock("../../../src/state/commandGating", () => ({
	getCommandGate: () => null,
}));

import { TooltipBanner } from "../../../src/components/controls/ControlsBlocks";

function render(node: React.ReactElement) {
	let r!: TestRenderer.ReactTestRenderer;
	TestRenderer.act(() => {
		r = TestRenderer.create(node);
	});
	return r;
}

describe("ControlsBlocks / TooltipBanner", () => {
	it("renders message text", () => {
		const r = render(<TooltipBanner message="Chassis bus offline" onClose={jest.fn()} />);
		const text = r.root
			.findAll((n) => (n.type as any) === "Text")
			.map((n) => String(n.props.children).replace(/,/g, ""))
			.join(" ");
		expect(text).toContain("Chassis bus offline");
	});

	it("calls onClose when dismissed", () => {
		const onClose = jest.fn();
		const r = render(<TooltipBanner message="x" onClose={onClose} />);
		const pressables = r.root.findAll((n) => (n.type as any) === "Pressable");
		TestRenderer.act(() => pressables[0].props.onPress());
		expect(onClose).toHaveBeenCalled();
	});
});
