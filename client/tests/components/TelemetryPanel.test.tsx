import React from "react";
import { render } from "@testing-library/react-native";
import { TelemetryPanel } from "../../src/components/TelemetryPanel";

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
it("renders Firmware section by default", () => {
const { getByText } = render(<TelemetryPanel state={makeState()} />);
expect(getByText(/firmware/i)).toBeTruthy();
expect(getByText(/steering-mode decode/i)).toBeTruthy();
});

it("renders BMS section when hasBms is true", () => {
const state = makeState({
hasBms: true,
bmsTempMin: 18,
bmsTempMax: 27,
bmsHvState: 2,
bmsContactorState: 1,
});
const { getByText } = render(<TelemetryPanel state={state} />);
expect(getByText(/bms extended/i)).toBeTruthy();
});

it("renders CAN health rows when canHealth has entries", () => {
const state = makeState({
canHealth: { chassis: { on: true, det: true }, body: { on: false, det: false } },
});
	const { getAllByText } = render(<TelemetryPanel state={state} />);
	expect(getAllByText(/chassis/i).length).toBeGreaterThan(0);
	expect(getAllByText(/body/i).length).toBeGreaterThan(0);
});
});
