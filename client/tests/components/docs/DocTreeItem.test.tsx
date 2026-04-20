import React from "react";
import TestRenderer from "react-test-renderer";

(globalThis as { IS_REACT_ACT_ENVIRONMENT?: boolean }).IS_REACT_ACT_ENVIRONMENT = true;

jest.mock("react-native", () => {
	const View = (props: any) => React.createElement("View", props, props.children);
	const Text = (props: any) => React.createElement("Text", props, props.children);
	const Pressable = (props: any) => React.createElement("Pressable", props, props.children);
	return {
		View,
		Text,
		Pressable,
		StyleSheet: { create: <T extends Record<string, unknown>>(o: T) => o, absoluteFillObject: {} },
	};
});

import { DocTreeItem } from "../../../src/components/docs/DocTreeItem";

function render(node: React.ReactElement) {
	let r!: TestRenderer.ReactTestRenderer;
	TestRenderer.act(() => {
		r = TestRenderer.create(node);
	});
	return r;
}

describe("DocTreeItem", () => {
	it("renders folder collapsed icon", () => {
		const r = render(
			<DocTreeItem
				title="Guides"
				depth={0}
				hasNextSiblings={[]}
				isFolder
				expanded={false}
				onPress={jest.fn()}
			/>,
		);
		const icons = r.root
			.findAll((n) => (n.type as any) === "Text")
			.map((n) => String(n.props.children));
		expect(icons).toContain("▸");
		expect(icons).toContain("Guides");
	});

	it("renders folder expanded icon", () => {
		const r = render(
			<DocTreeItem
				title="Guides"
				depth={1}
				hasNextSiblings={[true]}
				isFolder
				expanded
				onPress={jest.fn()}
			/>,
		);
		const icons = r.root
			.findAll((n) => (n.type as any) === "Text")
			.map((n) => String(n.props.children));
		expect(icons).toContain("▾");
	});

	it("renders leaf bullet for non-folders", () => {
		const r = render(
			<DocTreeItem
				title="intro.md"
				depth={0}
				hasNextSiblings={[]}
				isFolder={false}
				onPress={jest.fn()}
			/>,
		);
		const icons = r.root
			.findAll((n) => (n.type as any) === "Text")
			.map((n) => String(n.props.children));
		expect(icons).toContain("•");
	});

	it("fires onPress when pressed", () => {
		const onPress = jest.fn();
		const r = render(
			<DocTreeItem
				title="Guides"
				depth={0}
				hasNextSiblings={[]}
				isFolder
				onPress={onPress}
			/>,
		);
		const pressable = r.root.findAll((n) => (n.type as any) === "Pressable")[0];
		TestRenderer.act(() => pressable.props.onPress());
		expect(onPress).toHaveBeenCalled();
	});
});
