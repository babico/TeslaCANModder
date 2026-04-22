import type { IBlePeripheral, IBluetoothSerialPort, ISerialPort } from "./transport";

const BLE_NUS_SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
const BLE_NUS_RX_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
const BLE_NUS_TX_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";

type WebSerialLikePort = {
	open(options: { baudRate: number }): Promise<void>;
	close(): Promise<void>;
	readable?: {
		getReader(): {
			read(): Promise<{ value?: Uint8Array; done: boolean }>;
			releaseLock(): void;
		};
	} | null;
	writable?: {
		getWriter(): {
			write(data: Uint8Array): Promise<void>;
			releaseLock(): void;
		};
	} | null;
};

type WebSerialNavigator = Navigator & {
	serial?: {
		requestPort(): Promise<WebSerialLikePort>;
	};
};

type WebBluetoothCharacteristic = {
	startNotifications(): Promise<void>;
	addEventListener(name: "characteristicvaluechanged", listener: (event: Event) => void): void;
	writeValue(value: BufferSource): Promise<void>;
	writeValueWithoutResponse?(value: BufferSource): Promise<void>;
};

type WebBluetoothService = {
	getCharacteristic(uuid: string): Promise<WebBluetoothCharacteristic>;
};

type WebBluetoothServer = {
	connected?: boolean;
	getPrimaryService(uuid: string): Promise<WebBluetoothService>;
};

type WebBluetoothDevice = {
	gatt?: {
		connected?: boolean;
		connect(): Promise<WebBluetoothServer>;
		disconnect?(): void;
	};
};

type WebBluetoothNavigator = Navigator & {
	bluetooth?: {
		requestDevice(options: {
			filters: Array<{ services: string[] }>;
			optionalServices?: string[];
		}): Promise<WebBluetoothDevice>;
	};
};

function getWebSerialApi(): NonNullable<WebSerialNavigator["serial"]> {
	if (typeof navigator === "undefined") {
		throw new Error(
			"Serial transport is not available in this native runtime. Use REST API here, or open the web client to use Web Serial.",
		);
	}

	const serialApi = (navigator as WebSerialNavigator).serial;
	if (!serialApi) {
		throw new Error(
			"Web Serial is not available in this runtime. Use Chrome or Edge, or switch back to REST API.",
		);
	}

	return serialApi;
}

function getWebBluetoothApi(): NonNullable<WebBluetoothNavigator["bluetooth"]> {
	if (typeof navigator === "undefined") {
		throw new Error(
			"BLE transport is not available in this native runtime. Use REST API here, or open the web client to use Web Bluetooth.",
		);
	}

	const bluetoothApi = (navigator as WebBluetoothNavigator).bluetooth;
	if (!bluetoothApi) {
		throw new Error(
			"Web Bluetooth is not available in this runtime. BLE transport currently works only in browser builds that expose navigator.bluetooth.",
		);
	}

	return bluetoothApi;
}

function normalizeCommandLine(command: string): string {
	return command.endsWith("\n") ? command : `${command}\n`;
}

class WebSerialPortAdapter implements ISerialPort, IBluetoothSerialPort {
	private readonly port: WebSerialLikePort;
	private readonly baudRate: number;
	private opened = false;
	private readBuffer = "";

	constructor(port: WebSerialLikePort, baudRate = 115200) {
		this.port = port;
		this.baudRate = baudRate;
		this.opened = Boolean(this.port.readable && this.port.writable);
	}

	get isOpen(): boolean {
		return this.opened;
	}

	async close(): Promise<void> {
		if (!this.opened) {
			return;
		}

		await this.port.close();
		this.opened = false;
		this.readBuffer = "";
	}

	private async ensureOpen(): Promise<void> {
		if (this.opened) {
			return;
		}

		try {
			await this.port.open({ baudRate: this.baudRate });
			this.opened = true;
		} catch (error) {
			const message = error instanceof Error ? error.message : String(error);
			// Browser serial state can be shared across UI flows; if already open,
			// treat it as success and continue using the existing handle.
			if (message.toLowerCase().includes("already open")) {
				this.opened = true;
				return;
			}
			throw error;
		}
	}

	async writeLine(line: string): Promise<string> {
		await this.ensureOpen();

		if (!this.port.writable || !this.port.readable) {
			throw new Error("Selected serial port does not expose readable/writable streams.");
		}

		const writer = this.port.writable.getWriter();
		try {
			await writer.write(new TextEncoder().encode(normalizeCommandLine(line)));
		} finally {
			writer.releaseLock();
		}

		const reader = this.port.readable.getReader();
		const decoder = new TextDecoder();
		const candidateLines: string[] = [];
		const startedAt = Date.now();

		try {
			while (Date.now() - startedAt < 5000) {
				const readResult = await Promise.race([
					reader.read(),
					new Promise<{ value?: Uint8Array; done: boolean }>((_, reject) => {
						setTimeout(() => reject(new Error("Serial response timed out.")), 5000);
					}),
				]);

				const { value, done } = readResult;
				if (done) {
					break;
				}

				if (!value || value.length === 0) {
					continue;
				}

				this.readBuffer += decoder.decode(value, { stream: true });

				while (true) {
					const lineBreakIndex = this.readBuffer.search(/\r?\n/);
					if (lineBreakIndex < 0) {
						break;
					}

					const rawLine = this.readBuffer.slice(0, lineBreakIndex);
					const nextOffset =
						this.readBuffer[lineBreakIndex] === "\r" &&
						this.readBuffer[lineBreakIndex + 1] === "\n"
							? lineBreakIndex + 2
							: lineBreakIndex + 1;
					this.readBuffer = this.readBuffer.slice(nextOffset);

					const normalized = rawLine.trim();
					if (normalized.length > 0) {
						candidateLines.push(normalized);
					}
				}

				if (candidateLines.length > 0) {
					break;
				}
			}
		} finally {
			reader.releaseLock();
		}

		if (candidateLines.length === 0) {
			return "";
		}

		// Prefer full JSON payload lines to avoid returning partial fragments.
		const jsonLine = candidateLines.find((entry) => {
			if (!(entry.startsWith("{") || entry.startsWith("["))) {
				return false;
			}

			try {
				JSON.parse(entry);
				return true;
			} catch {
				return false;
			}
		});

		return jsonLine ?? candidateLines[0] ?? "";
	}
}

class WebBluetoothBlePeripheral implements IBlePeripheral {
	private readonly device: WebBluetoothDevice;
	private readonly rxCharacteristic: WebBluetoothCharacteristic;
	private readonly txCharacteristic: WebBluetoothCharacteristic;
	private readonly decoder = new TextDecoder();
	private notificationBuffer = "";
	private notificationTimer: ReturnType<typeof setTimeout> | null = null;
	private pendingResolve: ((value: string) => void) | null = null;
	private pendingReject: ((reason?: unknown) => void) | null = null;

	constructor(
		device: WebBluetoothDevice,
		rxCharacteristic: WebBluetoothCharacteristic,
		txCharacteristic: WebBluetoothCharacteristic,
	) {
		this.device = device;
		this.rxCharacteristic = rxCharacteristic;
		this.txCharacteristic = txCharacteristic;
		this.txCharacteristic.addEventListener("characteristicvaluechanged", (event) => {
			const target = event.target as { value?: DataView | null } | null;
			const view = target?.value;
			if (!view) {
				return;
			}

			const bytes = new Uint8Array(view.buffer, view.byteOffset, view.byteLength);
			this.notificationBuffer += this.decoder.decode(bytes, { stream: true });

			if (this.notificationBuffer.includes("\n")) {
				this.flushPending();
				return;
			}

			if (this.notificationTimer) {
				clearTimeout(this.notificationTimer);
			}

			this.notificationTimer = setTimeout(() => this.flushPending(), 120);
		});
	}

	get isConnected(): boolean {
		return this.device.gatt?.connected === true;
	}

	async disconnect(): Promise<void> {
		this.device.gatt?.disconnect?.();
	}

	private flushPending(): void {
		if (this.notificationTimer) {
			clearTimeout(this.notificationTimer);
			this.notificationTimer = null;
		}

		if (!this.pendingResolve) {
			this.notificationBuffer = "";
			return;
		}

		const payload = this.notificationBuffer.trim();
		this.notificationBuffer = "";

		const resolve = this.pendingResolve;
		this.pendingResolve = null;
		this.pendingReject = null;
		resolve(payload);
	}

	private async waitForNotification(): Promise<string> {
		if (this.pendingResolve || this.pendingReject) {
			throw new Error("BLE request already in flight.");
		}

		return new Promise<string>((resolve, reject) => {
			this.pendingResolve = resolve;
			this.pendingReject = reject;

			setTimeout(() => {
				if (!this.pendingReject) {
					return;
				}

				const timeoutReject = this.pendingReject;
				this.pendingResolve = null;
				this.pendingReject = null;
				this.notificationBuffer = "";
				timeoutReject(new Error("BLE response timed out."));
			}, 5000);
		});
	}

	private async writeCommand(command: string): Promise<string> {
		const payload = new TextEncoder().encode(normalizeCommandLine(command));
		const responsePromise = this.waitForNotification();

		if (this.rxCharacteristic.writeValueWithoutResponse) {
			await this.rxCharacteristic.writeValueWithoutResponse(payload);
		} else {
			await this.rxCharacteristic.writeValue(payload);
		}

		return responsePromise;
	}

	async writeAndRead(command: string): Promise<string> {
		if (!this.isConnected) {
			throw new Error("BLE peripheral is not connected.");
		}

		return this.writeCommand(command);
	}

	async readStatus(): Promise<string> {
		return this.writeAndRead("status");
	}
}

export async function requestRuntimeSerialPort(): Promise<ISerialPort & IBluetoothSerialPort> {
	const serialApi = getWebSerialApi();
	const port = await serialApi.requestPort();
	return new WebSerialPortAdapter(port);
}

export async function connectRuntimeBlePeripheral(): Promise<IBlePeripheral> {
	const bluetoothApi = getWebBluetoothApi();
	const device = await bluetoothApi.requestDevice({
		filters: [{ services: [BLE_NUS_SERVICE_UUID] }],
		optionalServices: [BLE_NUS_SERVICE_UUID],
	});

	if (!device.gatt) {
		throw new Error("Selected BLE device does not expose a GATT server.");
	}

	const server = await device.gatt.connect();
	const service = await server.getPrimaryService(BLE_NUS_SERVICE_UUID);
	const rxCharacteristic = await service.getCharacteristic(BLE_NUS_RX_UUID);
	const txCharacteristic = await service.getCharacteristic(BLE_NUS_TX_UUID);
	await txCharacteristic.startNotifications();

	return new WebBluetoothBlePeripheral(device, rxCharacteristic, txCharacteristic);
}
