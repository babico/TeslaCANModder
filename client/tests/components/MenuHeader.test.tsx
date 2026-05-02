import React from "react";
import { render, fireEvent } from "@testing-library/react-native";
import { MenuHeader } from "../../src/components/MenuHeader";

const tabs: any = [
{ id: "dashboard", label: "Dashboard" },
{ id: "console", label: "Console" },
{ id: "docs", label: "Docs" },
];

describe("MenuHeader", () => {
it("renders all tab labels", () => {
const { getByText } = render(
<MenuHeader tabs={tabs} activeTab="dashboard" onSelectTab={jest.fn()} />,
);
expect(getByText("Dashboard")).toBeTruthy();
expect(getByText("Console")).toBeTruthy();
expect(getByText("Docs")).toBeTruthy();
});

it("calls onSelectTab when a tab is pressed", () => {
const onSelectTab = jest.fn();
const { getByText } = render(
<MenuHeader tabs={tabs} activeTab="dashboard" onSelectTab={onSelectTab} />,
);
fireEvent.press(getByText("Console"));
expect(onSelectTab).toHaveBeenCalledWith("console");
});
});