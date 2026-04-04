import { BoardCommands } from './commands';

const NOOP = () => {};
const CONTROL_CHARACTER_PATTERN = /[\u0000-\u0008\u000B\u000C\u000E-\u001F\u007F]/g;

function sanitizeSerialLine(rawLine = '') {
  return String(rawLine)
    .replace(/\r/g, '')
    .replace(CONTROL_CHARACTER_PATTERN, '')
    .trim();
}

function extractJsonObjects(text = '') {
  const objects = [];
  let depth = 0;
  let startIndex = -1;

  for (let index = 0; index < text.length; index += 1) {
    const character = text[index];

    if (character === '{') {
      if (depth === 0) {
        startIndex = index;
      }
      depth += 1;
      continue;
    }

    if (character === '}' && depth > 0) {
      depth -= 1;
      if (depth === 0 && startIndex >= 0) {
        objects.push(text.slice(startIndex, index + 1));
        startIndex = -1;
      }
    }
  }

  return objects;
}

function looksLikeBoardJsonFragment(text = '') {
  return text.startsWith('{')
    || text.includes('{"t"')
    || text.includes('{"variant"')
    || text.includes('{"stream"')
    || text.includes('{"rawCan"');
}

function looksLikeTextLog(text = '') {
  return /^[A-Za-z][A-Za-z0-9 _.:,+()[\]/\\-]{0,240}$/.test(text);
}

export function parseSerialLine(rawLine = '') {
  const line = sanitizeSerialLine(rawLine);
  if (!line) {
    return [];
  }

  const jsonObjects = extractJsonObjects(line);
  if (jsonObjects.length > 0) {
    return jsonObjects.map((candidate) => {
      try {
        return { type: 'message', message: JSON.parse(candidate) };
      } catch {
        return { type: 'parse-error', line: candidate };
      }
    });
  }

  if (looksLikeTextLog(line)) {
    return [{ type: 'text', line }];
  }

  if (looksLikeBoardJsonFragment(line)) {
    return [{ type: 'parse-error', line }];
  }

  return [{ type: 'ignore', line }];
}

export function parseSerialChunk(buffer = '', chunk = '') {
  const combined = `${buffer}${chunk ?? ''}`.replace(/\r/g, '');
  const lines = combined.split('\n');
  const remainder = lines.pop() || '';
  const events = [];

  lines.forEach((rawLine) => {
    events.push(...parseSerialLine(rawLine));
  });

  return { remainder, events };
}

export class SerialBoardClient {
  constructor(callbacks = {}) {
    this.setCallbacks(callbacks);

    this.port = null;
    this.reader = null;
    this.writer = null;
    this.readableClosed = null;
    this.writableClosed = null;
    this.keepReading = false;
    this.disconnecting = false;
  }

  setCallbacks(callbacks = {}) {
    this.callbacks = {
      onOpen: callbacks.onOpen || NOOP,
      onMessage: callbacks.onMessage || NOOP,
      onText: callbacks.onText || NOOP,
      onParseError: callbacks.onParseError || NOOP,
      onError: callbacks.onError || NOOP,
      onClose: callbacks.onClose || NOOP,
    };
  }

  async connect(kind = 'usb') {
    if (!('serial' in navigator)) {
      throw new Error('Web Serial API is not available here. Use desktop Chrome/Edge for first flash, or Android Chrome for the paired HC-05 runtime path.');
    }

    if (kind === 'bluetooth') {
      alert('To connect via HC-05:\n1. Finish the USB-first setup first.\n2. Pair the HC-05 with your OS.\n3. In the next prompt, choose the paired Bluetooth serial COM port.');
    }

    let port = null;
    try {
      port = await navigator.serial.requestPort();
      await port.open({ baudRate: 115200 });

      const textDecoder = new TextDecoderStream();
      this.readableClosed = port.readable.pipeTo(textDecoder.writable).catch((error) => {
        this.callbacks.onError(error);
      });
      this.reader = textDecoder.readable.getReader();

      const textEncoder = new TextEncoderStream();
      this.writableClosed = textEncoder.readable.pipeTo(port.writable).catch((error) => {
        this.callbacks.onError(error);
      });
      this.writer = textEncoder.writable.getWriter();

      this.port = port;
      this.keepReading = true;
      this.callbacks.onOpen(kind);

      void this.readLoop();
      await this.send(BoardCommands.status());
    } catch (error) {
      if (port && !this.port) {
        try {
          await port.close();
        } catch (closeError) {
          this.callbacks.onError(closeError);
        }
      }

      throw error;
    }
  }

  async disconnect() {
    if (this.disconnecting) {
      return;
    }

    this.disconnecting = true;
    this.keepReading = false;

    try {
      if (this.reader) {
        try {
          await this.reader.cancel();
        } catch (error) {
          this.callbacks.onError(error);
        }
        this.reader.releaseLock();
        this.reader = null;
      }

      if (this.readableClosed) {
        try {
          await this.readableClosed;
        } catch (error) {
          this.callbacks.onError(error);
        }
        this.readableClosed = null;
      }

      if (this.writer) {
        try {
          await this.writer.close();
        } catch (error) {
          this.callbacks.onError(error);
        }
        this.writer.releaseLock();
        this.writer = null;
      }

      if (this.writableClosed) {
        try {
          await this.writableClosed;
        } catch (error) {
          this.callbacks.onError(error);
        }
        this.writableClosed = null;
      }

      if (this.port) {
        try {
          await this.port.close();
        } catch (error) {
          this.callbacks.onError(error);
        }
        this.port = null;
      }
    } finally {
      this.callbacks.onClose();
      this.disconnecting = false;
    }
  }

  async send(command) {
    if (!this.writer || !command) {
      return;
    }

    await this.writer.write(`${command}\n`);
  }

  async readLoop() {
    let buffer = '';

    while (this.reader && this.keepReading) {
      try {
        const { value, done } = await this.reader.read();
        if (done) {
          break;
        }

        if (!value) {
          continue;
        }

        const parsedChunk = parseSerialChunk(buffer, value);
        buffer = parsedChunk.remainder;

        for (const event of parsedChunk.events) {
          if (event.type === 'message') {
            this.callbacks.onMessage(event.message);
            continue;
          }

          if (event.type === 'text') {
            this.callbacks.onText(event.line);
            continue;
          }

          if (event.type === 'parse-error') {
            this.callbacks.onParseError(event.line);
          }
        }
      } catch (error) {
        if (this.keepReading) {
          this.callbacks.onError(error);
        }
        break;
      }
    }

    if (this.port) {
      await this.disconnect();
    }
  }
}
