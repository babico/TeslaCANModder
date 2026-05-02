import React from "react";
import { render } from "@testing-library/react-native";

const markdownPropsCapture: {
	markdownit?: any;
	children?: React.ReactNode;
	onLinkPress?: (url: string) => boolean;
} = {};

jest.mock("react-native-markdown-display", () => {
	return ({
		markdownit,
		children,
		onLinkPress,
	}: {
		markdownit: any;
		children: React.ReactNode;
		onLinkPress?: (url: string) => boolean;
	}) => {
		markdownPropsCapture.markdownit = markdownit;
		markdownPropsCapture.children = children;
		markdownPropsCapture.onLinkPress = onLinkPress;
		return null;
	};
});

import { MarkdownRenderer } from "../../../src/components/docs/MarkdownRenderer";

describe("MarkdownRenderer", () => {
	it("renders markdown through package-based renderer", () => {
		render(React.createElement(MarkdownRenderer, { markdown: "# Title\n\n- [x] Task" }));
		expect(markdownPropsCapture.children).toBe("# Title\n\n- [x] Task");
	});

	it("supports extended markdown specs via markdown-it plugins", () => {
		render(
			React.createElement(MarkdownRenderer, {
				markdown:
					"| a | b |\n| --- | --- |\n| 1 | 2 |\n\n- [x] done\n\nTerm\n: Definition\n\nH~2~O and x^2^\n\n[^1]\n\n[^1]: footnote",
			}),
		);

		const html = markdownPropsCapture.markdownit.render(String(markdownPropsCapture.children));

		expect(html).toContain("<table>");
		expect(html).toContain("task-list-item");
		expect(html).toContain("<dl>");
		expect(html).toContain("<sub>");
		expect(html).toContain("<sup>");
		expect(html).toContain("footnote");
	});

	it("forwards onLinkPress handler", () => {
		const onLinkPress = jest.fn(() => false);
		render(
			React.createElement(MarkdownRenderer, {
				markdown: "[Docs](./getting-started.md)",
				onLinkPress,
			}),
		);

		expect(markdownPropsCapture.onLinkPress).toBeDefined();
		const result = markdownPropsCapture.onLinkPress?.(
			"../../../src/components/docs/getting-started.md",
		);
		expect(onLinkPress).toHaveBeenCalledWith("../../../src/components/docs/getting-started.md");
		expect(result).toBe(false);
	});
});
