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

import { TelemetryPanel } from "../../src/components/TelemetryPanel";

function render(node: React.ReactElement) {
	let r!: TestRenderer.ReactTestRenderer;
	TestRenderer.act(() => {
		r = TestRenderer.create(node);
	});
	return r;
}

const makeState = (overrides: Record<string, any> = {}) =>
	({
		hasBms: false,
		hasTpms: false,
		hasPowertrain: false,
		hasSteeringMode: false,
		bmsTempMin: 0,
		bmsTempMax: 0,
		bmsHvState: 0,
		bmsContactorState: 0,
		bmsMaxRegenPower: 0,
		bmsMaxDischargePower: 0,
		bmsMinBusVoltage: 0,
		bmsMaxBusVoltage: 0,
		bmsPower: 0,
		tpmsPressureFL: 0,
		tpmsPressureFR: 0,
		tpmsPressureRL: 0,
		tpmsPressureRR: 0,
		tpmsTempFL: 0,
		tpmsTempFR: 0,
		tpmsTempRL: 0,
		tpmsTempRR: 0,
		fwCompat: 0,
		steeringMode: 0,
		driveMode: 0,
		uptime: 0,
		rate: 0,
		canHealth: {},
		...overrides,
	}) as any;

describe("TelemetryPanel", () => {
	it("renders Firmware section by default (always visible)", () => {
		const r = render(<TelemetryPanel state={makeState()} />);
		const t = r.root
			.findAll((n) => (n.type as any) === "Text")
			.map((n) => String(n.props.children).replace(/,/g, " "))
			.join(" ");
		expect(t.toLowerCase()).toContain("firmware");
	});

	it("renders BMS section when hasBms is true", () => {
		const state = makeState({
			hasBms: true,
			bmsTempMin: 18,
			bmsTempMax: 27,
			bmsHvState: 2,
			bmsContactorState: 1,
		});
		const r = render(<TelemetryPanel state={state} />);
		const t = r.root
			.findAll((n) => (n.type as any) === "Text")
			.map((n) => String(n.props.children).replace(/,/g, " "))
			.join(" ");
		expect(t.toLowerCase()).toContain("bms extended");
	});

	it("renders CAN health rows when canHealth has entries", () => {
		const state = makeState({
			canHealth: { chassis: { on: true, det: true }, body: { on: false, det: false } },
		});
		const r = render(<TelemetryPanel state={state} />);
		const t = r.root
			.findAll((n) => (n.type as any) === "Text")
			.map((n) => String(n.props.children).replace(/,/g, " "))
			.join(" ");
		expect(t).toContain("CHASSIS");
		expect(t).toContain("BODY");
	});
});
