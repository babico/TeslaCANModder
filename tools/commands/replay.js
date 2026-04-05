/** replay command — Replay recorded CAN frames from a JSONL file. */

import { createReadStream } from 'node:fs';
import { createInterface } from 'node:readline';
import { setTimeout as delay } from 'node:timers/promises';

export async function runReplay(session, opts, out, C) {
  const { replayFile, replaySpeed } = opts;

  if (!replayFile) {
    console.error('ERROR: --input <path> is required for replay command');
    process.exit(1);
  }

  out.section('CAN Frame Replay');
  out.info(`File: ${replayFile}`);
  out.info(`Speed: ${replaySpeed}x`);

  const lines = [];
  const rl = createInterface({ input: createReadStream(replayFile), crlfDelay: Infinity });
  for await (const line of rl) {
    const trimmed = line.trim();
    if (!trimmed || trimmed.startsWith('timestamp,')) continue; // skip CSV header
    try {
      const obj = JSON.parse(trimmed);
      lines.push(obj);
    } catch {
      // skip non-JSON lines
    }
  }

  if (lines.length === 0) {
    out.fail('No frames found in file');
    return;
  }

  out.info(`Loaded ${lines.length} frames`);

  let sent = 0;
  for (let i = 0; i < lines.length; i++) {
    const frame = lines[i];
    const cmd = JSON.stringify({ cmd: 'raw', id: frame.id, data: frame.data, bus: frame.bus ?? 0 });
    session.send(cmd);
    sent++;

    if (i % 100 === 0 && i > 0) {
      out.info(`Sent ${sent}/${lines.length} frames...`);
    }

    // Delay between frames based on replay speed (default ~10ms between frames)
    if (replaySpeed > 0) {
      await delay(Math.max(1, Math.round(10 / replaySpeed)));
    }
  }

  out.pass(`Replayed ${sent} frames at ${replaySpeed}x speed`);
}
