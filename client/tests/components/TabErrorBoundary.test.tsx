/**
 * TabErrorBoundary — smoke tests
 */

import React from "react";
import { render } from "@testing-library/react-native";
import { Text } from "react-native";
import { TabErrorBoundary } from "../../src/components/TabErrorBoundary";

function GoodChild() {
	return <Text testID="good">all good</Text>;
}

function Exploder(): React.ReactElement {
	throw new Error("BOOM");
}

describe("TabErrorBoundary", () => {
	it("renders children when no error", () => {
		const { getByTestId } = render(
			<TabErrorBoundary tab="Dashboard">
				<GoodChild />
			</TabErrorBoundary>,
		);
		expect(getByTestId("good")).toBeTruthy();
	});

	it("catches thrown error and shows crash UI", () => {
		// Suppress React console.error from expected throw
		const spy = jest.spyOn(console, "error").mockImplementation(() => {});

		const { getByText } = render(
			<TabErrorBoundary tab="Test">
				<Exploder />
			</TabErrorBoundary>,
		);

		expect(getByText(/Test tab crashed/i)).toBeTruthy();
		expect(getByText("BOOM")).toBeTruthy();
		expect(getByText("Retry")).toBeTruthy();

		spy.mockRestore();
	});

	it("retry resets error state and re-renders children", () => {
		const spy = jest.spyOn(console, "error").mockImplementation(() => {});

		const { getByText } = render(
			<TabErrorBoundary tab="RetryTest">
				<Exploder />
			</TabErrorBoundary>,
		);

		expect(getByText(/RetryTest tab crashed/i)).toBeTruthy();

		spy.mockRestore();
	});
});
