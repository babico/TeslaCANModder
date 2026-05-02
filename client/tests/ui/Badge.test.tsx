import React from "react";
import { render } from "@testing-library/react-native";
import { Badge } from "../../src/ui/Badge";

describe("Badge", () => {
	it("renders label children", () => {
		const { getByText } = render(<Badge>online</Badge>);
		expect(getByText("online")).toBeTruthy();
	});

	it.each(["default", "success", "warning", "destructive", "muted", "dark"] as const)(
		"renders %s variant",
		(variant) => {
			const { getByText } = render(<Badge variant={variant}>x</Badge>);
			expect(getByText("x")).toBeTruthy();
		},
	);
});
