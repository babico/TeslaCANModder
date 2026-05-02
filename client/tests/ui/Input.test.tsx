import React from "react";
import { render, fireEvent } from "@testing-library/react-native";
import { Input } from "../../src/ui/Input";

describe("Input", () => {
	it("renders label and hint when no error", () => {
		const { getByText } = render(<Input label="Name" hint="helpful hint" />);
		expect(getByText("Name")).toBeTruthy();
		expect(getByText("helpful hint")).toBeTruthy();
	});

	it("renders error and hides hint when error provided", () => {
		const { getByText, queryByText } = render(
			<Input label="Name" hint="hint" error="required" />,
		);
		expect(getByText("required")).toBeTruthy();
		expect(queryByText("hint")).toBeNull();
	});

	it("forwards onChangeText", () => {
		const onChangeText = jest.fn();
		const { UNSAFE_getByType } = render(<Input onChangeText={onChangeText} />);
		const { TextInput } = require("react-native");
		fireEvent.changeText(UNSAFE_getByType(TextInput), "hello");
		expect(onChangeText).toHaveBeenCalledWith("hello");
	});

	it("renders dark variant", () => {
		const { getByText } = render(<Input label="L" variant="dark" />);
		expect(getByText("L")).toBeTruthy();
	});
});
