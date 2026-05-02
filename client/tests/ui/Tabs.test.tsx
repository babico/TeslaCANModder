import React from "react";
import { render, fireEvent } from "@testing-library/react-native";
import { ScrollView } from "react-native";
import { Tabs } from "../../src/ui/Tabs";

const items = [
	{ key: "a", label: "Alpha" },
	{ key: "b", label: "Beta", badge: 3 },
];

describe("Tabs", () => {
	it("renders all labels and badge", () => {
		const { getByText } = render(<Tabs items={items} activeKey="a" onSelect={jest.fn()} />);
		expect(getByText("Alpha")).toBeTruthy();
		expect(getByText("Beta")).toBeTruthy();
		expect(getByText("3")).toBeTruthy();
	});

	it("calls onSelect for clicked tab", () => {
		const onSelect = jest.fn();
		const { getByText } = render(
			<Tabs items={items} activeKey="a" onSelect={onSelect} />,
		);
		fireEvent.press(getByText("Beta"));
		expect(onSelect).toHaveBeenCalledWith("b");
	});

	it.each(["default", "dark", "underline"] as const)("renders %s variant", (variant) => {
		const { getByText } = render(
			<Tabs items={items} activeKey="a" variant={variant} onSelect={jest.fn()} />,
		);
		expect(getByText("Alpha")).toBeTruthy();
		expect(getByText("Beta")).toBeTruthy();
	});

	it("wraps in ScrollView when scrollable", () => {
		const { UNSAFE_getAllByType } = render(
			<Tabs items={items} activeKey="a" scrollable onSelect={jest.fn()} />,
		);
		expect(UNSAFE_getAllByType(ScrollView).length).toBe(1);
	});
});
