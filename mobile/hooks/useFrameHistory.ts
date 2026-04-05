/** useFrameHistory — manages optional client-side CAN frame recording. */

import { useState, useCallback, useRef } from 'react';
import {
  setHistoryConfig,
  getHistoryConfig,
  startSession,
  addFrame,
  endSession,
  exportSessionCsv,
  exportSessionJson,
  type FrameRecord,
  type FrameSession,
  type FrameHistoryConfig,
} from '../lib/storage/frameHistory';

export function useFrameHistory() {
  const [config, setConfig] = useState<FrameHistoryConfig>(getHistoryConfig());
  const [recording, setRecording] = useState(false);
  const [currentFrameCount, setCurrentFrameCount] = useState(0);
  const sessionIdRef = useRef<string | null>(null);

  const toggleEnabled = useCallback((enabled: boolean) => {
    const newConfig = { ...config, enabled };
    setHistoryConfig(newConfig);
    setConfig(newConfig);
  }, [config]);

  const setMaxFrames = useCallback((max: number) => {
    const newConfig = { ...config, maxFramesPerSession: max };
    setHistoryConfig(newConfig);
    setConfig(newConfig);
  }, [config]);

  const start = useCallback(() => {
    if (!config.enabled) return;
    const id = startSession();
    sessionIdRef.current = id;
    setRecording(true);
    setCurrentFrameCount(0);
  }, [config.enabled]);

  const record = useCallback((frame: FrameRecord) => {
    if (!recording) return;
    const added = addFrame(frame);
    if (added) setCurrentFrameCount(prev => prev + 1);
  }, [recording]);

  const stop = useCallback(async (): Promise<FrameSession | null> => {
    setRecording(false);
    sessionIdRef.current = null;
    const session = await endSession();
    setCurrentFrameCount(0);
    return session;
  }, []);

  const exportCsv = useCallback((session: FrameSession) => {
    return exportSessionCsv(session);
  }, []);

  const exportJson = useCallback((session: FrameSession) => {
    return exportSessionJson(session);
  }, []);

  return {
    config,
    recording,
    currentFrameCount,
    toggleEnabled,
    setMaxFrames,
    start,
    record,
    stop,
    exportCsv,
    exportJson,
  };
}
