import {
  buildDiagnosticsEvents,
  filterDiagnosticsEvents,
  type DiagnosticsEvent,
} from "../../src/state/monitorDiagnostics";

describe("monitorDiagnostics", () => {
  it("builds sorted diagnostics events from lifecycle, history, board, and snapshots", () => {
    const events = buildDiagnosticsEvents({
      commandLifecycle: [
        {
          id: "pending-1",
          command: "status",
          status: "pending",
          startedAt: 100,
        },
        {
          id: "failed-1",
          command: "fsd",
          status: "failed",
          startedAt: 80,
          finishedAt: 95,
          detail: "timeout",
        },
      ],
      history: [
        {
          id: "h1",
          ts: 90,
          command: "horn",
          ok: true,
          response: "ok",
        },
      ],
      boardMessages: [
        { id: 1, type: "info", text: "board up", ts: "10:00:00" },
        { id: 2, type: "error", text: "fault", ts: "10:00:01" },
      ],
      frameSnapshots: [
        {
          id: "s1",
          ts: 85,
          frameCount: 120,
          busFilter: "1",
          frameFilter: "0x123",
        },
      ],
      formatTime: (epochMs) => `T${epochMs}`,
      nowMs: 1000,
    });

    expect(events[0]).toMatchObject({
      id: "board-1-0",
      ts: 1000,
      tsLabel: "10:00:00",
      category: "board",
      summary: "board up",
      detail: "INFO",
      ok: true,
    });

    expect(events.find((entry) => entry.id === "lifecycle-failed-1")).toMatchObject({
      tsLabel: "T95",
      summary: "fsd · FAILED",
      detail: "15ms · timeout",
      ok: false,
    });

    expect(events.find((entry) => entry.id === "snap-s1")).toMatchObject({
      summary: "120 frames captured",
      detail: "bus=1 filter=0x123",
      ok: true,
    });

    expect(events.map((entry) => entry.ts)).toEqual([
      1000,
      999,
      100,
      95,
      90,
      85,
    ]);
  });

  it("filters diagnostics by category and query", () => {
    const events: DiagnosticsEvent[] = [
      {
        id: "1",
        ts: 3,
        tsLabel: "T3",
        category: "command",
        summary: "Result · lock",
        detail: "OK",
        ok: true,
      },
      {
        id: "2",
        ts: 2,
        tsLabel: "T2",
        category: "board",
        summary: "fault",
        detail: "ERROR",
        ok: false,
      },
      {
        id: "3",
        ts: 1,
        tsLabel: "T1",
        category: "snapshot",
        summary: "10 frames captured",
        detail: "bus=all",
        ok: true,
      },
    ];

    expect(
      filterDiagnosticsEvents({ events, category: "board", query: "" })
    ).toEqual([events[1]]);

    expect(
      filterDiagnosticsEvents({ events, category: "all", query: "lock" })
    ).toEqual([events[0]]);

    expect(
      filterDiagnosticsEvents({ events, category: "all", query: "error" })
    ).toEqual([events[1]]);
  });

  it("respects maxEvents cap", () => {
    const events = buildDiagnosticsEvents({
      commandLifecycle: [],
      history: Array.from({ length: 5 }, (_, idx) => ({
        id: `h${idx}`,
        ts: idx,
        command: `cmd-${idx}`,
        ok: true,
        response: "ok",
      })),
      boardMessages: [],
      frameSnapshots: [],
      formatTime: (epochMs) => `${epochMs}`,
      maxEvents: 3,
    });

    expect(events).toHaveLength(3);
    expect(events.map((entry) => entry.id)).toEqual(["history-h4", "history-h3", "history-h2"]);
  });
});
