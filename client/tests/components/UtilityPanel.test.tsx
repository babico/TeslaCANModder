import React from "react";
import TestRenderer from "react-test-renderer";

(globalThis as { IS_REACT_ACT_ENVIRONMENT?: boolean }).IS_REACT_ACT_ENVIRONMENT = true;

const makeState = (overrides: Record<string, any> = {}) =>
	({
		seatbeltEmulation: false,
		speedAlert: false,
		canSim: false,
		singleShot: false,
		wiperPersist: false,
		mirrorAutoFold: false,
		hasBtnMap: false,
		btnMapLampShort: "",
		btnMapLampLong: "",
		btnMapLampDouble: "",
		btnMapParkShort: "",
		btnMapParkLong: "",
		btnMapParkDouble: "",
		...overrides,
	}) as any;

jest.mock("react-native", () => {
	const View = (props: any) => React.createElement("View", props, props.children);
	const Text = (props: any) => React.createElement("Text", props, props.children);
	return {
		View,
		Text,
		StyleSheet: { create: <T extends Record<string, unknown>>(o: T) => o, absoluteFillObject: {} },
	};
});

import { UtilityPanel } from "../../src/components/UtilityPanel";

function render(node: React.ReactElement) {
	let r!: TestRenderer.ReactTestRenderer;
	TestRenderer.act(() => {
		r = TestRenderer.create(node);
	});
	return r;
}

function texts(r: TestRenderer.ReactTestRenderer) {
	return r.root
		.findAll((n) => (n.type as any) === "Text")
		.map((n) => String(n.props.children))
		.join(" ");
}

describe("UtilityPanel", () => {
	it("renders utility feature labels", () => {
		const r = render(<UtilityPanel state={makeState()} />);
		const t = texts(r);
		expect(t).toContain("Seatbelt Emulation");
		expect(t).toContain("CAN Simulation");
		expect(t).toContain("Wiper Persistence");
	});

	it("shows button mapping rows when hasBtnMap is true", () => {
		const state = makeState({
			hasBtnMap: true,
			btnMapLampShort: "voiceCmd",
			btnMapLampLong: "flashHigh",
			btnMapLampDouble: "toggleHazard",
			btnMapParkShort: "parkToggle",
			btnMapParkLong: "parkHold",
			btnMapParkDouble: "parkRelease",
		});
		const r = render(<UtilityPanel state={state} />);
		const t = texts(r);
		expect(t).toContain("voiceCmd");
		expect(t).toContain("parkHold");
	});

	it("shows Unavailable when hasBtnMap is false", () => {
		const r = render(<UtilityPanel state={makeState()} />);
		expect(texts(r)).toContain("Unavailable");
	});
});
