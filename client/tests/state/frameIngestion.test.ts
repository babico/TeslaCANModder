/**
 * B-05 Frame Ingestion Pipeline unit tests
 * Covers: buffer cap, eviction, windowing, sampling, pause/resume,
 *         bus/text filter, config clamping, stress throughput target.
 */

import {
  frameIngestionReducer,
  initialFrameIngestionState,
  selectVisibleFrames,
  selectIngestionStats,
  MAX_FRAME_BUFFER,
  DEFAULT_WINDOW_SIZE,
  type CanFrame,
  type FrameIngestionState,
  type FrameIngestionAction,
} from "../../src/state/frameIngestion";

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

function frame(id: string, bus = "vehicle", ts = 0, data = "DEADBEEF"): CanFrame {
  return { id, bus, ts, data };
}

function frames(count: number, bus = "vehicle"): CanFrame[] {
  return Array.from({ length: count }, (_, i) => frame(`${i}`, bus, i));
}

function dispatch(
  state: FrameIngestionState,
  ...actions: FrameIngestionAction[]
): FrameIngestionState {
  return actions.reduce(frameIngestionReducer, state);
}

// ---------------------------------------------------------------------------
// Initial state
// ---------------------------------------------------------------------------

describe("initialFrameIngestionState", () => {
  it("starts with empty buffer and zero counters", () => {
    const s = initialFrameIngestionState();
    expect(s.buffer).toHaveLength(0);
    expect(s.totalIngested).toBe(0);
    expect(s.totalEvicted).toBe(0);
  });

  it("accepts custom config overrides", () => {
    const s = initialFrameIngestionState({ windowSize: 10, paused: true });
    expect(s.config.windowSize).toBe(10);
    expect(s.config.paused).toBe(true);
  });
});

// ---------------------------------------------------------------------------
// INGEST_FRAMES
// ---------------------------------------------------------------------------

describe("INGEST_FRAMES", () => {
  it("prepends frames (newest-first order)", () => {
    const s = dispatch(
      initialFrameIngestionState(),
      { type: "INGEST_FRAMES", payload: { frames: [frame("a"), frame("b")] } },
      { type: "INGEST_FRAMES", payload: { frames: [frame("c")] } }
    );
    expect(s.buffer[0].id).toBe("c");
    expect(s.buffer[1].id).toBe("a");
    expect(s.buffer[2].id).toBe("b");
  });

  it("increments totalIngested correctly", () => {
    const s = dispatch(
      initialFrameIngestionState(),
      { type: "INGEST_FRAMES", payload: { frames: frames(10) } },
      { type: "INGEST_FRAMES", payload: { frames: frames(5) } }
    );
    expect(s.totalIngested).toBe(15);
  });

  it("caps buffer at maxBuffer and tracks evictions", () => {
    const cap = 20;
    const s = dispatch(
      initialFrameIngestionState({ maxBuffer: cap }),
      { type: "INGEST_FRAMES", payload: { frames: frames(30) } }
    );
    expect(s.buffer.length).toBe(cap);
    expect(s.totalEvicted).toBe(10);
    expect(s.totalIngested).toBe(30);
  });

  it("no-op for empty frame array", () => {
    const s0 = initialFrameIngestionState();
    const s1 = dispatch(s0, { type: "INGEST_FRAMES", payload: { frames: [] } });
    expect(s1).toBe(s0); // same reference — no mutation
  });
});

// ---------------------------------------------------------------------------
// SET_CONFIG
// ---------------------------------------------------------------------------

describe("SET_CONFIG", () => {
  it("updates config fields", () => {
    const s = dispatch(
      initialFrameIngestionState(),
      { type: "SET_CONFIG", payload: { windowSize: 100, busFilter: "body" } }
    );
    expect(s.config.windowSize).toBe(100);
    expect(s.config.busFilter).toBe("body");
  });

  it("clamps windowSize to [1, 200]", () => {
    const s1 = dispatch(
      initialFrameIngestionState(),
      { type: "SET_CONFIG", payload: { windowSize: 999 } }
    );
    expect(s1.config.windowSize).toBe(200);

    const s2 = dispatch(
      initialFrameIngestionState(),
      { type: "SET_CONFIG", payload: { windowSize: 0 } }
    );
    expect(s2.config.windowSize).toBe(1);
  });

  it("clamps sampleStep to [1, 20]", () => {
    const s1 = dispatch(
      initialFrameIngestionState(),
      { type: "SET_CONFIG", payload: { sampleStep: 100 } }
    );
    expect(s1.config.sampleStep).toBe(20);

    const s2 = dispatch(
      initialFrameIngestionState(),
      { type: "SET_CONFIG", payload: { sampleStep: 0 } }
    );
    expect(s2.config.sampleStep).toBe(1);
  });
});

// ---------------------------------------------------------------------------
// CLEAR_BUFFER
// ---------------------------------------------------------------------------

describe("CLEAR_BUFFER", () => {
  it("empties buffer but preserves counters", () => {
    const s = dispatch(
      initialFrameIngestionState(),
      { type: "INGEST_FRAMES", payload: { frames: frames(10) } },
      { type: "CLEAR_BUFFER" }
    );
    expect(s.buffer).toHaveLength(0);
    expect(s.totalIngested).toBe(10);
  });
});

// ---------------------------------------------------------------------------
// PAUSE / RESUME
// ---------------------------------------------------------------------------

describe("PAUSE / RESUME", () => {
  it("PAUSE sets paused=true", () => {
    const s = dispatch(initialFrameIngestionState(), { type: "PAUSE" });
    expect(s.config.paused).toBe(true);
  });

  it("RESUME sets paused=false", () => {
    const s = dispatch(
      initialFrameIngestionState({ paused: true }),
      { type: "RESUME" }
    );
    expect(s.config.paused).toBe(false);
  });
});

// ---------------------------------------------------------------------------
// selectVisibleFrames
// ---------------------------------------------------------------------------

describe("selectVisibleFrames", () => {
  it("returns empty array when paused", () => {
    const s = dispatch(
      initialFrameIngestionState({ paused: true }),
      { type: "INGEST_FRAMES", payload: { frames: frames(5) } }
    );
    expect(selectVisibleFrames(s)).toHaveLength(0);
  });

  it("applies windowSize", () => {
    const s = dispatch(
      initialFrameIngestionState({ windowSize: 3 }),
      { type: "INGEST_FRAMES", payload: { frames: frames(10) } }
    );
    expect(selectVisibleFrames(s)).toHaveLength(3);
  });

  it("applies bus filter", () => {
    const mixed = [
      frame("v1", "vehicle"),
      frame("b1", "body"),
      frame("v2", "vehicle"),
    ];
    const s = dispatch(
      initialFrameIngestionState({ busFilter: "body" }),
      { type: "INGEST_FRAMES", payload: { frames: mixed } }
    );
    const visible = selectVisibleFrames(s);
    expect(visible.every((f) => f.bus === "body")).toBe(true);
    expect(visible.length).toBe(1);
  });

  it("applies text filter on id, name, data", () => {
    const mixed = [
      { id: "0x123", bus: "vehicle", ts: 0, data: "ABCD", name: "SpeedMsg" },
      { id: "0x456", bus: "vehicle", ts: 1, data: "FEED", name: "TempMsg" },
    ];
    const s = dispatch(
      initialFrameIngestionState({ textFilter: "speed" }),
      { type: "INGEST_FRAMES", payload: { frames: mixed } }
    );
    const visible = selectVisibleFrames(s);
    expect(visible).toHaveLength(1);
    expect(visible[0].name).toBe("SpeedMsg");
  });

  it("applies sample step (every other frame)", () => {
    const s = dispatch(
      initialFrameIngestionState({ sampleStep: 2, windowSize: 10 }),
      { type: "INGEST_FRAMES", payload: { frames: frames(10) } }
    );
    const visible = selectVisibleFrames(s);
    expect(visible).toHaveLength(5); // 10 / 2
  });
});

// ---------------------------------------------------------------------------
// selectIngestionStats
// ---------------------------------------------------------------------------

describe("selectIngestionStats", () => {
  it("reports correct buffer utilization", () => {
    const s = dispatch(
      initialFrameIngestionState({ maxBuffer: 100 }),
      { type: "INGEST_FRAMES", payload: { frames: frames(40) } }
    );
    const stats = selectIngestionStats(s);
    expect(stats.bufferSize).toBe(40);
    expect(stats.bufferCapacity).toBe(100);
    expect(stats.bufferUtilization).toBeCloseTo(0.4);
    expect(stats.totalIngested).toBe(40);
    expect(stats.isPaused).toBe(false);
  });
});

// ---------------------------------------------------------------------------
// Stress throughput target
// ---------------------------------------------------------------------------

describe("stress test: throughput", () => {
  it(`ingests ${MAX_FRAME_BUFFER} frames in < 50 ms`, () => {
    const bigBatch = frames(MAX_FRAME_BUFFER);
    const s0 = initialFrameIngestionState();
    const start = Date.now();
    const s1 = frameIngestionReducer(s0, {
      type: "INGEST_FRAMES",
      payload: { frames: bigBatch },
    });
    const elapsed = Date.now() - start;
    expect(s1.buffer).toHaveLength(MAX_FRAME_BUFFER);
    expect(elapsed).toBeLessThan(50);
  }, 5000); // generous test timeout
});
