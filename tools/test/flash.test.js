import fs from "node:fs";
import os from "node:os";
import path from "node:path";

import { describe, expect, it } from "@jest/globals";

import { resolveFlashImagePlan } from "../commands/flash.js";

describe("resolveFlashImagePlan", () => {
	it("treats named release images as merged flash images at 0x0", () => {
		const imagePath = path.join("build", "firmware", "esp32_wifi.bin");
		const plan = resolveFlashImagePlan(imagePath, null);

		expect(plan.kind).toBe("merged-image");
		expect(plan.segments).toEqual([{ address: "0x0", file: path.resolve(imagePath) }]);
	});

	it("builds the full ESP32 layout for raw PlatformIO firmware.bin output", () => {
		const tempDir = fs.mkdtempSync(path.join(os.tmpdir(), "tcm-flash-"));
		const bootloader = path.join(tempDir, "bootloader.bin");
		const partitions = path.join(tempDir, "partitions.bin");
		const firmware = path.join(tempDir, "firmware.bin");
		const bootApp0 = path.join(tempDir, "boot_app0.bin");

		for (const file of [bootloader, partitions, firmware, bootApp0]) {
			fs.writeFileSync(file, "test");
		}

		const plan = resolveFlashImagePlan(firmware, bootApp0);

		expect(plan.kind).toBe("platformio-build");
		expect(plan.segments).toEqual([
			{ address: "0x1000", file: bootloader },
			{ address: "0x8000", file: partitions },
			{ address: "0xe000", file: bootApp0 },
			{ address: "0x10000", file: firmware },
		]);

		fs.rmSync(tempDir, { recursive: true, force: true });
	});
});
