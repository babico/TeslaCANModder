import React from "react";
import { render } from "@testing-library/react-native";
import { Alert } from "../../src/ui/Alert";

describe("Alert", () => {
	it("renders description without title by default", () => {
		const { getByText } = render(<Alert description="hello world" />);
		expect(getByText("hello world")).toBeTruthy();
	});

	it("renders title when provided", () => {
		const { getByText } = render(<Alert title="Heads up" description="check this" />);
		expect(getByText("Heads up")).toBeTruthy();
		expect(getByText("check this")).toBeTruthy();
	});

	it.each(["default", "success", "warning", "destructive"] as const)(
		"renders %s variant",
		(variant) => {
			const { getByText } = render(<Alert description="msg" variant={variant} />);
			expect(getByText("msg")).toBeTruthy();
		},
	);
});
