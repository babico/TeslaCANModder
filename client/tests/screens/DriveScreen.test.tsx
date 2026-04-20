jest.mock("@teslacanmodder/protocol", () => ({
	initialBoardState: {},
	commands: {},
}));

jest.mock("react-native", () => ({
	View: () => null,
	Text: () => null,
	ScrollView: () => null,
	Pressable: () => null,
	StyleSheet: { create: <T extends Record<string, unknown>>(o: T) => o, absoluteFillObject: {} },
	useWindowDimensions: () => ({ width: 800, height: 600 }),
}));

jest.mock("react-native-svg", () => ({
	__esModule: true,
	default: () => null,
	Circle: () => null,
	Path: () => null,
	Rect: () => null,
	G: () => null,
	Line: () => null,
	Text: () => null,
}));

jest.mock("../../src/state/BoardConnectionContext", () => ({
	useBoardConnection: () => ({ statusText: "", sendCommand: jest.fn(), board: {} }),
}));

import { DriveScreen } from "../../src/screens/DriveScreen";

describe("DriveScreen module", () => {
	it("exports a function component", () => {
		expect(typeof DriveScreen).toBe("function");
	});
});
