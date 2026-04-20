import {
  buildMonitorControlState,
  resolveQuickActionBlockReason,
} from "../../src/hardware/monitorControlState";

describe("monitorControlState", () => {
  it("returns fully enabled state when command and transport are both available", () => {
    expect(
      buildMonitorControlState({
        selectedCommandAvailable: true,
        transportCanExecuteCommands: true,
        transportCanFetchStatus: true,
      })
    ).toEqual({
      canFetchStatus: true,
      canRunTransportCommand: true,
      canRunSelectedCommand: true,
      selectedCommandWarning: null,
      transportCommandWarning: null,
      autoPollWarning: null,
    });
  });

  it("returns selected command warning and blocks selected command when command gate is unavailable", () => {
    expect(
      buildMonitorControlState({
        selectedCommandAvailable: false,
        selectedCommandReason: "Vehicle bus is offline",
        transportCanExecuteCommands: true,
        transportCanFetchStatus: true,
      })
    ).toEqual({
      canFetchStatus: true,
      canRunTransportCommand: true,
      canRunSelectedCommand: false,
      selectedCommandWarning: "⊘ Vehicle bus is offline",
      transportCommandWarning: null,
      autoPollWarning: null,
    });
  });

  it("returns transport command warning and auto-poll warning when transport is not ready", () => {
    expect(
      buildMonitorControlState({
        selectedCommandAvailable: true,
        transportCanExecuteCommands: false,
        transportCanFetchStatus: false,
        transportCommandBlockReason: "BLE is selected but not active",
        transportStatusBlockReason: "BLE is selected but not active",
      })
    ).toEqual({
      canFetchStatus: false,
      canRunTransportCommand: false,
      canRunSelectedCommand: false,
      selectedCommandWarning: null,
      transportCommandWarning: "⊘ BLE is selected but not active",
      autoPollWarning: "⊘ BLE is selected but not active",
    });
  });

  describe("resolveQuickActionBlockReason", () => {
    it("returns null when command and transport are both available", () => {
      expect(
        resolveQuickActionBlockReason({
          commandAvailable: true,
          transportCanExecuteCommands: true,
        })
      ).toBeNull();
    });

    it("prioritizes command gate reason when command is unavailable", () => {
      expect(
        resolveQuickActionBlockReason({
          commandAvailable: false,
          commandReason: "Vehicle bus is offline",
          transportCanExecuteCommands: true,
        })
      ).toBe("Vehicle bus is offline");
    });

    it("returns transport block reason when transport is unavailable", () => {
      expect(
        resolveQuickActionBlockReason({
          commandAvailable: true,
          transportCanExecuteCommands: false,
          transportCommandBlockReason: "BLE is selected but not active",
        })
      ).toBe("BLE is selected but not active");
    });
  });
});
