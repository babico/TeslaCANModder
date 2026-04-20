// Full UI behaviour for ConsoleScreen is covered in tests/screens/screens.test.tsx.
// This file exists as a per-screen module-level smoke test to guarantee the
// module loads independently and exports the expected component.

jest.mock("react-native", () => ({
	View: () => null,
	Text: () => null,
	TextInput: () => null,
	Pressable: () => null,
	ScrollView: () => null,
	Switch: () => null,
	StyleSheet: { create: <T extends Record<string, unknown>>(o: T) => o, absoluteFillObject: {} },
	useWindowDimensions: () => ({ width: 800, height: 600 }),
}));

jest.mock("../../src/state/BoardConnectionContext", () => ({
	useBoardConnection: () => ({ statusText: "", sendCommand: jest.fn(), board: {} }),
}));

import { ConsoleScreen } from "../../src/screens/ConsoleScreen";

describe("ConsoleScreen module", () => {
	it("exports a function component", () => {
		expect(typeof ConsoleScreen).toBe("function");
	});
});
