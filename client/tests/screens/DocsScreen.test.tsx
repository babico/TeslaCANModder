// Full UI behaviour for DocsScreen is covered in tests/screens/screens.test.tsx.
// This file exists as a per-screen module-level smoke test.

jest.mock("../../src/docs/catalog", () => ({
	DEFAULT_DOC_ROUTE: "x",
	EMPTY_DOC_TREE: { kind: "folder", path: "", title: "Docs", order: 0, children: [] },
	loadDocsCatalog: async () => ({
		docsByRoute: {},
		docTree: { title: "Docs", children: [] },
	}),
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
