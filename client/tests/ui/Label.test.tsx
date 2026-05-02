import React from "react";
import { render } from "@testing-library/react-native";
import { Label } from "../../src/ui/Label";

describe("Label", () => {
	it.each(["default", "dark", "muted"] as const)("renders %s variant", (variant) => {
		const { getByText } = render(<Label variant={variant}>Hello</Label>);
		expect(getByText("Hello")).toBeTruthy();
	});

	it("uppercases text in caps variant", () => {
		const { getByText } = render(<Label variant="caps">hello</Label>);
		expect(getByText("HELLO")).toBeTruthy();
	});
});
