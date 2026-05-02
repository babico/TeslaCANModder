import React from "react";
import { render } from "@testing-library/react-native";
import { StatChip } from "../../src/ui/StatChip";

describe("StatChip", () => {
	it("renders label and value", () => {
		const { getByText } = render(<StatChip label="Speed" value="42" />);
		expect(getByText("Speed")).toBeTruthy();
		expect(getByText("42")).toBeTruthy();
	});
	it("renders unit when provided", () => {
		const { getByText } = render(<StatChip label="Speed" value="42" unit="mph" />);
		expect(getByText("mph")).toBeTruthy();
	});
	it("renders dark+accent variant", () => {
		const { getByText } = render(<StatChip label="X" value="1" variant="dark" accent />);
		expect(getByText("1")).toBeTruthy();
	});
});
