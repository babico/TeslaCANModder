type WebSerialNavigator = Navigator & {
	serial?: {
		requestPort(): Promise<SerialPort>;
	};
};

type FlashLogger = (line: string) => void;

export interface WebEspFlashOptions {
	assetName: string;
	assetUrl: string;
	onLog?: FlashLogger;
	onProgress?: (written: number, total: number) => void;
	eraseAll?: boolean;
	baudRate?: number;
}

const DEFAULT_FLASH_BAUD = 460800;

function logLine(logger: FlashLogger | undefined, line: string): void {
	const trimmed = line.trim();
	if (trimmed.length === 0) {
		return;
	}
	logger?.(trimmed);
}

function getWebSerialApi(): NonNullable<WebSerialNavigator["serial"]> {
	const serialApi = (globalThis.navigator as WebSerialNavigator | undefined)?.serial;
	if (!serialApi) {
		throw new Error(
			"USB flashing requires the web client in Chrome or Edge with Web Serial enabled.",
		);
	}
	return serialApi;
}

export function supportsBrowserEspFlash(): boolean {
	return Boolean((globalThis.navigator as WebSerialNavigator | undefined)?.serial);
}

export async function flashMergedEspReleaseImage(options: WebEspFlashOptions): Promise<void> {
	const serialApi = getWebSerialApi();
	const response = await fetch(options.assetUrl, {
		headers: { Accept: "application/octet-stream" },
	});
	if (!response.ok) {
		throw new Error(`Failed to download ${options.assetName} (HTTP ${response.status}).`);
	}

	const imageBytes = new Uint8Array(await response.arrayBuffer());
	logLine(
		options.onLog,
		`Downloaded ${options.assetName} (${imageBytes.byteLength.toLocaleString()} bytes).`,
	);

	const port = await serialApi.requestPort();
	const module = await import("esptool-js");
	const transport = new module.Transport(port, false);
	const loader = new module.ESPLoader({
		transport,
		baudrate: options.baudRate ?? DEFAULT_FLASH_BAUD,
		debugLogging: false,
		terminal: {
			clean() {
				// No-op; the flasher screen owns the visible log buffer.
			},
			write(message: string) {
				logLine(options.onLog, message);
			},
			writeLine(message: string) {
				logLine(options.onLog, message);
			},
		},
	});

	try {
		logLine(options.onLog, "Connecting to ESP32 bootloader...");
		const chipName = await loader.main();
		logLine(options.onLog, `Connected: ${chipName}`);
		logLine(options.onLog, "Writing merged firmware image...");

		await loader.writeFlash({
			fileArray: [{ data: imageBytes, address: 0x0 }],
			flashMode: "keep",
			flashFreq: "keep",
			flashSize: "keep",
			eraseAll: options.eraseAll ?? false,
			compress: true,
			reportProgress(_fileIndex: number, written: number, total: number) {
				options.onProgress?.(written, total);
			},
		});

		await loader.after("hard_reset");
		logLine(options.onLog, "ESP32 reset requested.");
	} finally {
		await transport.disconnect().catch((err) => {
			console.warn("Flashing cleanup: transport disconnect failed", err);
			return undefined;
		});
	}
}
