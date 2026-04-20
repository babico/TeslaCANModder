import React from "react";
import TestRenderer from "react-test-renderer";

(globalThis as { IS_REACT_ACT_ENVIRONMENT?: boolean }).IS_REACT_ACT_ENVIRONMENT = true;

jest.mock("../src/AppExperience", () => ({
  __esModule: true,
  default: () => React.createElement("AppExperienceMock", null, "app"),
}));

jest.mock("../src/state/BoardConnectionContext", () => ({
  BoardConnectionProvider: ({ children }: { children: React.ReactNode }) =>
    React.createElement("BoardConnectionProviderMock", null, children),
}));

jest.mock("react-native-safe-area-context", () => ({
  SafeAreaProvider: ({ children }: { children: React.ReactNode }) =>
    React.createElement("SafeAreaProviderMock", null, children),
}));

import AppViewWeb from "../src/AppView.web";

describe("AppView.web", () => {
  it("wraps AppExperience with BoardConnectionProvider", () => {
    let renderer!: TestRenderer.ReactTestRenderer;
    TestRenderer.act(() => {
      renderer = TestRenderer.create(React.createElement(AppViewWeb));
    });

    const tree = renderer.root;
    const provider = tree.findAll((node) => (node.type as any) === "BoardConnectionProviderMock");
    const safeArea = tree.findAll((node) => (node.type as any) === "SafeAreaProviderMock");
    const experience = tree.findAll((node) => (node.type as any) === "AppExperienceMock");

    expect(provider.length).toBe(1);
    expect(safeArea.length).toBe(1);
    expect(experience.length).toBe(1);
  });
});
