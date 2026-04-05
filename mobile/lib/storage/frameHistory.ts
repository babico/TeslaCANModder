/**
 * Frame history — optional client-side storage for CAN frame sessions.
 * Uses AsyncStorage on mobile, falls back to in-memory on web.
 */

import { Platform } from 'react-native';

const STORAGE_KEY = '@teslacanmodder/frame_history';
const MAX_SESSIONS = 20;

export interface FrameRecord {
  id: number;
  dir: string;
  dlc: number;
  data: string;
  seq?: number;
  ts: number;
}

export interface FrameSession {
  id: string;
  startedAt: number;
  endedAt?: number;
  frameCount: number;
  frames: FrameRecord[];
}

export interface FrameHistoryConfig {
  enabled: boolean;
  maxFramesPerSession: number;
}

const DEFAULT_CONFIG: FrameHistoryConfig = {
  enabled: false,
  maxFramesPerSession: 10000,
};

let _config = { ...DEFAULT_CONFIG };
let _currentSession: FrameSession | null = null;

// Lazy-load AsyncStorage to avoid crash on web when not available
async function getStorage() {
  if (Platform.OS === 'web') return null;
  try {
    const mod = await import('@react-native-async-storage/async-storage');
    return mod.default;
  } catch {
    return null;
  }
}

export function setHistoryConfig(config: Partial<FrameHistoryConfig>) {
  _config = { ..._config, ...config };
}

export function getHistoryConfig(): FrameHistoryConfig {
  return { ..._config };
}

export function startSession(): string {
  const id = `session_${Date.now()}`;
  _currentSession = {
    id,
    startedAt: Date.now(),
    frameCount: 0,
    frames: [],
  };
  return id;
}

export function addFrame(frame: FrameRecord): boolean {
  if (!_config.enabled || !_currentSession) return false;
  if (_currentSession.frames.length >= _config.maxFramesPerSession) return false;
  _currentSession.frames.push(frame);
  _currentSession.frameCount++;
  return true;
}

export async function endSession(): Promise<FrameSession | null> {
  if (!_currentSession) return null;
  _currentSession.endedAt = Date.now();
  const session = { ..._currentSession };

  const storage = await getStorage();
  if (storage) {
    try {
      const raw = await storage.getItem(STORAGE_KEY);
      const sessions: FrameSession[] = raw ? JSON.parse(raw) : [];
      sessions.unshift(session);
      if (sessions.length > MAX_SESSIONS) sessions.length = MAX_SESSIONS;
      await storage.setItem(STORAGE_KEY, JSON.stringify(sessions));
    } catch { /* storage error — silently drop */ }
  }

  _currentSession = null;
  return session;
}

export async function getSavedSessions(): Promise<FrameSession[]> {
  const storage = await getStorage();
  if (!storage) return [];
  try {
    const raw = await storage.getItem(STORAGE_KEY);
    return raw ? JSON.parse(raw) : [];
  } catch {
    return [];
  }
}

export async function clearSavedSessions(): Promise<void> {
  const storage = await getStorage();
  if (storage) {
    await storage.removeItem(STORAGE_KEY);
  }
}

export function getCurrentSession(): FrameSession | null {
  return _currentSession ? { ..._currentSession } : null;
}

/** Export session as CSV string. */
export function exportSessionCsv(session: FrameSession): string {
  const lines = ['timestamp,id,id_hex,direction,dlc,data,seq'];
  for (const f of session.frames) {
    const hex = `0x${f.id.toString(16).toUpperCase().padStart(3, '0')}`;
    lines.push(`${f.ts},${f.id},${hex},${f.dir},${f.dlc},${f.data},${f.seq ?? ''}`);
  }
  return lines.join('\n');
}

/** Export session as JSON string. */
export function exportSessionJson(session: FrameSession): string {
  return JSON.stringify(session, null, 2);
}
