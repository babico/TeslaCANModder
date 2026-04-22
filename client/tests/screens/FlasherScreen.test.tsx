// Full UI behaviour for FlasherScreen is covered in tests/screens/screens.test.tsx.
// This file exists as a per-screen module-level smoke test.

jest.mock("react-native", () => ({
	View: () => null,
	Text: () => null,
	Pressable: () => null,
	ScrollView: () => null,
	Linking: { openURL: jest.fn(async () => true) },
	StyleSheet: { create: <T extends Record<string, unknown>>(o: T) => o, absoluteFillObject: {} },
	useWindowDimensions: () => ({ width: 800, height: 600 }),
	Platform: { OS: "web" },
}));

jest.mock("../../src/state/BoardConnectionContext", () => ({
	useBoardConnection: () => ({ statusText: "", sendCommand: jest.fn(), board: {} }),
}));

import { FlasherScreen } from "../../src/screens/FlasherScreen";

describe("FlasherScreen module", () => {
	it("exports a function component", () => {
		expect(typeof FlasherScreen).toBe("function");
	});
});
