import {
  addFrameSnapshot,
  addHistoryEntry,
  buildFrameSnapshot,
  buildHistoryEntry,
  filterHistoryByCommand,
  getHistoryStats,
  getHistorySuccessRate,
  getRecentHistory,
  pushHistory,
  saveFrameSnapshot,
  type FrameSnapshot,
  type HistoryEntry,
} from "../../src/state/monitorSessionHistory";

describe("buildHistoryEntry", () => {
  it("creates history entry with required fields", () => {
    const result = buildHistoryEntry({
      command: "unlock_doors",
      ok: true,
      response: "Door unlocked",
    });

    expect(result.command).toBe("unlock_doors");
    expect(result.ok).toBe(true);
    expect(result.response).toBe("Door unlocked");
    expect(result.id).toBeDefined();
    expect(result.ts).toBeDefined();
  });

  it("generates unique IDs", () => {
    const entry1 = buildHistoryEntry({
      command: "cmd1",
      ok: true,
      response: "resp1",
    });
    const entry2 = buildHistoryEntry({
      command: "cmd2",
      ok: false,
      response: "resp2",
    });

    expect(entry1.id).not.toBe(entry2.id);
  });

  it("records timestamp", () => {
    const before = Date.now();
    const entry = buildHistoryEntry({
      command: "test",
      ok: true,
      response: "ok",
    });
    const after = Date.now();

    expect(entry.ts).toBeGreaterThanOrEqual(before);
    expect(entry.ts).toBeLessThanOrEqual(after + 10);
  });
});

describe("addHistoryEntry", () => {
  it("adds entry to beginning of history", () => {
    const existing: HistoryEntry[] = [
      {
        id: "1",
        ts: 1000,
        command: "cmd1",
        ok: true,
        response: "ok1",
      },
    ];

    const newEntry: HistoryEntry = {
      id: "2",
      ts: 2000,
      command: "cmd2",
      ok: false,
      response: "ok2",
    };

    const result = addHistoryEntry({ current: existing, entry: newEntry });

    expect(result[0]).toEqual(newEntry);
    expect(result[1]).toEqual(existing[0]);
    expect(result.length).toBe(2);
  });

  it("respects max length limit", () => {
    const existing = Array.from({ length: 20 }, (_, i) => ({
      id: `${i}`,
      ts: i * 1000,
      command: `cmd${i}`,
      ok: true,
      response: `ok${i}`,
    }));

    const newEntry: HistoryEntry = {
      id: "new",
      ts: 30000,
      command: "new_cmd",
      ok: true,
      response: "new_ok",
    };

    const result = addHistoryEntry({
      current: existing,
      entry: newEntry,
      maxLength: 20,
    });

    expect(result.length).toBe(20);
    expect(result[0]).toEqual(newEntry);
  });

  it("defaults to max length of 20", () => {
    const existing = Array.from({ length: 20 }, (_, i) => ({
      id: `${i}`,
      ts: i,
      command: `c${i}`,
      ok: true,
      response: `r${i}`,
    }));

    const newEntry: HistoryEntry = {
      id: "new",
      ts: 999,
      command: "new",
      ok: true,
      response: "new",
    };

    const result = addHistoryEntry({ current: existing, entry: newEntry });

    expect(result.length).toBe(20);
  });
});

describe("pushHistory", () => {
  it("builds and adds entry in one operation", () => {
    const current: HistoryEntry[] = [];

    const result = pushHistory({
      current,
      command: "test_cmd",
      ok: true,
      response: "test_response",
    });

    expect(result.length).toBe(1);
    expect(result[0].command).toBe("test_cmd");
    expect(result[0].ok).toBe(true);
  });

  it("respects max length", () => {
    const current = Array.from({ length: 5 }, (_, i) => ({
      id: `${i}`,
      ts: i,
      command: `cmd${i}`,
      ok: true,
      response: `resp${i}`,
    }));

    const result = pushHistory({
      current,
      command: "new",
      ok: false,
      response: "error",
      maxLength: 5,
    });

    expect(result.length).toBe(5);
  });
});

describe("buildFrameSnapshot", () => {
  it("creates frame snapshot with current state", () => {
    const result = buildFrameSnapshot({
      busFilter: "all",
      frameFilter: "0x123",
      frameCount: 100,
    });

    expect(result.busFilter).toBe("all");
    expect(result.frameFilter).toBe("0x123");
    expect(result.frameCount).toBe(100);
    expect(result.id).toBeDefined();
    expect(result.ts).toBeDefined();
  });

  it("trims frame filter", () => {
    const result = buildFrameSnapshot({
      busFilter: "all",
      frameFilter: "  0x456  ",
      frameCount: 50,
    });

    expect(result.frameFilter).toBe("0x456");
  });

  it("generates unique snapshot IDs", () => {
    const snap1 = buildFrameSnapshot({
      busFilter: "all",
      frameFilter: "filter1",
      frameCount: 10,
    });
    const snap2 = buildFrameSnapshot({
      busFilter: "all",
      frameFilter: "filter2",
      frameCount: 20,
    });

    expect(snap1.id).not.toBe(snap2.id);
  });
});

describe("addFrameSnapshot", () => {
  it("adds snapshot to beginning of list", () => {
    const existing: FrameSnapshot[] = [
      {
        id: "1",
        ts: 1000,
        frameCount: 10,
        busFilter: "all",
        frameFilter: "old",
      },
    ];

    const newSnapshot: FrameSnapshot = {
      id: "2",
      ts: 2000,
      frameCount: 20,
      busFilter: "all",
      frameFilter: "new",
    };

    const result = addFrameSnapshot({
      current: existing,
      snapshot: newSnapshot,
    });

    expect(result[0]).toEqual(newSnapshot);
    expect(result[1]).toEqual(existing[0]);
  });

  it("respects max length", () => {
    const existing = Array.from({ length: 20 }, (_, i) => ({
      id: `${i}`,
      ts: i * 1000,
      frameCount: i,
      busFilter: "all",
      frameFilter: `filter${i}`,
    }));

    const newSnapshot: FrameSnapshot = {
      id: "new",
      ts: 999999,
      frameCount: 999,
      busFilter: "all",
      frameFilter: "new",
    };

    const result = addFrameSnapshot({
      current: existing,
      snapshot: newSnapshot,
      maxLength: 20,
    });

    expect(result.length).toBe(20);
  });
});

describe("saveFrameSnapshot", () => {
  it("builds and adds snapshot in one operation", () => {
    const current: FrameSnapshot[] = [];

    const result = saveFrameSnapshot({
      current,
      busFilter: "1",
      frameFilter: "0x789",
      frameCount: 150,
    });

    expect(result.length).toBe(1);
    expect(result[0].busFilter).toBe("1");
    expect(result[0].frameFilter).toBe("0x789");
    expect(result[0].frameCount).toBe(150);
  });

  it("respects max length", () => {
    const current = Array.from({ length: 10 }, (_, i) => ({
      id: `${i}`,
      ts: i,
      frameCount: i,
      busFilter: "all",
      frameFilter: `old${i}`,
    }));

    const result = saveFrameSnapshot({
      current,
      busFilter: "all",
      frameFilter: "new",
      frameCount: 999,
      maxLength: 10,
    });

    expect(result.length).toBe(10);
  });
});

describe("filterHistoryByCommand", () => {
  const history: HistoryEntry[] = [
    {
      id: "1",
      ts: 1000,
      command: "unlock_doors",
      ok: true,
      response: "ok",
    },
    { id: "2", ts: 2000, command: "lock_doors", ok: true, response: "ok" },
    { id: "3", ts: 3000, command: "honk", ok: false, response: "error" },
  ];

  it("filters by command name substring", () => {
    const result = filterHistoryByCommand({
      history,
      query: "doors",
    });

    expect(result.length).toBe(2);
    expect(result.every((e) => e.command.includes("doors"))).toBe(true);
  });

  it("is case-insensitive", () => {
    const result = filterHistoryByCommand({
      history,
      query: "UNLOCK",
    });

    expect(result.length).toBe(1);
    expect(result[0].command).toBe("unlock_doors");
  });

  it("returns all if query is empty", () => {
    const result = filterHistoryByCommand({
      history,
      query: "",
    });

    expect(result.length).toBe(3);
  });

  it("returns empty array for no matches", () => {
    const result = filterHistoryByCommand({
      history,
      query: "nonexistent",
    });

    expect(result.length).toBe(0);
  });
});

describe("getHistorySuccessRate", () => {
  it("calculates success rate", () => {
    const history: HistoryEntry[] = [
      { id: "1", ts: 1000, command: "c1", ok: true, response: "ok" },
      { id: "2", ts: 2000, command: "c2", ok: true, response: "ok" },
      { id: "3", ts: 3000, command: "c3", ok: false, response: "error" },
    ];

    const result = getHistorySuccessRate({ history });

    expect(result).toBe(67); // 2/3 * 100 = 66.67, rounded to 67
  });

  it("returns 100 for all successful", () => {
    const history: HistoryEntry[] = [
      { id: "1", ts: 1000, command: "c1", ok: true, response: "ok" },
      { id: "2", ts: 2000, command: "c2", ok: true, response: "ok" },
    ];

    const result = getHistorySuccessRate({ history });

    expect(result).toBe(100);
  });

  it("returns 0 for all failed", () => {
    const history: HistoryEntry[] = [
      { id: "1", ts: 1000, command: "c1", ok: false, response: "error" },
    ];

    const result = getHistorySuccessRate({ history });

    expect(result).toBe(0);
  });

  it("returns 0 for empty history", () => {
    const result = getHistorySuccessRate({ history: [] });

    expect(result).toBe(0);
  });
});

describe("getRecentHistory", () => {
  const history = Array.from({ length: 20 }, (_, i) => ({
    id: `${i}`,
    ts: i * 1000,
    command: `cmd${i}`,
    ok: i % 2 === 0,
    response: `resp${i}`,
  }));

  it("returns recent entries", () => {
    const result = getRecentHistory({ history, count: 5 });

    expect(result.length).toBe(5);
    expect(result[0].id).toBe("0");
    expect(result[4].id).toBe("4");
  });

  it("defaults to 10 entries", () => {
    const result = getRecentHistory({ history });

    expect(result.length).toBe(10);
  });

  it("returns all if count exceeds history length", () => {
    const result = getRecentHistory({ history, count: 100 });

    expect(result.length).toBe(20);
  });
});

describe("getHistoryStats", () => {
  it("computes aggregate statistics", () => {
    const history: HistoryEntry[] = [
      { id: "1", ts: 1000, command: "c1", ok: true, response: "ok" },
      { id: "2", ts: 2000, command: "c2", ok: true, response: "ok" },
      { id: "3", ts: 3000, command: "c3", ok: false, response: "error" },
    ];

    const result = getHistoryStats({ history });

    expect(result.total).toBe(3);
    expect(result.successful).toBe(2);
    expect(result.failed).toBe(1);
    expect(result.successRate).toBe(67);
  });

  it("handles empty history", () => {
    const result = getHistoryStats({ history: [] });

    expect(result.total).toBe(0);
    expect(result.successful).toBe(0);
    expect(result.failed).toBe(0);
    expect(result.successRate).toBe(0);
  });

  it("handles all failures", () => {
    const history: HistoryEntry[] = [
      { id: "1", ts: 1000, command: "c1", ok: false, response: "error" },
      { id: "2", ts: 2000, command: "c2", ok: false, response: "error" },
    ];

    const result = getHistoryStats({ history });

    expect(result.total).toBe(2);
    expect(result.successful).toBe(0);
    expect(result.failed).toBe(2);
    expect(result.successRate).toBe(0);
  });
});
