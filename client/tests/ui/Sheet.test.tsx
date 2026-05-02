import React from "react";
import { render, fireEvent } from "@testing-library/react-native";
import { Sheet } from "../../src/ui/Sheet";

describe("Sheet", () => {
	it("does not render Modal content when not visible", () => {
		const { queryByText } = render(
			<Sheet visible={false} onClose={jest.fn()} title="T">
				<></>
			</Sheet>,
		);
		expect(queryByText("T")).toBeNull();
	});

	it("renders title and children when visible", () => {
		const { getByText } = render(
			<Sheet visible onClose={jest.fn()} title="My Sheet">
				<></>
			</Sheet>,
		);
		expect(getByText("My Sheet")).toBeTruthy();
	});

	it("calls onClose when backdrop is pressed", () => {
		const onClose = jest.fn();
		const { UNSAFE_getAllByProps } = render(
			<Sheet visible onClose={onClose}>
				<></>
			</Sheet>,
		);
		const backdrop = UNSAFE_getAllByProps({ onPress: onClose })[0];
		fireEvent.press(backdrop);
		expect(onClose).toHaveBeenCalled();
	});
});
