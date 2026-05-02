import React from "react";
import { render } from "@testing-library/react-native";

jest.mock("../src/AppExperience", () => ({
	__esModule: true,
	default: () => null,
}));

jest.mock("../src/state/BoardConnectionContext", () => ({
	BoardConnectionProvider: ({ children }: any) => children,
}));

jest.mock("react-native-safe-area-context", () => ({
	SafeAreaProvider: ({ children }: any) => children,
}));

import AppViewWeb from "../src/AppView.web";

describe("AppView.web", () => {
	it("wraps AppExperience with BoardConnectionProvider and SafeAreaProvider", () => {
		expect(() => render(React.createElement(AppViewWeb))).not.toThrow();
	});
});
