/** useBoardState — manages firmware board state from JSON messages. */

import { useState, useCallback, useRef } from 'react';

export interface BoardFeatures {
  fsd: boolean;
  profile: boolean;
  nag: boolean;
  speedOffset: boolean;
  isaSpeedChime: boolean;
  summon: boolean;
}

export interface CanFrame {
  key: string;
  id: number;
  dir: string;
  seq?: number;
  dlc: number;
  data: string;
  ts: string;
}

export interface LogMessage {
  id: number;
  type: 'info' | 'error';
  text: string;
  ts: string;
}

export interface BoardState {
  variant: string;
  hardware: string;
  driver: string;
  uptime: number;
  rate: number;

  fsd: boolean;
  nag: boolean;
  profile: number;
  profilePinned: boolean;
  offset: number;
  offsetPinned: boolean;
  isaChime: boolean;
  summonActive: boolean;

  canOnline: boolean;
  standby: boolean;
  bus2: boolean;

  streaming: boolean;
  frames: CanFrame[];
  frameCount: number;

  messages: LogMessage[];

  features: BoardFeatures;
}

const initialState: BoardState = {
  variant: 'hw4',
  hardware: '—',
  driver: '—',
  uptime: 0,
  rate: 0,

  fsd: false,
  nag: false,
  profile: 1,
  profilePinned: false,
  offset: 0,
  offsetPinned: false,
  isaChime: false,
  summonActive: false,

  canOnline: false,
  standby: false,
  bus2: false,

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
      setState(prev => ({
        ...prev,
        variant: (msg.variant as string) || 'hw4',
        hardware: (msg.hw as string) || '—',
        driver: (msg.drv as string) || '—',
        features: (msg.features as BoardFeatures) || prev.features,
        fsd: msg.fsd !== undefined ? Boolean(msg.fsd) : prev.fsd,
        nag: msg.nag !== undefined ? Boolean(msg.nag) : prev.nag,
        profile: (msg.sp as number) ?? prev.profile,
        profilePinned: msg.spPin !== undefined ? Boolean(msg.spPin) : prev.profilePinned,
        offset: (msg.offset as number) ?? prev.offset,
        offsetPinned: msg.offsetPin !== undefined ? Boolean(msg.offsetPin) : prev.offsetPinned,
        isaChime: msg.isaChime !== undefined ? Boolean(msg.isaChime) : prev.isaChime,
        canOnline: msg.canOnline !== undefined ? Boolean(msg.canOnline) : prev.canOnline,
        standby: msg.standby !== undefined ? Boolean(msg.standby) : prev.standby,
        bus2: msg.bus2 !== undefined ? Boolean(msg.bus2) : prev.bus2,
      }));
      addMessage('info', `Board connected: ${msg.hw}`);
    }

    else if (t === 'status') {
      setState(prev => ({
        ...prev,
        variant: (msg.variant as string) || prev.variant,
        hardware: (msg.hw as string) || prev.hardware,
        driver: (msg.drv as string) || prev.driver,
        uptime: (msg.up as number) || 0,
        rate: (msg.rate as number) || 0,
        fsd: Boolean(msg.fsd),
        nag: Boolean(msg.nag),
        profile: (msg.sp as number) ?? prev.profile,
        profilePinned: Boolean(msg.spPin),
        offset: (msg.offset as number) ?? prev.offset,
        offsetPinned: Boolean(msg.offsetPin),
        isaChime: Boolean(msg.isaChime),
        streaming: Boolean((msg.stream as any)?.on),
        features: (msg.features as BoardFeatures) || prev.features,
        canOnline: msg.canOnline !== undefined ? Boolean(msg.canOnline) : prev.canOnline,
        standby: msg.standby !== undefined ? Boolean(msg.standby) : prev.standby,
        bus2: msg.bus2 !== undefined ? Boolean(msg.bus2) : prev.bus2,
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
