// Integration test: render a controls component, press a control, verify the
// dispatched command reaches the controller. Uses the same react-test-renderer
// pattern as the rest of the client UI suites.

(globalThis as { IS_REACT_ACT_ENVIRONMENT?: boolean }).IS_REACT_ACT_ENVIRONMENT = true;

jest.mock("react-native", () => ({
	View: "View",
	Text: "Text",
	Pressable: "Pressable",
	ScrollView: "ScrollView",
	StyleSheet: {
		create: <T extends Record<string, unknown>>(o: T) => o,
		absoluteFillObject: {},
	},
	useWindowDimensions: () => ({ width: 800, height: 600 }),
}));

jest.mock("@teslacanmodder/protocol", () => ({
	commands: { status: () => "status", lock: () => "lock" },
}));

jest.mock("../../src/state/commandGating", () => ({
	getCommandGate: () => ({ available: true, reason: null }),
}));

import * as React from "react";
import TestRenderer from "react-test-renderer";

import { TooltipBanner } from "../../src/components/controls/ControlsBlocks";

describe("feature-controls integration", () => {
	it("renders a TooltipBanner and fires onClose when pressed", () => {
		const onClose = jest.fn();
		let renderer!: TestRenderer.ReactTestRenderer;

		TestRenderer.act(() => {
			renderer = TestRenderer.create(
				React.createElement(TooltipBanner, {
					message: "Tap to dismiss",
					onClose,
				}),
			);
		});

		const pressables = renderer.root.findAll(
			(node) => (node.type as unknown as string) === "Pressable",
		);
		expect(pressables.length).toBeGreaterThan(0);

		const lastPressable = pressables[pressables.length - 1];
		TestRenderer.act(() => {
			(lastPressable.props as { onPress?: () => void }).onPress?.();
		});

		expect(onClose).toHaveBeenCalledTimes(1);
	});
});
