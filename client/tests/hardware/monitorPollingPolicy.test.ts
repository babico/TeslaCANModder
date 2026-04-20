import {
  buildMonitorTransportGateSnapshot,
  getAutoPollPolicy,
  getCommandExecutionBlockReason,
  getStatusPollingBlockReason,
} from "../../src/hardware/monitorPollingPolicy";

describe("monitorPollingPolicy", () => {
  describe("getStatusPollingBlockReason", () => {
    it("returns null when status fetch is allowed", () => {
      expect(getStatusPollingBlockReason(true, "blocked")).toBeNull();
    });

    it("returns provided block reason when status fetch is not allowed", () => {
      expect(getStatusPollingBlockReason(false, "BLE is not active")).toBe(
        "BLE is not active"
      );
    });

    it("returns fallback reason when status fetch is not allowed and block reason is missing", () => {
      expect(getStatusPollingBlockReason(false)).toBe(
        "Selected transport is not ready for status polling."
      );
    });
  });

  describe("getCommandExecutionBlockReason", () => {
    it("returns null when command execution is allowed", () => {
      expect(getCommandExecutionBlockReason(true, "blocked")).toBeNull();
    });

    it("returns provided block reason when command execution is not allowed", () => {
      expect(getCommandExecutionBlockReason(false, "BLE is not active")).toBe(
        "BLE is not active"
      );
    });

    it("returns fallback reason when command execution is not allowed and block reason is missing", () => {
      expect(getCommandExecutionBlockReason(false)).toBe(
        "Selected transport is not ready for command execution."
      );
    });
  });

  describe("getAutoPollPolicy", () => {
    it("returns idle when auto poll is disabled", () => {
      expect(
        getAutoPollPolicy({
          autoPoll: false,
          canFetchStatus: true,
          pollSeconds: 2,
        })
      ).toEqual({ action: "idle" });
    });

    it("returns disable policy when transport is not ready", () => {
      expect(
        getAutoPollPolicy({
          autoPoll: true,
          canFetchStatus: false,
          pollSeconds: 2,
          blockReason: "Socket not active",
        })
      ).toEqual({ action: "disable", reason: "Socket not active" });
    });

    it("returns disable policy with fallback reason when transport is not ready and block reason is missing", () => {
      expect(
        getAutoPollPolicy({
          autoPoll: true,
          canFetchStatus: false,
          pollSeconds: 2,
        })
      ).toEqual({
        action: "disable",
        reason: "Auto poll disabled because selected transport is not active.",
      });
    });

    it("returns idle when poll interval is zero", () => {
      expect(
        getAutoPollPolicy({
          autoPoll: true,
          canFetchStatus: true,
          pollSeconds: 0,
        })
      ).toEqual({ action: "idle" });
    });

    it("returns start policy when all prerequisites are satisfied", () => {
      expect(
        getAutoPollPolicy({
          autoPoll: true,
          canFetchStatus: true,
          pollSeconds: 5,
        })
      ).toEqual({ action: "start", everySeconds: 5 });
    });
  });

  describe("buildMonitorTransportGateSnapshot", () => {
    it("returns fully enabled snapshot when transport is ready", () => {
      expect(buildMonitorTransportGateSnapshot(true, "blocked")).toEqual({
        canExecuteCommands: true,
        canFetchStatus: true,
        commandBlockReason: null,
        statusBlockReason: null,
      });
    });

    it("returns blocked snapshot with provided reason when transport is not ready", () => {
      expect(buildMonitorTransportGateSnapshot(false, "Socket not active")).toEqual({
        canExecuteCommands: false,
        canFetchStatus: false,
        commandBlockReason: "Socket not active",
        statusBlockReason: "Socket not active",
      });
    });
  });
});
