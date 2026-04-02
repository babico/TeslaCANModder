export class STK500 {
  constructor(port) {
    this.port = port;
    this.reader = null;
    this.writer = null;
    this.buffer = new Uint8Array(0);
  }

  async connect() {
    await this.port.open({ baudRate: 115200, bufferSize: 2048 });

    // Toggle the control lines long enough for CH340 + Optiboot to reset reliably.
    await this.port.setSignals({ dataTerminalReady: false, requestToSend: false });
    await this.sleep(250);
    await this.port.setSignals({ dataTerminalReady: true, requestToSend: true });
    await this.sleep(250);

    this.reader = this.port.readable.getReader();
    this.writer = this.port.writable.getWriter();
    void this.readLoop();
  }

  async disconnect() {
    if (this.reader) {
      try {
        await this.reader.cancel();
      } finally {
        this.reader.releaseLock();
        this.reader = null;
      }
    }

    if (this.writer) {
      try {
        await this.writer.close();
      } finally {
        this.writer.releaseLock();
        this.writer = null;
      }
    }

    await this.port.close();
  }

  async readLoop() {
    try {
      while (this.reader) {
        const { value, done } = await this.reader.read();
        if (value) {
          const newBuffer = new Uint8Array(this.buffer.length + value.length);
          newBuffer.set(this.buffer);
          newBuffer.set(value, this.buffer.length);
          this.buffer = newBuffer;
        }

        if (done) {
          break;
        }
      }
    } catch (error) {
      console.warn('Reader error or closed:', error);
    }
  }

  async sendCommand(command, expectBytes = 2) {
    this.buffer = new Uint8Array(0);
    await this.writer.write(new Uint8Array(command));
    return this.waitForResponse(expectBytes, 1500);
  }

  async waitForResponse(length, timeoutMs) {
    const start = Date.now();
    while (this.buffer.length < length) {
      if (Date.now() - start > timeoutMs) {
        throw new Error('STK500 timeout waiting for response');
      }
      await this.sleep(10);
    }

    const response = this.buffer.slice(0, length);
    this.buffer = this.buffer.slice(length);
    if (response[0] !== 0x14 || response[response.length - 1] !== 0x10) {
      throw new Error(`STK500 sync error: expected 0x14 ... 0x10, got [${response.join(',')}]`);
    }
    return response;
  }

  async sync() {
    for (let attempt = 0; attempt < 5; attempt += 1) {
      try {
        await this.sendCommand([0x30, 0x20], 2);
        return true;
      } catch (error) {
        console.debug(`STK500 sync retry ${attempt + 1} failed:`, error);
        await this.sleep(150);
      }
    }

    throw new Error('STK500 could not sync to the board. Ensure you selected the correct serial port.');
  }

  async enterProgMode() {
    await this.sendCommand([0x50, 0x20], 2);
  }

  async leaveProgMode() {
    await this.sendCommand([0x51, 0x20], 2);
  }

  async loadAddress(wordAddress) {
    await this.sendCommand([0x55, wordAddress & 0xff, (wordAddress >> 8) & 0xff, 0x20], 2);
  }

  async writePage(data) {
    if (data.length > 128) {
      throw new Error('Page size exceeds 128 bytes');
    }

    const command = [0x64, (data.length >> 8) & 0xff, data.length & 0xff, 0x46];
    const payload = new Uint8Array(command.length + data.length + 1);
    payload.set(command);
    payload.set(data, command.length);
    payload[payload.length - 1] = 0x20;
    await this.sendCommand(payload, 2);
  }

  sleep(ms) {
    return new Promise((resolve) => setTimeout(resolve, ms));
  }
}

function parseHexLine(line) {
  if (!line.startsWith(':')) {
    throw new Error('HEX line must start with ":"');
  }

  const payload = line.slice(1);
  if (payload.length < 10 || payload.length % 2 !== 0) {
    throw new Error(`Invalid HEX record length: ${line}`);
  }

  const bytes = [];
  for (let index = 0; index < payload.length; index += 2) {
    const value = Number.parseInt(payload.slice(index, index + 2), 16);
    if (Number.isNaN(value)) {
      throw new Error(`Invalid HEX byte in line: ${line}`);
    }
    bytes.push(value);
  }

  const checksum = bytes.reduce((sum, value) => (sum + value) & 0xff, 0);
  if (checksum !== 0) {
    throw new Error(`HEX checksum mismatch in line: ${line}`);
  }

  const byteCount = bytes[0];
  const address = (bytes[1] << 8) | bytes[2];
  const recordType = bytes[3];
  const data = bytes.slice(4, 4 + byteCount);

  if (data.length !== byteCount) {
    throw new Error(`HEX byte count mismatch in line: ${line}`);
  }

  return { byteCount, address, recordType, data };
}

export function parseHex(hexString) {
  const lines = hexString.split(/\r?\n/);
  const flashSize = 32768;
  const buffer = new Uint8Array(flashSize).fill(0xff);
  let maxAddress = 0;
  let baseAddress = 0;

  for (const rawLine of lines) {
    const line = rawLine.trim();
    if (!line) {
      continue;
    }

    const { byteCount, address, recordType, data } = parseHexLine(line);

    if (recordType === 0x00) {
      const fullAddress = baseAddress + address;
      if (fullAddress + byteCount > flashSize) {
        throw new Error(`HEX image exceeds Uno flash size at address 0x${fullAddress.toString(16)}`);
      }

      buffer.set(data, fullAddress);
      maxAddress = Math.max(maxAddress, fullAddress + byteCount);
      continue;
    }

    if (recordType === 0x01) {
      break;
    }

    if (recordType === 0x02) {
      baseAddress = ((data[0] << 8) | data[1]) * 16;
      continue;
    }

    if (recordType === 0x04) {
      baseAddress = ((data[0] << 8) | data[1]) * 65536;
    }
  }

  return buffer.slice(0, maxAddress);
}
