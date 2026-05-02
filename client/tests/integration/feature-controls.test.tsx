import * as React from "react";
import { render, fireEvent } from "@testing-library/react-native";

jest.mock("@teslacanmodder/protocol", () => ({
commands: { status: () => "status", lock: () => "lock" },
}));

jest.mock("../../src/state/commandGating", () => ({
getCommandGate: () => ({ available: true, reason: null }),
}));

import { TooltipBanner } from "../../src/components/controls/ControlsBlocks";

describe("feature-controls integration", () => {
it("renders a TooltipBanner and fires onClose when pressed", () => {
const onClose = jest.fn();
const { getByText } = render(
React.createElement(TooltipBanner, {
message: "Tap to dismiss",
onClose,
}),
);
fireEvent.press(getByText("✕"));
expect(onClose).toHaveBeenCalledTimes(1);
});
});
