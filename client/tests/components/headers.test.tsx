import React from "react";
import TestRenderer from "react-test-renderer";

(globalThis as { IS_REACT_ACT_ENVIRONMENT?: boolean }).IS_REACT_ACT_ENVIRONMENT = true;

const boardConnState = {
  selectedTransportType: "http",
  isSelectedTransportReady: true,
  connectionBusy: false,
  baseUrl: "http://192.168.4.1",
  commandPath: "/api/command",
  statusPath: "/api/status",
  lastResult: "ready",
  setSelectedTransportType: jest.fn(),
  setBaseUrl: jest.fn(),
  setCommandPath: jest.fn(),
  setStatusPath: jest.fn(),
  applyConnection: jest.fn(async () => undefined),
  applyPreset: jest.fn(),
};

jest.mock("react-native", () => {
  const View = (props: any) => React.createElement("View", props, props.children);
  const Text = (props: any) => React.createElement("Text", props, props.children);
  const TextInput = (props: any) => React.createElement("TextInput", props, props.children);
  const ScrollView = (props: any) => React.createElement("ScrollView", props, props.children);
  const Pressable = (props: any) => React.createElement("Pressable", props, props.children);

  return {
    View,
    Text,
    TextInput,
    ScrollView,
    Pressable,
    StyleSheet: {
      create: <T extends Record<string, unknown>>(obj: T) => obj,
      absoluteFillObject: {},
    },
  };
});

jest.mock("../../src/components/ConnectionHeader", () => {
  const actual = jest.requireActual("../../src/components/ConnectionHeader");
  return actual;
});

jest.mock("../../src/state/BoardConnectionContext", () => ({
  CONNECTION_PRESETS: [
    {
      name: "Vehicle AP",
      connection: {
        baseUrl: "http://192.168.4.1",
        commandPath: "/api/command",
        statusPath: "/api/status",
      },
    },
  ],
  useBoardConnection: () => boardConnState,
}));

jest.mock("../../src/ui/Sheet", () => ({
  Sheet: ({ children }: { children: React.ReactNode }) => React.createElement("Sheet", null, children),
}));

import { MenuHeader } from "../../src/components/MenuHeader";
import { ConnectionHeader } from "../../src/components/ConnectionHeader";

describe("Header UI", () => {
  it("renders menu tabs and triggers tab selection", () => {
    const onSelectTab = jest.fn();
    let renderer!: TestRenderer.ReactTestRenderer;

    TestRenderer.act(() => {
      renderer = TestRenderer.create(
        React.createElement(MenuHeader, {
          tabs: [
            { id: "dashboard", label: "Dashboard" },
            { id: "console", label: "Console" },
          ],
          activeTab: "dashboard",
          onSelectTab,
        })
      );
    });

    const pressables = renderer.root.findAll((node) => (node.type as any) === "Pressable");
    TestRenderer.act(() => {
      pressables[1].props.onPress();
    });

    expect(onSelectTab).toHaveBeenCalledWith("console");
  });

  it("renders connection header with shared board context", () => {
    let renderer!: TestRenderer.ReactTestRenderer;

    TestRenderer.act(() => {
      renderer = TestRenderer.create(React.createElement(ConnectionHeader));
    });

    const textContent = renderer.root.findAll((node) => (node.type as any) === "Text").map((node) => node.props.children).flat().join(" ");

    expect(textContent).toContain("Tesla CAN Modder");
    expect(textContent).toContain("Connected");
    expect(textContent).toContain("REST API");
  });
});
