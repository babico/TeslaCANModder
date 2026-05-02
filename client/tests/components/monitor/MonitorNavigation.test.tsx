import React from "react";
import { render, fireEvent } from "@testing-library/react-native";
import {
MonitorSidebarNavigation,
MonitorBottomBar,
} from "../../../src/components/monitor/MonitorNavigation";

const items: any = [
{ tab: "console", label: "Console", icon: "►" },
{ tab: "connection", label: "Connection", icon: "⚡" },
{ tab: "diagnostics", label: "Diagnostics", icon: "◉" },
];

describe("MonitorSidebarNavigation", () => {
it("renders all nav items, frame count and CAN status", () => {
const onChangeSection = jest.fn();
	const { getAllByText, getByText } = render(
		<MonitorSidebarNavigation
			items={items}
			activeSection="console"
			onChangeSection={onChangeSection}
			frameCount={42}
			canConnected={true}
		/>,
	);
	expect(getAllByText("Console").length).toBeGreaterThan(0);
	expect(getAllByText("Connection").length).toBeGreaterThan(0);
	expect(getByText(/42/)).toBeTruthy();
	expect(getByText(/CAN OK/)).toBeTruthy();
});

it("calls onChangeSection on item press", () => {
const onChange = jest.fn();
const { getAllByText } = render(
<MonitorSidebarNavigation
items={items}
activeSection="console"
onChangeSection={onChange}
frameCount={0}
canConnected={false}
/>,
);
fireEvent.press(getAllByText("Connection")[0]);
expect(onChange).toHaveBeenCalledWith("connection");
});
});

describe("MonitorBottomBar", () => {
it("renders all items and routes presses", () => {
const onChange = jest.fn();
const { getByText } = render(
<MonitorBottomBar items={items} activeSection="console" onChangeSection={onChange} />,
);
fireEvent.press(getByText("Diagnostics"));
expect(onChange).toHaveBeenCalledWith("diagnostics");
});
});
