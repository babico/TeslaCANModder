// BoardConnectionContext provider/hook integration is covered indirectly via
// the headers.test.tsx, screens.test.tsx, and ControlsScreen.test.tsx suites
// that mock the hook. This file is a per-module smoke test to ensure the
// context module loads in isolation.

jest.mock("@teslacanmodder/protocol", () => ({
	initialBoardState: {},
	commands: {},
	PROFILE_LABELS: {},
	COMMAND_RANGES: {},
	VALID_VARIANTS: [],
}));

jest.mock("react-native", () => ({
	StyleSheet: { create: <T extends Record<string, unknown>>(o: T) => o, absoluteFillObject: {} },
}));

import * as BoardConnectionModule from "../../src/state/BoardConnectionContext";

describe("BoardConnectionContext module", () => {
	it("exports a useBoardConnection hook", () => {
		expect(typeof BoardConnectionModule.useBoardConnection).toBe("function");
	});

	it("exports CONNECTION_PRESETS array", () => {
		expect(Array.isArray(BoardConnectionModule.CONNECTION_PRESETS)).toBe(true);
	});
});
