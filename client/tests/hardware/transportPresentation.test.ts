import {
  MONITOR_TRANSPORT_OPTIONS,
  buildMonitorTransportStatus,
  getMonitorTransportExecutionPolicy,
  getMonitorTransportOption,
  isMonitorTransportReady,
} from "../../src/hardware/transportPresentation";

describe("transportPresentation", () => {
  it("exposes all supported monitor transport options", () => {
    expect(MONITOR_TRANSPORT_OPTIONS.map((option) => option.id)).toEqual([
      "http",
      "ble",
      "serial",
      "bluetooth-serial",
    ]);
  });

  it("returns a ready status for active REST transport", () => {
    expect(
      buildMonitorTransportStatus("http", "http", "http://192.168.4.1")
    ).toEqual({
      tone: "ready",
      title: "REST transport active",
      detail: "Monitor commands and status reads will use http://192.168.4.1.",
    });
  });

  it("returns a pending status when the selected transport is not active yet", () => {
    expect(
      buildMonitorTransportStatus("ble", "http", "http://192.168.4.1")
    ).toEqual({
      tone: "pending",
      title: "BLE selected",
      detail: "Waiting for runtime adapter wiring via connectViaBle(...). Active transport remains http.",
    });
  });

  it("returns the selected transport descriptor", () => {
    expect(getMonitorTransportOption("bluetooth-serial")).toMatchObject({
      label: "Bluetooth COM",
      supportsInlineConfig: false,
      applyLabel: "connectViaBluetoothComPort(...)",
    });
  });

  it("marks selected transport as ready when it matches active transport", () => {
    expect(isMonitorTransportReady("serial", "serial")).toBe(true);
  });

  it("marks selected transport as pending when it differs from active transport", () => {
    expect(isMonitorTransportReady("ble", "http")).toBe(false);
  });

  it("returns execution policy as ready when selected transport is active", () => {
    expect(getMonitorTransportExecutionPolicy("serial", "serial")).toEqual({
      ready: true,
    });
  });

  it("returns execution policy with block reason when selected transport is pending", () => {
    expect(getMonitorTransportExecutionPolicy("ble", "http")).toEqual({
      ready: false,
      blockReason:
        "BLE is selected but not active yet. Wire runtime adapter via connectViaBle(...) or switch back to the active http transport before running commands.",
    });
  });
});
