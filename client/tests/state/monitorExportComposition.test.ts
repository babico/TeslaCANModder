import type { CanFrame } from "@teslacanmodder/protocol";
import {
  buildExportHistoryEntry,
  buildExportRows,
  buildExportSummary,
  computeExportChecksum,
  formatExportRowsAsCsv,
  type ExportMetadata,
  type ExportRow,
  validateExportMode,
  verifyExportIntegrity,
} from "../../src/state/monitorExportComposition";

describe("buildExportRows", () => {
  it("converts CanFrame to ExportRow format", () => {
    const frames: CanFrame[] = [
      {
        key: "frame-1",
        id: 0x123,
        dir: "tx",
        bus: 0,
        busName: "Chassis",
        dlc: 8,
        data: "12 34 56 78 9A BC DE F0",
        ts: "1000",
      },
    ];

    const result = buildExportRows({ frames });

    expect(result).toHaveLength(1);
    expect(result[0].idHex).toBe("0x123");
    expect(result[0].bus).toBe(0);
    expect(result[0].busName).toBe("Chassis");
    expect(result[0].dir).toBe("tx");
    expect(result[0].data).toBe("12 34 56 78 9A BC DE F0");
    expect(result[0].ts).toBe("1000");
  });

  it("formats ID as uppercase hex with 0x prefix", () => {
    const frames: CanFrame[] = [
      {
        key: "f1",
        id: 0xabc,
        dir: "rx",
        bus: 1,
        busName: "Vehicle",
        dlc: 4,
        data: "00 01 02 03",
        ts: "100",
      },
    ];

    const result = buildExportRows({ frames });

    expect(result[0].idHex).toBe("0xABC");
  });

  it("handles multiple frames", () => {
    const frames: CanFrame[] = Array.from({ length: 3 }, (_, i) => ({
      key: `f${i}`,
      id: 0x100 + i,
      dir: i % 2 === 0 ? "tx" : "rx",
      bus: i % 2,
      busName: i % 2 === 0 ? "Chassis" : "Vehicle",
      dlc: 8,
      data: "00 00 00 00 00 00 00 00",
      ts: `${i * 100}`,
    }));

    const result = buildExportRows({ frames });

    expect(result).toHaveLength(3);
    expect(result[0].idHex).toBe("0x100");
    expect(result[1].idHex).toBe("0x101");
    expect(result[2].idHex).toBe("0x102");
  });
});

describe("computeExportChecksum", () => {
  it("produces consistent hash for same rows", () => {
    const rows: ExportRow[] = [
      {
        ts: "1000",
        bus: 0,
        busName: "Chassis",
        idHex: "0x123",
        dlc: 8,
        dir: "tx",
        data: "00 01 02 03 04 05 06 07",
      },
    ];

    const hash1 = computeExportChecksum(rows);
    const hash2 = computeExportChecksum(rows);

    expect(hash1).toBe(hash2);
  });

  it("produces different hash for different rows", () => {
    const rows1: ExportRow[] = [
      {
        ts: "1000",
        bus: 0,
        busName: "Chassis",
        idHex: "0x123",
        dlc: 8,
        dir: "tx",
        data: "00 01 02 03 04 05 06 07",
      },
    ];

    const rows2: ExportRow[] = [
      {
        ts: "1000",
        bus: 0,
        busName: "Chassis",
        idHex: "0x123",
        dlc: 8,
        dir: "tx",
        data: "FF 01 02 03 04 05 06 07", // Different data
      },
    ];

    const hash1 = computeExportChecksum(rows1);
    const hash2 = computeExportChecksum(rows2);

    expect(hash1).not.toBe(hash2);
  });

  it("handles empty rows", () => {
    const result = computeExportChecksum([]);

    expect(result).toBe(2166136261); // FNV offset basis
  });

  it("produces unsigned 32-bit integer", () => {
    const rows: ExportRow[] = Array.from({ length: 100 }, (_, i) => ({
      ts: `${i}`,
      bus: i % 3,
      busName: "Test",
      idHex: `0x${(i + 256).toString(16).toUpperCase()}`,
      dlc: 8,
      dir: "tx",
      data: "00 00 00 00 00 00 00 00",
    }));

    const hash = computeExportChecksum(rows);

    expect(hash).toBeGreaterThanOrEqual(0);
    expect(hash).toBeLessThanOrEqual(0xffffffff);
  });
});

describe("verifyExportIntegrity", () => {
  it("verifies rows with correct checksum and count", () => {
    const rows: ExportRow[] = [
      {
        ts: "1000",
        bus: 0,
        busName: "Chassis",
        idHex: "0x123",
        dlc: 8,
        dir: "tx",
        data: "00 01 02 03 04 05 06 07",
      },
    ];

    const checksum = computeExportChecksum(rows);
    const result = verifyExportIntegrity(rows, checksum, 1);

    expect(result).toBe(true);
  });

  it("rejects rows with wrong count", () => {
    const rows: ExportRow[] = [
      {
        ts: "1000",
        bus: 0,
        busName: "Chassis",
        idHex: "0x123",
        dlc: 8,
        dir: "tx",
        data: "00 01 02 03 04 05 06 07",
      },
    ];

    const checksum = computeExportChecksum(rows);
    const result = verifyExportIntegrity(rows, checksum, 5); // Wrong count

    expect(result).toBe(false);
  });

  it("rejects rows with wrong checksum", () => {
    const rows: ExportRow[] = [
      {
        ts: "1000",
        bus: 0,
        busName: "Chassis",
        idHex: "0x123",
        dlc: 8,
        dir: "tx",
        data: "00 01 02 03 04 05 06 07",
      },
    ];

    const result = verifyExportIntegrity(rows, 12345, 1); // Wrong checksum

    expect(result).toBe(false);
  });

  it("rejects when both count and checksum are wrong", () => {
    const rows: ExportRow[] = [];

    const result = verifyExportIntegrity(rows, 99999, 999);

    expect(result).toBe(false);
  });
});

describe("formatExportRowsAsCsv", () => {
  const baseMetadata: ExportMetadata & { exportedAt?: string; platform?: any } = {
    schemaVersion: "monitor-export.v1",
    dataset: {
      id: "known",
      label: "Known IDs",
      source: { vehicle: "Tesla", firmware: "Known ID Map", mcu: "generic", soc: "n/a" },
    },
    boardState: {} as any,
    bus: "all",
    textFilter: "",
    frameWindowSize: 50,
    frameSampleStep: 1,
    decodeEnabled: true,
    feedPaused: false,
    filteredFrames: 10,
    renderedFrames: 10,
    snapshots: 0,
    commandHistory: 0,
    notifications: 0,
    exportedAt: "2026-04-19T00:00:00Z",
    platform: { variant: "Model 3/Y", hardware: "HW3/HW4", board: "Tegra/Atom" },
  };

  it("formats rows as CSV with headers", () => {
    const rows: ExportRow[] = [
      {
        ts: "1000",
        bus: 0,
        busName: "Chassis",
        idHex: "0x123",
        dlc: 8,
        dir: "tx",
        data: "00 01 02 03 04 05 06 07",
      },
    ];

    const result = formatExportRowsAsCsv({ rows, metadata: baseMetadata });

    expect(result).toContain("ts,bus,busName,idHex,dlc,dir,data");
    expect(result).toContain(`"1000"`);
    expect(result).toContain(`"Chassis"`);
    expect(result).toContain(`"0x123"`);
  });

  it("includes metadata comments in CSV header", () => {
    const rows: ExportRow[] = [];

    const result = formatExportRowsAsCsv({ rows, metadata: baseMetadata });

    expect(result).toContain("# schema=monitor-export.v1");
    expect(result).toContain("# dataset=known");
    expect(result).toContain("# platform=Model 3/Y/HW3/HW4/Tegra/Atom");
  });

  it("escapes quotes in data fields", () => {
    const rows: ExportRow[] = [
      {
        ts: `1000`,
        bus: 0,
        busName: `Chas"sis`,
        idHex: "0x123",
        dlc: 8,
        dir: "tx",
        data: "00 01 02 03 04 05 06 07",
      },
    ];

    const result = formatExportRowsAsCsv({ rows, metadata: baseMetadata });

    expect(result).toContain(`"Chas""sis"`);
  });

  it("includes row count and checksum in metadata", () => {
    const rows: ExportRow[] = [
      {
        ts: "1000",
        bus: 0,
        busName: "Chassis",
        idHex: "0x123",
        dlc: 8,
        dir: "tx",
        data: "00 01 02 03 04 05 06 07",
      },
    ];

    const result = formatExportRowsAsCsv({ rows, metadata: baseMetadata });

    expect(result).toContain("# rows=1 checksum=");
    expect(result).toContain("bus=all filter=");
  });
});

describe("validateExportMode", () => {
  it("accepts valid export modes", () => {
    expect(validateExportMode("csv")).toBe(true);
    expect(validateExportMode("json")).toBe(true);
    expect(validateExportMode("raw-json")).toBe(true);
    expect(validateExportMode("raw-jsonl")).toBe(true);
    expect(validateExportMode("decoded")).toBe(true);
  });

  it("rejects invalid export modes", () => {
    expect(validateExportMode("xml")).toBe(false);
    expect(validateExportMode("yaml")).toBe(false);
    expect(validateExportMode("")).toBe(false);
    expect(validateExportMode("invalid-mode")).toBe(false);
  });
});

describe("buildExportSummary", () => {
  it("creates JSON export summary", () => {
    const result = buildExportSummary({
      rowCount: 100,
      checksum: 12345,
      valid: true,
      mode: "json",
      schemaVersion: "monitor-export.v1",
    });

    expect(result).toContain("100");
    expect(result).toContain("JSON");
    expect(result).toContain("12345");
    expect(result).toContain("valid=true");
  });

  it("creates CSV export summary", () => {
    const result = buildExportSummary({
      rowCount: 50,
      checksum: 99999,
      valid: false,
      mode: "csv",
      schemaVersion: "monitor-export.v1",
    });

    expect(result).toContain("50");
    expect(result).toContain("CSV");
    expect(result).toContain("valid=false");
  });

  it("handles different modes", () => {
    const result = buildExportSummary({
      rowCount: 1000,
      checksum: 55555,
      valid: true,
      mode: "raw-jsonl",
      schemaVersion: "v2",
    });

    expect(result).toContain("RAW-JSONL");
    expect(result).toContain("v2");
  });
});

describe("buildExportHistoryEntry", () => {
  it("creates valid history entry for export", () => {
    const result = buildExportHistoryEntry({
      command: "monitor:export:json",
      rowCount: 50,
      checksum: 12345,
      schemaVersion: "monitor-export.v1",
      valid: true,
    });

    expect(result.command).toBe("monitor:export:json");
    expect(result.ok).toBe(true);
    expect(result.response).toContain("rows=50");
    expect(result.response).toContain("checksum=12345");
  });

  it("marks invalid exports in history", () => {
    const result = buildExportHistoryEntry({
      command: "monitor:export:csv",
      rowCount: 100,
      checksum: 99999,
      schemaVersion: "v1",
      valid: false,
    });

    expect(result.ok).toBe(false);
  });

  it("includes schema version in response", () => {
    const result = buildExportHistoryEntry({
      command: "test:export",
      rowCount: 0,
      checksum: 0,
      schemaVersion: "custom.v2",
      valid: true,
    });

    expect(result.response).toContain("schema=custom.v2");
  });
});
