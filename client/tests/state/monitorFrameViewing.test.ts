import {
  filterFrames,
  selectVisibleFrames,
  applyFrameViewingPipeline,
  type CanFrame,
} from "../../src/state/monitorFrameViewing";

const frames: CanFrame[] = [
  {
    key: "f0",
    id: 0x100,
    dir: "rx",
    bus: 0,
    busName: "Vehicle",
    dlc: 8,
    data: "00 11 22 33 44 55 66 77",
    ts: "10:00:00",
  },
  {
    key: "f1",
    id: 0x123,
    dir: "rx",
    bus: 1,
    busName: "Body",
    dlc: 8,
    data: "AA BB CC DD EE FF 00 11",
    ts: "10:00:01",
  },
  {
    key: "f2",
    id: 0x200,
    dir: "tx",
    bus: 0,
    busName: "Vehicle",
    dlc: 8,
    data: "12 34 56 78 9A BC DE F0",
    ts: "10:00:02",
  },
  {
    key: "f3",
    id: 0x456,
    dir: "rx",
    bus: 2,
    busName: "Chassis",
    dlc: 8,
    data: "FF EE DD CC BB AA 99 88",
    ts: "10:00:03",
  },
];

describe("monitorFrameViewing", () => {
  describe("filterFrames", () => {
    it("returns all frames when busFilter is 'all'", () => {
      const result = filterFrames({
        frames,
        busFilter: "all",
        textFilter: "",
      });
      expect(result).toHaveLength(4);
    });

    it("filters frames by bus number", () => {
      const result = filterFrames({
        frames,
        busFilter: "1",
        textFilter: "",
      });
      expect(result).toHaveLength(1);
      expect(result[0].id).toBe(0x123);
    });

    it("filters frames by text (hex ID, case-insensitive)", () => {
      const result = filterFrames({
        frames,
        busFilter: "all",
        textFilter: "123",
      });
      expect(result).toHaveLength(1);
      expect(result[0].id).toBe(0x123);
    });

    it("filters frames by text (data hex)", () => {
      const result = filterFrames({
        frames,
        busFilter: "all",
        textFilter: "AA BB",
      });
      expect(result).toHaveLength(1);
      expect(result[0].key).toBe("f1");
    });

    it("combines bus and text filters", () => {
      const result = filterFrames({
        frames,
        busFilter: "0",
        textFilter: "200",
      });
      expect(result).toHaveLength(1);
      expect(result[0].id).toBe(0x200);
    });

    it("matches ID search case-insensitively", () => {
      const result = filterFrames({
        frames,
        busFilter: "all",
        textFilter: "123",
      });
      expect(result).toHaveLength(1);
      expect(result[0].id).toBe(0x123);
    });
  });

  describe("selectVisibleFrames", () => {
    it("applies window size", () => {
      const result = selectVisibleFrames({
        frames,
        windowSize: 2,
        sampleStep: 1,
      });
      expect(result).toHaveLength(2);
      expect(result[0].id).toBe(0x100);
      expect(result[1].id).toBe(0x123);
    });

    it("applies sample step (every Nth frame)", () => {
      const result = selectVisibleFrames({
        frames,
        windowSize: 4,
        sampleStep: 2,
      });
      expect(result).toHaveLength(2);
      expect(result[0].id).toBe(0x100);
      expect(result[1].id).toBe(0x200);
    });

    it("windows first, then samples", () => {
      const result = selectVisibleFrames({
        frames,
        windowSize: 3,
        sampleStep: 2,
      });
      expect(result).toHaveLength(2);
      expect(result.map((f) => f.id)).toEqual([0x100, 0x200]);
    });

    it("no-op when sampleStep is 1", () => {
      const result = selectVisibleFrames({
        frames,
        windowSize: 3,
        sampleStep: 1,
      });
      expect(result).toHaveLength(3);
    });
  });

  describe("applyFrameViewingPipeline", () => {
    it("combines filter, window, and sample in order", () => {
      const result = applyFrameViewingPipeline({
        frames,
        busFilter: "all",
        textFilter: "",
        windowSize: 3,
        sampleStep: 2,
      });
      expect(result).toHaveLength(2);
      expect(result.map((f) => f.id)).toEqual([0x100, 0x200]);
    });

    it("filters, then windows, then samples", () => {
      const result = applyFrameViewingPipeline({
        frames,
        busFilter: "0",
        textFilter: "",
        windowSize: 2,
        sampleStep: 1,
      });
      expect(result).toHaveLength(2);
      expect(result.map((f) => f.id)).toEqual([0x100, 0x200]);
    });

    it("returns empty when no frames match filter", () => {
      const result = applyFrameViewingPipeline({
        frames,
        busFilter: "2",
        textFilter: "no-match-xyz",
        windowSize: 10,
        sampleStep: 1,
      });
      expect(result).toHaveLength(0);
    });
  });
});
