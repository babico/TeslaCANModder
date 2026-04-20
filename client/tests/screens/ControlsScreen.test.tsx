import React from "react";
import TestRenderer from "react-test-renderer";

(globalThis as { IS_REACT_ACT_ENVIRONMENT?: boolean }).IS_REACT_ACT_ENVIRONMENT = true;

jest.mock("react-native", () => {
  const View = (props: any) => React.createElement("View", props, props.children);
  const Text = (props: any) => React.createElement("Text", props, props.children);
  const TextInput = (props: any) => React.createElement("TextInput", props, props.children);
  const ScrollView = (props: any) => React.createElement("ScrollView", props, props.children);
  const Pressable = (props: any) => React.createElement("Pressable", props, props.children);
  const Modal = (props: any) => React.createElement("Modal", props, props.children);

  return {
    View,
    Text,
    TextInput,
    ScrollView,
    Pressable,
    Modal,
    StyleSheet: {
      create: <T extends Record<string, unknown>>(obj: T) => obj,
      absoluteFillObject: {},
    },
  };
});

jest.mock("../../src/state/commandGating", () => ({
  getCommandGate: () => ({ available: true, reason: null }),
}));

jest.mock("../../src/hardware/controller", () => ({
  ALL_COMMANDS: [
    { name: "lock", argCount: 0 },
    { name: "unlock", argCount: 0 },
    { name: "horn", argCount: 0 },
    { name: "powerOff", argCount: 0 },
  ],
}));

import { ControlsScreen } from "../../src/screens/ControlsScreen";

describe("ControlsScreen", () => {
  const boardState: any = {
    chassisOnline: true,
    vehicleOnline: true,
    bodyOnline: true,
    otaInProgress: false,
    hasBms: true,
    vehicleSpeed: 0,
    fsd: false,
  };

  it("renders preset controls and opens palette", () => {
    let renderer!: TestRenderer.ReactTestRenderer;

    TestRenderer.act(() => {
      renderer = TestRenderer.create(
        React.createElement(ControlsScreen, {
          boardState,
          onRunCommand: jest.fn(),
        })
      );
    });

    const texts = renderer.root
      .findAll((node) => (node.type as any) === "Text")
      .map((node) => node.props.children)
      .flat()
      .join(" ");

    expect(texts).toContain("Speed Profile Controls");
    expect(texts).toContain("Command Palette");
    expect(texts).toContain("Battery (BMS)");

    const paletteButton = renderer.root.findAll((node) => {
      if ((node.type as any) !== "Pressable") return false;
      const text = node.findAll((child) => (child.type as any) === "Text")
        .map((child) => child.props.children)
        .flat()
        .join(" ");
      return text.includes("Palette");
    })[0];

    TestRenderer.act(() => {
      paletteButton.props.onPress();
    });

    expect(renderer.root.findAll((node) => (node.type as any) === "Modal").length).toBeGreaterThan(0);
  });
});
