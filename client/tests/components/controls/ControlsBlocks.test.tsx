import React from "react";
import { render, fireEvent } from "@testing-library/react-native";

jest.mock("../../../src/state/commandGating", () => ({
	getCommandGate: () => null,
}));

import { TooltipBanner } from "../../../src/components/controls/ControlsBlocks";

describe("ControlsBlocks / TooltipBanner", () => {
	it("renders message text", () => {
		const { getByText } = render(
			<TooltipBanner message="Chassis bus offline" onClose={jest.fn()} />,
		);
		expect(getByText(/Chassis bus offline/)).toBeTruthy();
	});

	it("calls onClose when dismissed", () => {
		const onClose = jest.fn();
		const { getByText } = render(
			<TooltipBanner message="x" onClose={onClose} />,
		);
		fireEvent.press(getByText("✕"));
		expect(onClose).toHaveBeenCalled();
	});
});
