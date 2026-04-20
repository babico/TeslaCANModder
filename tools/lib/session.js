/** Board serial session — manages connection, message queue, and waiters. */

import { createInterface } from "node:readline";

export async function openSerial(port, baud) {
	try {
		const { SerialPort } = await import("serialport");
		return await new Promise((resolve, reject) => {
			const sp = new SerialPort({ path: port, baudRate: baud }, (err) => {
				if (err) reject(err);
				else resolve(sp);
			});
		});
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
	constructor(sp, timeoutMs = 3000) {
		this._sp = sp;
		this._defaultTimeout = timeoutMs;
		this._queue = [];
		this._waiters = [];
		this._allMessages = [];

		const rl = createInterface({ input: sp, crlfDelay: Infinity });
		rl.on("line", (raw) => this._onRawLine(raw));
	}

	_onRawLine(rawLine) {
		const line = rawLine.replace(/\r/g, "").trim();
		if (!line) return;

		let msg = null;
		let type = "text";

		const start = line.indexOf("{");
		const end = line.lastIndexOf("}");
		if (start >= 0 && end > start) {
			try {
				msg = JSON.parse(line.slice(start, end + 1));
				type = msg.t || "unknown";
			} catch {
				type = "parse-error";
			}
		}

		const entry = { raw: line, msg, type };
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

	send(command) {
		if (this._sp.writable !== false) this._sp.write(`${command}\n`);
	}

	messages() {
		return this._allMessages;
	}

	close() {
		if (this._sp.isOpen) this._sp.close();
	}
}
