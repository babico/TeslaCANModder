import React from "react";
import { render, fireEvent } from "@testing-library/react-native";
import { DocTreeItem } from "../../../src/components/docs/DocTreeItem";

describe("DocTreeItem", () => {
	it("renders leaf with bullet", () => {
		const { getByText } = render(
			<DocTreeItem
				title="Leaf"
				depth={0}
				hasNextSiblings={[]}
				isFolder={false}
				onPress={jest.fn()}
			/>,
		);
		expect(getByText("Leaf")).toBeTruthy();
		expect(getByText("\u2022")).toBeTruthy();
	});

	it("renders collapsed branch with arrow pointing right", () => {
		const { getByText } = render(
			<DocTreeItem
				title="Branch"
				depth={0}
				hasNextSiblings={[]}
				isFolder
				expanded={false}
				onPress={jest.fn()}
			/>,
		);
		expect(getByText("\u25b8")).toBeTruthy();
	});

	it("renders expanded branch with down arrow", () => {
		const { getByText } = render(
			<DocTreeItem
				title="Branch"
				depth={0}
				hasNextSiblings={[]}
				isFolder
				expanded
				onPress={jest.fn()}
			/>,
		);
		expect(getByText("\u25be")).toBeTruthy();
	});

	it("fires onPress when tapped", () => {
		const onPress = jest.fn();
		const { getByText } = render(
			<DocTreeItem
				title="Item"
				depth={0}
				hasNextSiblings={[]}
				isFolder={false}
				onPress={onPress}
			/>,
		);
		fireEvent.press(getByText("Item"));
		expect(onPress).toHaveBeenCalled();
	});
});
