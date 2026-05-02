import React from "react";
import { render } from "@testing-library/react-native";
import { Card } from "../../src/ui/Card";

describe("Card", () => {
	it.each(["default", "dark", "subtle"] as const)(
		"renders %s variant with children",
		(variant) => {
			const { toJSON } = render(
				<Card variant={variant}>
					<>{"inner"}</>
				</Card>,
			);
			expect(toJSON()).not.toBeNull();
		},
	);
});
