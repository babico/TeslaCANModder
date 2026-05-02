jest.mock("@teslacanmodder/protocol", () => ({
	initialBoardState: {},
	commands: {},
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
