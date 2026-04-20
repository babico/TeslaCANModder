import React from "react";
import TestRenderer from "react-test-renderer";

(globalThis as { IS_REACT_ACT_ENVIRONMENT?: boolean }).IS_REACT_ACT_ENVIRONMENT = true;

const makeState = (overrides: Record<string, any> = {}) =>
	({
		hasBms: false,
		chassisOnline: true,
		bmsSoc: 0,
		bmsPower: 0,
		bmsFullyCharged: false,
		bmsChargeTimeToFull: 0,
		bmsEnergyToCharge: 0,
		bmsContactorState: 0,
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

import { ChargeStatusCard } from "../../src/components/ChargeStatusCard";

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

describe("ChargeStatusCard", () => {
	it("renders unavailable message when no BMS", () => {
		const r = render(<ChargeStatusCard state={makeState()} />);
		expect(texts(r)).toContain("BMS data unavailable");
	});

	it("renders charging metrics when state indicates charging", () => {
		const state = makeState({
			hasBms: true,
			chassisOnline: true,
			bmsSoc: 0.82,
			bmsPower: -7.4,
			bmsChargeTimeToFull: 42,
			bmsEnergyToCharge: 18.5,
			bmsContactorState: 1,
		});
		const r = render(<ChargeStatusCard state={state} />);
		const t = texts(r);
		expect(t).toContain("82");
		expect(t).toContain("7.4");
		expect(t).toContain("42");
	});
});
