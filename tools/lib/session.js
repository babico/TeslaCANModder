/** Board serial session — manages connection, message queue, and waiters. */

import { createInterface } from "node:readline";

/**
 * Open a port with `autoOpen:false` so the callback is attached via `open()`.
 * A constructor callback may fire synchronously and hit the `sp` temporal
 * dead zone; going through `open()` keeps the reference initialized first.
 */
export function openSerialWith(SerialPort, port, baud) {
	const sp = new SerialPort({ path: port, baudRate: baud, autoOpen: false });
	return new Promise((resolve, reject) => {
		sp.open((err) => (err ? reject(err) : resolve(sp)));
	});
}

export async function openSerial(port, baud) {
	try {
		const { SerialPort } = await import("serialport");
		return await openSerialWith(SerialPort, port, baud);
	} catch (err) {
		if (err.code === "ERR_MODULE_NOT_FOUND" || err.code === "MODULE_NOT_FOUND") {
			throw new Error(
				"'serialport' package not found.\nInstall it with:\n\n  npm install -w @teslacanmodder/tools serialport\n",
			);
		}
		throw err;
	}
}

export class BoardSession {
	constructor(sp, timeoutMs = 3000, onWriteError = null) {
		this._sp = sp;
		this._defaultTimeout = timeoutMs;
		this._onWriteError = onWriteError;
		this._queue = [];
		this._waiters = [];
		this._allMessages = [];

		this._rl = createInterface({ input: sp, crlfDelay: Infinity });
		this._rl.on("line", (raw) => this._onRawLine(raw));
	}

	_onRawLine(rawLine) {
		const line = rawLine.replace(/\r/g, "").trim();
		if (!line) return;

		let msg = null;
		let type = "text";
		let parseError = null;

		const start = line.indexOf("{");
		const end = line.lastIndexOf("}");
		if (start >= 0 && end > start) {
			try {
				msg = JSON.parse(line.slice(start, end + 1));
				type = msg.t || "unknown";
			} catch (error) {
				type = "parse-error";
				parseError = error instanceof Error ? error.message : String(error);
			}
		}

		const entry = { raw: line, msg, type };
		if (parseError) entry.error = parseError;
		this._allMessages.push(entry);
		this._dispatch(entry);
	}

	_dispatch(entry) {
		for (let i = 0; i < this._waiters.length; i++) {
			const w = this._waiters[i];
			if (w.predicate(entry)) {
				this._waiters.splice(i, 1);
				clearTimeout(w.timer);
				w.resolve(entry);
				return;
			}
		}
		this._queue.push(entry);
	}

	waitFor(predicate, timeoutMs) {
		const ms = timeoutMs ?? this._defaultTimeout;
		const idx = this._queue.findIndex(predicate);
		if (idx >= 0) return Promise.resolve(this._queue.splice(idx, 1)[0]);

		return new Promise((resolve) => {
			const timer = setTimeout(() => {
				const pos = this._waiters.findIndex((w) => w.timer === timer);
				if (pos >= 0) this._waiters.splice(pos, 1);
				resolve(null);
			}, ms);
			this._waiters.push({ predicate, resolve, timer });
		});
	}

	waitForType(t, timeoutMs) {
		return this.waitFor((e) => e.msg?.t === t, timeoutMs);
	}

	waitForAck(cmd, timeoutMs) {
		return this.waitFor((e) => e.msg?.t === "ack" && e.msg?.cmd === cmd, timeoutMs);
	}

	drainType(t) {
		const out = this._queue.filter((e) => e.msg?.t === t);
		this._queue = this._queue.filter((e) => e.msg?.t !== t);
		return out;
	}

	/**
	 * Send a command to the board.
	 *
	 * Firmware expects JSON-RPC envelopes: `{"cmd":"<name>"}` + newline.
	 * Plain text inputs are auto-wrapped for convenience. Pass
	 * `{raw: true}` to send the bytes verbatim (escape hatch for protocol
	 * extensions that are not yet mapped to a `cmd` field).
	 *
	 * @param {string} command Either a plain command name (e.g. "status")
	 *   or a complete JSON object. When the string starts with `{`, it is
	 *   sent untouched.
	 * @param {{raw?: boolean}} [opts] Send `command` verbatim when `raw:true`.
	 */
	send(command, opts = {}) {
		if (this._sp.writable === false) {
			throw new Error("Serial port is not writable; command dropped");
		}
		let line;
		if (opts.raw) {
			line = command;
		} else if (command.startsWith("{")) {
			line = command;
		} else {
			line = JSON.stringify({ cmd: command });
		}
		this._sp.write(`${line}\n`, (error) => {
			if (!error) return;
			if (typeof this._onWriteError === "function") {
				this._onWriteError(error);
			} else {
				console.error(`[session] serial write failed: ${error.message}`);
			}
		});
	}

	messages() {
		return this._allMessages;
	}

	close() {
		this._rl?.close();
		if (this._sp.isOpen) this._sp.close();
	}
}
