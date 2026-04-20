import React from "react";
import TestRenderer from "react-test-renderer";

(globalThis as { IS_REACT_ACT_ENVIRONMENT?: boolean }).IS_REACT_ACT_ENVIRONMENT = true;

let mockWidth = 1280;

jest.mock("react-native", () => {
  const View = (props: any) => React.createElement("View", props, props.children);
  const Text = (props: any) => React.createElement("Text", props, props.children);
  const TextInput = (props: any) => React.createElement("TextInput", props, props.children);
  const ScrollView = (props: any) => React.createElement("ScrollView", props, props.children);
  const Pressable = (props: any) => React.createElement("Pressable", props, props.children);
  const Modal = (props: any) => React.createElement("Modal", props, props.children);
  const Switch = (props: any) => React.createElement("Switch", props, props.children);

  return {
    View,
    Text,
    TextInput,
    ScrollView,
    Pressable,
    Modal,
    Switch,
    Platform: { OS: "web" },
    StyleSheet: {
      create: <T extends Record<string, unknown>>(obj: T) => obj,
      absoluteFillObject: {},
    },
    useWindowDimensions: () => ({ width: mockWidth, height: 800 }),
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
  ],
}));

jest.mock("../../src/generated/docsContent", () => ({
  DEFAULT_DOC_ROUTE: "getting-started",
  BUNDLED_DOCS_BY_ROUTE: {
    "getting-started": {
      routePath: "getting-started",
      path: "docs/getting-started.md",
      description: "Welcome doc",
      body: "# Getting Started\n\n| A | B |\n| --- | --- |\n| 1 | 2 |",
    },
    "guides/setup": {
      routePath: "guides/setup",
      path: "docs/guides/setup.md",
      description: "Setup",
      body: "## Setup",
    },
  },
  DOC_TREE: {
    title: "Docs",
    children: [
      { kind: "doc", path: "getting-started", routePath: "getting-started", title: "Getting Started" },
      {
        kind: "folder",
        path: "guides",
        title: "Guides",
        children: [
          { kind: "doc", path: "guides/setup", routePath: "guides/setup", title: "Setup" },
        ],
      },
    ],
  },
}));

jest.mock("../../src/components/docs/MarkdownRenderer", () => ({
  MarkdownRenderer: ({ markdown }: { markdown: string }) => React.createElement("MarkdownRenderer", { markdown }),
}));

jest.mock("../../src/state/BoardConnectionContext", () => ({
  useBoardConnection: () => ({
    statusText: "connected",
    sendCommand: jest.fn(async () => "ok"),
  }),
}));

import { ControlsScreen } from "../../src/screens/ControlsScreen";
import { ConsoleScreen } from "../../src/screens/ConsoleScreen";
import { FlasherScreen } from "../../src/screens/FlasherScreen";
import { DocsScreen } from "../../src/screens/DocsScreen";

describe("Screen UI coverage", () => {
  const boardState: any = {
    chassisOnline: true,
    vehicleOnline: true,
    bodyOnline: true,
    otaInProgress: false,
    hasBms: true,
    vehicleSpeed: 0,
    frameCount: 2,
    canHealth: { 0: { on: true, det: true } },
  };

  it("renders ControlsScreen and opens command palette", () => {
    let renderer!: TestRenderer.ReactTestRenderer;

    TestRenderer.act(() => {
      renderer = TestRenderer.create(
        React.createElement(ControlsScreen, {
          boardState,
          onRunCommand: jest.fn(),
        })
      );
    });

    const pressables = renderer.root.findAll((node) => {
      if ((node.type as any) !== "Pressable") return false;
      const text = node.findAll((child) => (child.type as any) === "Text")
        .map((child) => child.props.children)
        .flat()
        .join(" ");
      return text.includes("Palette");
    });

    TestRenderer.act(() => {
      pressables[0].props.onPress();
    });

    expect(renderer.root.findAll((node) => (node.type as any) === "Modal").length).toBeGreaterThan(0);
  });

  it("renders ConsoleScreen with mobile nav and diagnostics", () => {
    mockWidth = 420;
    let renderer!: TestRenderer.ReactTestRenderer;

    TestRenderer.act(() => {
      renderer = TestRenderer.create(
        React.createElement(ConsoleScreen, {
          availableCommands: ["status", "ping", "fsd"],
          boardState: { ...boardState, frameCount: 2 },
          selectedTransportOption: { id: "http", label: "REST API", detail: "", applyLabel: "", supportsInlineConfig: true },
          transportStatus: { tone: "ready", title: "REST transport active", detail: "ok" },
          frameCount: 2,
          visibleFrames: [
            { key: "f0", id: 0x100, busName: "Chassis", dlc: 8, dir: "rx", data: "00", bus: 0, ts: "1" },
          ],
          frameFilter: "",
          busFilter: "all",
          frameFeedPaused: false,
          boardInfoFeedPaused: false,
          frameWindowSize: 50,
          frameSampleStep: 1,
          selectedDecoderDataset: { id: "known", label: "Known", dataset: { dataset_source: { vehicle: "Tesla", firmware: "", mcu: "", soc: "" }, frames: [] } },
          decoderDatasets: [{ id: "known", label: "Known" }],
          liveDecodedFeed: [],
          frameDecodedNameByKey: {},
          diagnosticsQuery: "",
          diagnosticsCategory: "all",
          diagnosticsEvents: [],
          statusText: "idle",
          lastResult: "ready",
          history: [],
          bleDeviceName: "TeslaCANModder",
          bleConfigBusy: false,
          onFrameFilterChange: jest.fn(),
          onBusFilterChange: jest.fn(),
          onFrameWindowSizeChange: jest.fn(),
          onFrameSampleStepChange: jest.fn(),
          onFrameFeedPausedChange: jest.fn(),
          onBoardInfoFeedPausedChange: jest.fn(),
          onDiagnosticsQueryChange: jest.fn(),
          onDiagnosticsCategoryChange: jest.fn(),
          onDatasetChange: jest.fn(),
          onRunCommand: jest.fn(async () => undefined),
          onRunRawCommand: jest.fn(async () => "ok"),
          onFetchStatus: jest.fn(async () => undefined),
          onRefreshBleStatus: jest.fn(async () => undefined),
          onBleDeviceNameInputChange: jest.fn(),
          onApplyBleDeviceName: jest.fn(async () => undefined),
          onExportJson: jest.fn(),
          onExportCsv: jest.fn(),
          onExportRawJson: jest.fn(),
          onExportRawJsonl: jest.fn(),
          onExportDecodedJson: jest.fn(),
          onExportDecodedCsv: jest.fn(),
          onExportDatasetDbc: jest.fn(),
          onExportSessionPackage: jest.fn(),
          onSaveSnapshot: jest.fn(),
          onClearFeed: jest.fn(),
        })
      );
    });

    const textNodes = renderer.root.findAll((node) => (node.type as any) === "Text").map((node) => node.props.children).flat().join(" ");
    expect(textNodes).toContain("CAN Monitor");
    expect(textNodes).toContain("Events");
  });

  it("renders FlasherScreen with Chassis bus label", () => {
    let renderer!: TestRenderer.ReactTestRenderer;

    TestRenderer.act(() => {
      renderer = TestRenderer.create(React.createElement(FlasherScreen));
    });

    const textNodes = renderer.root.findAll((node) => (node.type as any) === "Text").map((node) => node.props.children).flat().join(" ");
    expect(textNodes).toContain("Chassis");
    expect(textNodes).not.toContain("CAN Buses FSD");
  });

  it("renders DocsScreen and navigates docs tree", () => {
    mockWidth = 1280;
    const onNavigateDoc = jest.fn();
    let renderer!: TestRenderer.ReactTestRenderer;

    TestRenderer.act(() => {
      renderer = TestRenderer.create(
        React.createElement(DocsScreen, {
          activeDocRoute: "getting-started",
          onNavigateDoc,
        })
      );
    });

    const pressables = renderer.root.findAll((node) => (node.type as any) === "Pressable");
    TestRenderer.act(() => {
      pressables[0].props.onPress();
    });

    expect(onNavigateDoc).toHaveBeenCalled();
    expect(renderer.root.findAll((node) => (node.type as any) === "MarkdownRenderer").length).toBe(1);
  });
});
