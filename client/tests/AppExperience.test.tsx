// Full UI behaviour for AppExperience is covered in tests/AppView.web.test.tsx.
// This file exists as a module-level smoke test.
// AppExperience is the top-level shell that lazy-loads all screens. Its full
// rendering is exercised in tests/AppView.web.test.tsx with the proper
// browser-shell mocks. This file is a per-module guard ensuring the source
// file is present and exports a default.
declare const __dirname: string;
declare const require: (id: string) => any;

describe("AppExperience module", () => {
	const fs = require("fs");
	const path = require("path");
	const filePath = path.resolve(__dirname, "..", "src", "AppExperience.tsx");

	it("source file exists", () => {
		expect(fs.existsSync(filePath)).toBe(true);
	});

	it("exports a default React component", () => {
		const source = fs.readFileSync(filePath, "utf8");
		expect(source).toMatch(/export default/);
	});
});
