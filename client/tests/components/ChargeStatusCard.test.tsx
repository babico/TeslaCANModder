import React from "react";
import { render } from "@testing-library/react-native";
import { ChargeStatusCard } from "../../src/components/ChargeStatusCard";

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

describe("ChargeStatusCard", () => {
	it("renders unavailable message when no BMS", () => {
		const { getByText } = render(<ChargeStatusCard state={makeState()} />);
		expect(getByText(/BMS data unavailable/)).toBeTruthy();
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
		const { getByText } = render(<ChargeStatusCard state={state} />);
		expect(getByText(/82/)).toBeTruthy();
		expect(getByText(/7\.4/)).toBeTruthy();
		expect(getByText(/42/)).toBeTruthy();
	});
});
