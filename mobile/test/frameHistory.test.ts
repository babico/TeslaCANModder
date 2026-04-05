import {
  setHistoryConfig,
  getHistoryConfig,
  startSession,
  addFrame,
  endSession,
  getCurrentSession,
  exportSessionCsv,
  exportSessionJson,
  type FrameRecord,
  type FrameSession,
} from '../lib/storage/frameHistory';

// Reset module state between tests
beforeEach(() => {
  setHistoryConfig({ enabled: false, maxFramesPerSession: 10000 });
  // End any lingering session
  endSession().catch(() => {});
});

const frame: FrameRecord = {
  id: 69,
  dir: 'rx',
  dlc: 8,
  data: '0A1B2C3D4E5F6A7B',
  seq: 1,
  ts: 1700000000000,
};

describe('config', () => {
  it('getHistoryConfig returns defaults', () => {
    const cfg = getHistoryConfig();
    expect(cfg.enabled).toBe(false);
    expect(cfg.maxFramesPerSession).toBe(10000);
  });

  it('setHistoryConfig merges partial updates', () => {
    setHistoryConfig({ enabled: true });
    const cfg = getHistoryConfig();
    expect(cfg.enabled).toBe(true);
    expect(cfg.maxFramesPerSession).toBe(10000);
  });
});

describe('session lifecycle', () => {
  it('startSession creates a session', () => {
    const id = startSession();
    expect(id).toMatch(/^session_\d+$/);
    const current = getCurrentSession();
    expect(current).not.toBeNull();
    expect(current!.id).toBe(id);
    expect(current!.frameCount).toBe(0);
  });

  it('getCurrentSession returns null when no session', () => {
    expect(getCurrentSession()).toBeNull();
  });
});

describe('addFrame', () => {
  it('returns false when disabled', () => {
    startSession();
    expect(addFrame(frame)).toBe(false);
  });

  it('returns false when no session', () => {
    setHistoryConfig({ enabled: true });
    expect(addFrame(frame)).toBe(false);
  });

  it('adds frame when enabled and session active', () => {
    setHistoryConfig({ enabled: true });
    startSession();
    expect(addFrame(frame)).toBe(true);
    const session = getCurrentSession()!;
    expect(session.frameCount).toBe(1);
    expect(session.frames).toHaveLength(1);
  });

  it('respects maxFramesPerSession', () => {
    setHistoryConfig({ enabled: true, maxFramesPerSession: 2 });
    startSession();
    expect(addFrame(frame)).toBe(true);
    expect(addFrame({ ...frame, seq: 2 })).toBe(true);
    expect(addFrame({ ...frame, seq: 3 })).toBe(false);
  });
});

describe('endSession', () => {
  it('returns null when no session', async () => {
    const result = await endSession();
    expect(result).toBeNull();
  });

  it('returns session data and clears current', async () => {
    setHistoryConfig({ enabled: true });
    const id = startSession();
    addFrame(frame);
    const session = await endSession();
    expect(session).not.toBeNull();
    expect(session!.id).toBe(id);
    expect(session!.frameCount).toBe(1);
    expect(session!.endedAt).toBeDefined();
    expect(getCurrentSession()).toBeNull();
  });
});

describe('exportSessionCsv', () => {
  it('generates CSV with header and data rows', () => {
    const session: FrameSession = {
      id: 'test',
      startedAt: 1700000000000,
      frameCount: 1,
      frames: [frame],
    };
    const csv = exportSessionCsv(session);
    const lines = csv.split('\n');
    expect(lines[0]).toBe('timestamp,id,id_hex,direction,dlc,data,seq');
    expect(lines[1]).toContain('69');
    expect(lines[1]).toContain('0x045');
    expect(lines[1]).toContain('rx');
    expect(lines[1]).toContain('0A1B2C3D4E5F6A7B');
  });

  it('handles missing seq', () => {
    const session: FrameSession = {
      id: 'test',
      startedAt: 0,
      frameCount: 1,
      frames: [{ id: 69, dir: 'rx', dlc: 8, data: 'FF', ts: 0 }],
    };
    const csv = exportSessionCsv(session);
    const dataLine = csv.split('\n')[1];
    // seq should be empty at the end
    expect(dataLine.endsWith(',')).toBe(true);
  });
});

describe('exportSessionJson', () => {
  it('produces valid JSON', () => {
    const session: FrameSession = {
      id: 'test',
      startedAt: 0,
      frameCount: 0,
      frames: [],
    };
    const json = exportSessionJson(session);
    const parsed = JSON.parse(json);
    expect(parsed.id).toBe('test');
  });
});
