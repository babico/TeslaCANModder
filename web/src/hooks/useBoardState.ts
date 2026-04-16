import { useState, useCallback, useRef } from 'react';
import type { BoardState, BoardFeatures, BoardMessage, ConsoleMessage } from '@teslacanmodder/protocol';

const BUS_NAMES = ['FSD', 'Vehicle', 'Body'];

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
  busFsd: true,
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

export interface UseBoardStateReturn {
  state: BoardState;
  handleMessage: (msg: Record<string, unknown>) => void;
  addMessage: (type: ConsoleMessage['type'], text: string) => void;
  clearFrames: () => void;
  clearMessages: () => void;
  reset: () => void;
}

export function useBoardState(): UseBoardStateReturn {
  const [state, setState] = useState<BoardState>(initialState);
  const messageIdRef = useRef<number>(0);

  const addMessage = useCallback((type: ConsoleMessage['type'], text: string) => {
    const id = ++messageIdRef.current;
    setState(prev => ({
      ...prev,
      messages: [
        {
          id,
          type,
          text,
          ts: new Date().toLocaleTimeString(),
        },
        ...prev.messages,
      ].slice(0, 100),
    }));
  }, []);

  const handleMessage = useCallback((raw: Record<string, unknown>) => {
    const msg = raw as unknown as BoardMessage;
    if (msg.t === 'boot') {
      setState(prev => ({
        ...prev,
        variant: msg.variant || 'hw4',
        hardware: msg.hw || '—',
        driver: msg.drv || '—',
        features: msg.features || prev.features,
        fsd: msg.fsd !== undefined ? Boolean(msg.fsd) : prev.fsd,
        nag: msg.nag !== undefined ? Boolean(msg.nag) : prev.nag,
        profile: msg.sp ?? prev.profile,
        profilePinned: msg.spPin !== undefined ? Boolean(msg.spPin) : prev.profilePinned,
        offset: msg.offset ?? prev.offset,
        offsetPinned: msg.offsetPin !== undefined ? Boolean(msg.offsetPin) : prev.offsetPinned,
        isaChime: msg.isaChime !== undefined ? Boolean(msg.isaChime) : prev.isaChime,
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
    
    else if (msg.t === 'status') {
      setState(prev => ({
        ...prev,
        variant: msg.variant || prev.variant,
        hardware: msg.hw || prev.hardware,
        driver: msg.drv || prev.driver,
        uptime: msg.up || 0,
        rate: msg.rate || 0,
        fsd: Boolean(msg.fsd),
        nag: Boolean(msg.nag),
        profile: msg.sp ?? prev.profile,
        profilePinned: Boolean(msg.spPin),
        offset: msg.offset ?? prev.offset,
        offsetPinned: Boolean(msg.offsetPin),
        isaChime: Boolean(msg.isaChime),
        streaming: Boolean(msg.stream?.on),
        features: msg.features || prev.features,
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
    
    else if (msg.t === 'frame') {
      setState(prev => {
        const newFrames = [
          {
            key: `${msg.seq}-${msg.id}`,
            id: msg.id,
            dir: msg.dir,
            bus: msg.bus,
            busName: BUS_NAMES[msg.bus] || `Bus${msg.bus}`,
            seq: msg.seq,
            dlc: msg.dlc,
            data: msg.d || '',
            ts: new Date().toLocaleTimeString(),
          },
          ...prev.frames,
        ].slice(0, 100);
        
        return {
          ...prev,
          frames: newFrames,
          frameCount: prev.frameCount + 1,
        };
      });
    }
    
    else if (msg.t === 'ack') {
      addMessage('info', `OK ${msg.cmd}`);
    }
    
    else if (msg.t === 'error') {
      addMessage('error', msg.msg || 'Error');
    }
    
    else if (msg.t === 'log') {
      const text = msg.msg || 'Log';
      addMessage('info', text);
      if (text.includes('Summon burst started')) {
        setState(prev => ({ ...prev, summonActive: true }));
      } else if (text.includes('Summon burst complete') || text.includes('Summon stopped')) {
        setState(prev => ({ ...prev, summonActive: false }));
      }
    }
    
    else if (msg.t === 'pong') {
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
