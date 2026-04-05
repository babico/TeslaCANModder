/** JSON line parser — extracts JSON messages from noisy serial input. */

export interface ParsedEvent {
  type: 'message' | 'ignore';
  message?: Record<string, unknown>;
  raw: string;
}

/** Parse a single serial line, extracting JSON if present. */
export function parseSerialLine(line: string): ParsedEvent[] {
  const cleaned = line.replace(/\0/g, '').trim();
  if (!cleaned) return [];

  const start = cleaned.indexOf('{');
  const end = cleaned.lastIndexOf('}');

  if (start >= 0 && end > start) {
    try {
      const msg = JSON.parse(cleaned.slice(start, end + 1));
      return [{ type: 'message', message: msg, raw: cleaned }];
    } catch {
      return [{ type: 'ignore', raw: cleaned }];
    }
  }

  return [{ type: 'ignore', raw: cleaned }];
}

/** Parse a chunk of serial data, returning events and leftover buffer. */
export function parseSerialChunk(
  remainder: string,
  chunk: string,
): { remainder: string; events: ParsedEvent[] } {
  const combined = remainder + chunk;
  const lines = combined.split('\n');
  const newRemainder = lines.pop() || '';
  const events: ParsedEvent[] = [];

  for (const line of lines) {
    events.push(...parseSerialLine(line));
  }

  return { remainder: newRemainder, events };
}
