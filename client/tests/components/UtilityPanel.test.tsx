import React from "react";
import { render } from "@testing-library/react-native";
import { UtilityPanel } from "../../src/components/UtilityPanel";

const makeState = (overrides: Record<string, any> = {}) =>
({
seatbeltEmulation: false,
speedAlert: false,
canSim: false,
singleShot: false,
wiperPersist: false,
mirrorAutoFold: false,
apGateEnabled: true,
apGateOpen: false,
apGateReason: "waiting",
hasBtnMap: false,
btnMapLampShort: "",
btnMapLampLong: "",
btnMapLampDouble: "",
btnMapParkShort: "",
btnMapParkLong: "",
btnMapParkDouble: "",
...overrides,
}) as any;

describe("UtilityPanel", () => {
it("renders utility feature labels", () => {
const { getByText } = render(<UtilityPanel state={makeState()} />);
expect(getByText(/Seatbelt Emulation/)).toBeTruthy();
expect(getByText(/AP Injection Gate/)).toBeTruthy();
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
const { getByText } = render(<UtilityPanel state={state} />);
expect(getByText(/voiceCmd/)).toBeTruthy();
expect(getByText(/parkHold/)).toBeTruthy();
});

it("shows Unavailable when hasBtnMap is false", () => {
const { getAllByText } = render(<UtilityPanel state={makeState()} />);
expect(getAllByText(/Unavailable/).length).toBeGreaterThan(0);
});
});
