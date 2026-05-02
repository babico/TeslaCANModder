import { buildExportProvenance } from "../../src/state/monitorExport";

describe("monitorExport", () => {
	it("builds export provenance with platform, filters, and session summary", () => {
		const boardState = {
			variant: "hw4",
			hardware: "ESP32",
			board: "esp32",
			driver: "twai",
			frames: [
				{
					key: "newest",
					id: 0x123,
					dir: "rx",
					bus: 1,
					busName: "Vehicle",
					dlc: 8,
					data: "00 11 22 33 44 55 66 77",
					ts: "10:00:01",
				},
				{
					key: "oldest",
					id: 0x100,
					dir: "tx",
					bus: 0,
					busName: "Chassis",
					dlc: 8,
					data: "AA BB CC DD EE FF 00 11",
					ts: "10:00:00",
				},
			],
			messages: [
				{ id: 1, type: "info", text: "ok", ts: "10:00:02" },
				{ id: 2, type: "error", text: "warn", ts: "10:00:03" },
			],
		} as any;

		const provenance = buildExportProvenance({
			schemaVersion: "monitor-export.v1",
			dataset: {
				id: "known",
				label: "Known IDs",
				source: { vehicle: "Tesla", firmware: "Known", mcu: "generic", soc: "n/a" },
			},
			boardState,
			bus: "1",
			textFilter: " 0x123 ",
			frameWindowSize: 100,
			frameSampleStep: 2,
			decodeEnabled: true,
			feedPaused: false,
			filteredFrames: 20,
			renderedFrames: 10,
			snapshots: 4,
			commandHistory: 7,
			notifications: 2,
		});

		expect(provenance.schemaVersion).toBe("monitor-export.v1");
		expect(provenance.dataset.id).toBe("known");
		expect(provenance.platform).toEqual({
			variant: "hw4",
			hardware: "ESP32",
			board: "esp32",
			driver: "twai",
		});
		expect(provenance.filters).toEqual({
			bus: "1",
			text: "0x123",
			frameWindowSize: 100,
			frameSampleStep: 2,
			decodeEnabled: true,
			feedPaused: false,
		});
		expect(provenance.sessionSummary).toEqual({
			totalFrames: 2,
			filteredFrames: 20,
			renderedFrames: 10,
			snapshots: 4,
			commandHistory: 7,
			notifications: 2,
			timeframe: {
				first: "10:00:00",
				last: "10:00:01",
			},
		});
		expect(typeof provenance.exportedAt).toBe("string");
	});
});
