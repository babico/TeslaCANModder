/** useBoardState — manages firmware board state from JSON messages. */

import { useState, useCallback, useRef } from 'react';
import type { BoardFeatures, CanFrame, ConsoleMessage as LogMessage, BoardState } from '@teslacanmodder/protocol';

function detectBoard(hw: string): 'arduino' | 'esp32' | 'unknown' {
  const lower = hw.toLowerCase();
  if (lower.includes('arduino') || lower.includes('uno')) return 'arduino';
  if (lower.includes('esp32') || lower.includes('esp')) return 'esp32';
  return 'unknown';
}

const initialState: BoardState = {
  variant: 'hw4',
  hardware: '—',
  driver: '—',
  board: 'unknown',
  uptime: 0,
  rate: 0,

  fsd: false,
  nag: false,
  profile: 1,
  profilePinned: false,
  offset: 0,
  offsetPinned: false,
  isaChime: false,
  summonInject: false,
  summonActive: false,

  canOnline: false,
  standby: false,
  bus1: false,
  bus2: false,
  bus3: false,
  busFsd: false,
  busVehicle: false,
  busBody: false,

  streaming: false,
  frames: [],
  frameCount: 0,

  messages: [],

  features: {
    fsd: true,
    profile: true,
    nag: true,
    speedOffset: false,
    isaSpeedChime: false,
    summon: false,
  },
};

export function useBoardState() {
  const [state, setState] = useState<BoardState>(initialState);
  const messageIdRef = useRef(0);

  const addMessage = useCallback((type: 'info' | 'error', text: string) => {
    const id = ++messageIdRef.current;
    setState(prev => ({
      ...prev,
      messages: [
        { id, type, text, ts: new Date().toLocaleTimeString() },
        ...prev.messages,
      ].slice(0, 100),
    }));
  }, []);

  const handleMessage = useCallback((msg: Record<string, unknown>) => {
    const t = msg.t as string;

    if (t === 'boot') {
      const hw = (msg.hw as string) || '—';
      setState(prev => ({
        ...prev,
        variant: (msg.variant as string) || 'hw4',
        hardware: hw,
        driver: (msg.drv as string) || '—',
        board: detectBoard(hw),
        features: (msg.features as BoardFeatures) || prev.features,
        fsd: msg.fsd !== undefined ? Boolean(msg.fsd) : prev.fsd,
        nag: msg.nag !== undefined ? Boolean(msg.nag) : prev.nag,
        profile: (msg.sp as number) ?? prev.profile,
        profilePinned: msg.spPin !== undefined ? Boolean(msg.spPin) : prev.profilePinned,
        offset: (msg.offset as number) ?? prev.offset,
        offsetPinned: msg.offsetPin !== undefined ? Boolean(msg.offsetPin) : prev.offsetPinned,
        isaChime: msg.isaChime !== undefined ? Boolean(msg.isaChime) : prev.isaChime,
        summonInject: msg.summonInject !== undefined ? Boolean(msg.summonInject) : prev.summonInject,
        canOnline: msg.canOnline !== undefined ? Boolean(msg.canOnline) : prev.canOnline,
        standby: msg.standby !== undefined ? Boolean(msg.standby) : prev.standby,
        bus1: msg.bus1 !== undefined ? Boolean(msg.bus1) : prev.bus1,
        bus2: msg.bus2 !== undefined ? Boolean(msg.bus2) : prev.bus2,
        bus3: msg.bus3 !== undefined ? Boolean(msg.bus3) : prev.bus3,
        busFsd: msg.busFsd !== undefined ? Boolean(msg.busFsd) : prev.busFsd,
        busVehicle: msg.busVehicle !== undefined ? Boolean(msg.busVehicle) : prev.busVehicle,
        busBody: msg.busBody !== undefined ? Boolean(msg.busBody) : prev.busBody,
      }));
      addMessage('info', `Board connected: ${msg.hw}`);
    }

    else if (t === 'status') {
      const hw = (msg.hw as string) || '';
      setState(prev => ({
        ...prev,
        variant: (msg.variant as string) || prev.variant,
        hardware: hw || prev.hardware,
        driver: (msg.drv as string) || prev.driver,
        board: hw ? detectBoard(hw) : prev.board,
        uptime: (msg.up as number) || 0,
        rate: (msg.rate as number) || 0,
        fsd: Boolean(msg.fsd),
        nag: Boolean(msg.nag),
        profile: (msg.sp as number) ?? prev.profile,
        profilePinned: Boolean(msg.spPin),
        offset: (msg.offset as number) ?? prev.offset,
        offsetPinned: Boolean(msg.offsetPin),
        isaChime: Boolean(msg.isaChime),
        summonInject: msg.summonInject !== undefined ? Boolean(msg.summonInject) : prev.summonInject,
        streaming: Boolean((msg.stream as any)?.on),
        features: (msg.features as BoardFeatures) || prev.features,
        canOnline: msg.canOnline !== undefined ? Boolean(msg.canOnline) : prev.canOnline,
        standby: msg.standby !== undefined ? Boolean(msg.standby) : prev.standby,
        bus1: msg.bus1 !== undefined ? Boolean(msg.bus1) : prev.bus1,
        bus2: msg.bus2 !== undefined ? Boolean(msg.bus2) : prev.bus2,
        bus3: msg.bus3 !== undefined ? Boolean(msg.bus3) : prev.bus3,
        busFsd: msg.busFsd !== undefined ? Boolean(msg.busFsd) : prev.busFsd,
        busVehicle: msg.busVehicle !== undefined ? Boolean(msg.busVehicle) : prev.busVehicle,
        busBody: msg.busBody !== undefined ? Boolean(msg.busBody) : prev.busBody,
      }));
    }

    else if (t === 'frame') {
      setState(prev => {
        const frame: CanFrame = {
          key: `${msg.seq}-${msg.id}`,
          id: msg.id as number,
          dir: msg.dir as string,
          seq: msg.seq as number | undefined,
          dlc: msg.dlc as number,
          data: (msg.d as string) || '',
          ts: new Date().toLocaleTimeString(),
        };
        return {
          ...prev,
          frames: [frame, ...prev.frames].slice(0, 100),
          frameCount: prev.frameCount + 1,
        };
      });
    }

    else if (t === 'ack') {
      addMessage('info', `OK ${msg.cmd}`);
    }

    else if (t === 'error') {
      addMessage('error', (msg.msg as string) || 'Error');
    }

    else if (t === 'log') {
      const text = (msg.msg as string) || 'Log';
      addMessage('info', text);
      if (text.includes('Summon burst started')) {
        setState(prev => ({ ...prev, summonActive: true }));
      } else if (text.includes('Summon burst complete') || text.includes('Summon stopped')) {
        setState(prev => ({ ...prev, summonActive: false }));
      }
    }

    else if (t === 'pong') {
      addMessage('info', 'Pong received');
    }
  }, [addMessage]);

  const clearFrames = useCallback(() => {
    setState(prev => ({ ...prev, frames: [], frameCount: 0 }));
  }, []);

  const clearMessages = useCallback(() => {
    setState(prev => ({ ...prev, messages: [] }));
  }, []);

  const reset = useCallback(() => {
    setState(initialState);
  }, []);

  return {
    state,
    handleMessage,
    addMessage,
    clearFrames,
    clearMessages,
    reset,
  };
}
