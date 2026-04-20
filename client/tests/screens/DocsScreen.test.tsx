// Full UI behaviour for DocsScreen is covered in tests/screens/screens.test.tsx.
// This file exists as a per-screen module-level smoke test.

jest.mock("react-native", () => ({
	View: () => null,
	Text: () => null,
	TextInput: () => null,
	Pressable: () => null,
	ScrollView: () => null,
	StyleSheet: { create: <T extends Record<string, unknown>>(o: T) => o, absoluteFillObject: {} },
	useWindowDimensions: () => ({ width: 800, height: 600 }),
}));

jest.mock("../../src/generated/docsContent", () => ({
	DEFAULT_DOC_ROUTE: "x",
	BUNDLED_DOCS_BY_ROUTE: {},
	DOC_TREE: { title: "Docs", children: [] },
}));

jest.mock("../../src/components/docs/MarkdownRenderer", () => ({
	MarkdownRenderer: () => null,
}));

import { DocsScreen } from "../../src/screens/DocsScreen";

describe("DocsScreen module", () => {
	it("exports a function component", () => {
		expect(typeof DocsScreen).toBe("function");
	});
});
