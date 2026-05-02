import React from "react";
import { render, fireEvent } from "@testing-library/react-native";

jest.mock("../../src/ui/Sheet", () => ({
	Sheet: ({ visible, children }: any) => (visible ? children : null),
}));

import { Select } from "../../src/ui/Select";

const options = [
	{ label: "One", value: "1" },
	{ label: "Two", value: "2" },
];

describe("Select", () => {
	it("shows placeholder when no value selected", () => {
		const { getByText } = render(
			<Select options={options} placeholder="Pick…" onChange={jest.fn()} />,
		);
		expect(getByText(/Pick/)).toBeTruthy();
	});

	it("renders selected label when value provided", () => {
		const { getByText } = render(<Select options={options} value="2" onChange={jest.fn()} />);
		expect(getByText("Two")).toBeTruthy();
	});

	it("opens sheet on press and fires onChange when option selected", () => {
		const onChange = jest.fn();
		const { getByText } = render(
			<Select options={options} onChange={onChange} placeholder="Pick…" />,
		);
		// Open the Select by pressing the trigger
		fireEvent.press(getByText("Pick…"));
		// Select first option from the open sheet
		fireEvent.press(getByText("One"));
		expect(onChange).toHaveBeenCalledWith("1");
	});

	it("does not open when disabled", () => {
		const onChange = jest.fn();
		const { queryByText } = render(<Select options={options} onChange={onChange} disabled />);
		expect(queryByText("One")).toBeNull();
		expect(queryByText("Two")).toBeNull();
	});

	it("renders error in place of hint", () => {
		const { getByText, queryByText } = render(
			<Select options={options} hint="hint" error="bad" onChange={jest.fn()} />,
		);
		expect(getByText("bad")).toBeTruthy();
		expect(queryByText("hint")).toBeNull();
	});
});
