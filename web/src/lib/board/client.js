import { BoardCommands } from './commands';

const NOOP = () => {};

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

        buffer += value;
        const lines = buffer.split('\n');
        buffer = lines.pop() || '';

        for (const rawLine of lines) {
          const line = rawLine.trim();
          if (!line) {
            continue;
          }

          try {
            this.callbacks.onMessage(JSON.parse(line));
          } catch {
            this.callbacks.onParseError(line);
            this.callbacks.onText(line);
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
